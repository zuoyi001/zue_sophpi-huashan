#include "pubsub.h"

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <semaphore.h>

#include "cvi_eventhub.h"
#if USING_LIST_HEAD
#include "list.h"
#endif
#include "uthash.h"
#include "utlist.h"
#ifdef PS_FREE_RTOS
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#else
#include <pthread.h>
#endif

typedef uint32_t u32;
typedef struct ps_queue_s {
#ifdef PS_FREE_RTOS
    QueueHandle_t queue;
#else
    ps_msg_t **messages;
    size_t size;
    size_t count;
    size_t head;
    size_t tail;
    pthread_mutex_t mux;
    pthread_cond_t not_empty;
#endif
} ps_queue_t;

typedef struct subscriber_list_s {
    ps_subscriber_t *su;
    struct subscriber_list_s *next;
    struct subscriber_list_s *prev;
} subscriber_list_t;

typedef struct topic_map_s {
    u32 topic;
    subscriber_list_t *subscribers;
    ps_msg_t *sticky;
    UT_hash_handle hh;
} topic_map_t;

typedef struct subscriptions_list_s {
    topic_map_t *tm;
    struct subscriptions_list_s *next;
    struct subscriptions_list_s *prev;
} subscriptions_list_t;

struct ps_subscriber_s {
#ifdef SUBSCRIBE_QUEUE
    uint32_t overflow;
    ps_queue_t *q;
#endif
    subscriptions_list_t *subs;
#ifdef USING_LIST_HEAD
    struct list_head list;
#else
    ps_subscriber_t *next;
#endif
    int32_t (*new_msg_cb)(void *argv, ps_msg_t *msg);
};

#ifdef PS_FREE_RTOS
static SemaphoreHandle_t lock;
#else
static pthread_mutex_t lock;
#endif

static topic_map_t *topic_map = NULL;

static uint32_t stat_live_msg;
static uint32_t stat_live_subscribers;
#ifdef USING_LIST_HEAD
struct list_head subs_list;
#else
ps_subscriber_t subs_list_head;
#endif

typedef struct pubsub_thread_mem_s {
    pthread_t PubsubTid;
    ps_msg_t Pubsubmsg;
} pubsub_thread_mem_t;

#ifdef PS_FREE_RTOS
#define PORT_LOCK xSemaphoreTake(lock, portMAX_DELAY);
#define PORT_UNLOCK xSemaphoreGive(lock);
#else
#define PORT_LOCK pthread_mutex_lock(&lock);
#define PORT_UNLOCK pthread_mutex_unlock(&lock);
#endif
static int inited = 0;
void ps_init(void) {
    if (inited == 0) {
#ifdef PS_FREE_RTOS
    lock = xSemaphoreCreateMutex();
#else
    pthread_mutex_init(&lock, NULL);
#endif
#ifdef USING_LIST_HEAD
        INIT_LIST_HEAD(&subs_list);
#else
        subs_list_head.next = NULL;
#endif
        inited = 1;
    }
}
#if SUBSCRIBE_QUEUE
#ifndef PS_FREE_RTOS
static int deadline_ms(int64_t ms, struct timespec *tout) {
#ifdef PS_USE_GETTIMEOFDAY
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tout->tv_sec = tv.tv_sec;
    tout->tv_nsec = tv.tv_usec * 1000;
#else
    clock_gettime(CLOCK_MONOTONIC, tout);
#endif
    tout->tv_sec += (ms / 1000);
    tout->tv_nsec += ((ms % 1000) * 1000000);
    if (tout->tv_nsec > 1000000000) {
        tout->tv_sec++;
        tout->tv_nsec -= 1000000000;
    }
    return 0;
}
#endif
#endif
#ifdef SUBSCRIBE_QUEUE
ps_queue_t *ps_new_queue(size_t sz) {
    ps_queue_t *q = calloc(1, sizeof(ps_queue_t));
#ifdef PS_FREE_RTOS
    q->queue = xQueueCreate(sz, sizeof(void *));
#else
    q->size = sz;
    q->messages = calloc(sz, sizeof(void *));
    pthread_mutex_init(&q->mux, NULL);

#ifdef PS_USE_GETTIMEOFDAY
    pthread_cond_init(&q->not_empty, NULL);
#else
    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
    pthread_cond_init(&q->not_empty, &cond_attr);
    pthread_condattr_destroy(&cond_attr);
#endif
#endif
    return q;
}

void ps_free_queue(ps_queue_t *q) {
#ifdef PS_FREE_RTOS
    vQueueDelete(q->queue);
#else
    free(q->messages);
    pthread_mutex_destroy(&q->mux);
    pthread_cond_destroy(&q->not_empty);
#endif
    free(q);
}

int ps_queue_push(ps_queue_t *q, ps_msg_t *msg) {
#ifdef PS_FREE_RTOS
    if (xQueueSend(q->queue, &msg, 0) != pdTRUE) {
        return -1;
    }
    return 0;
#else
    int ret = 0;
    pthread_mutex_lock(&q->mux);
    if (q->count > q->size) {
        ps_unref_msg(q->messages[q->tail]);
        q->count = q->size;
        q->head = q->tail;
        q->messages[q->head] = msg;
        if (++q->tail >= q->size) q->tail = 0;
    } else {
        q->messages[q->head] = msg;
        if (++q->head >= q->size) q->head = 0;
        q->count++;
    }

    pthread_cond_signal(&q->not_empty);

    pthread_mutex_unlock(&q->mux);
    return ret;
#endif
}

ps_msg_t *ps_queue_pull(ps_queue_t *q, int64_t timeout) {
    ps_msg_t *msg = NULL;

#ifdef PS_FREE_RTOS
    if (timeout < 0) {
        timeout = portMAX_DELAY;
    }
    if (xQueueReceive(q->queue, &msg, timeout / portTICK_PERIOD_MS) != pdTRUE) {
        return NULL;
    }
    return msg;

#else
    struct timespec tout = {0};
    bool init_tout = false;

    pthread_mutex_lock(&q->mux);
    while (q->count == 0) {
        if (timeout == 0) {
            goto exit_fn;
        } else if (timeout < 0) {
            pthread_cond_wait(&q->not_empty, &q->mux);
        } else {
            if (!init_tout) {
                init_tout = true;
                deadline_ms(timeout, &tout);
            }
            if (pthread_cond_timedwait(&q->not_empty, &q->mux, &tout) != 0)
                goto exit_fn;
        }
    }
    msg = q->messages[q->tail];
    if (++q->tail >= q->size) q->tail = 0;
    q->count--;

exit_fn:
    pthread_mutex_unlock(&q->mux);
    return msg;
#endif
}

size_t ps_queue_waiting(ps_queue_t *q) {
#ifdef PS_FREE_RTOS
    return uxQueueMessagesWaiting(q->queue);
#else
    size_t res = 0;
    pthread_mutex_lock(&q->mux);
    res = q->count;
    pthread_mutex_unlock(&q->mux);
    return res;
#endif
}
#endif

void ps_msg_set_topic(ps_msg_t *msg, const u32 topic) {
    if (msg != NULL) {
        msg->topic = topic;
    }
}

ps_msg_t *ps_ref_msg(ps_msg_t *msg) {
#ifdef SUBSCRIBE_QUEUE
    if (msg != NULL) __sync_add_and_fetch(&msg->_ref, 1);
#endif
    return msg;
}

void ps_unref_msg(ps_msg_t *msg) {
    if (msg == NULL) return;
#ifdef SUBSCRIBE_QUEUE
    if (__sync_sub_and_fetch(&msg->_ref, 1) == 0) {
        free(msg);
        __sync_sub_and_fetch(&stat_live_msg, 1);
    }
#else
    __sync_sub_and_fetch(&stat_live_msg, 1);
#endif
}

int ps_stats_live_msg(void) { return __sync_fetch_and_add(&stat_live_msg, 0); }

static int free_topic_if_empty(topic_map_t *tm) {
    if (tm && tm->subscribers == NULL) {
        if (topic_map != NULL) HASH_DEL(topic_map, tm);
        tm = NULL;
    }
    return 0;
}
static topic_map_t *fetch_topic(const u32 topic) {
    topic_map_t *tm = NULL;
    HASH_FIND_INT(topic_map, &topic, tm);
    return tm;
}

static topic_map_t *create_topic(const u32 topic) {
    topic_map_t *tm = NULL;
    tm = calloc(1, sizeof(*tm));
    tm->topic = topic;
    HASH_ADD_KEYPTR(hh, topic_map, &tm->topic, sizeof(tm->topic), tm);
    return tm;
}

static topic_map_t *fetch_topic_create_if_not_exist(const u32 topic) {
    topic_map_t *tm = NULL;
    tm = fetch_topic(topic);
    if (tm == NULL) {
        tm = create_topic(topic);
    }
    return tm;
}

int register_topic(const u32 topic) {
    int ret = 0;
    PORT_LOCK
    if (fetch_topic_create_if_not_exist(topic) == NULL) {
        ret = -1;
    }
    PORT_UNLOCK
    return ret;
}

int unregister_topic(const u32 topic) {
    int ret = 0;
    topic_map_t *tm = NULL;
    PORT_LOCK
    tm = fetch_topic(topic);
    if (tm != NULL) {
        HASH_DEL(topic_map, tm);
        tm = NULL;
    }
    PORT_UNLOCK
    return ret;
}
ps_subscriber_t *to_ps_subscriber(void *argv) {
    return (ps_subscriber_t *)argv;
}
static int push_subscriber_queue(ps_subscriber_t *su, ps_msg_t *msg) {
#ifdef SUBSCRIBE_QUEUE
    ps_ref_msg(msg);
    if (ps_queue_push(su->q, msg) != 0) {
        __sync_add_and_fetch(&su->overflow, 1);
        ps_unref_msg(msg);
        return -1;
    }
#endif
    if (su->new_msg_cb != NULL) {
        su->new_msg_cb((void *)su, msg);
    }

    return 0;
}

static void add_new_subscriber(ps_subscriber_t *su) {
    ps_subscriber_t *ptr = &subs_list_head;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = su;
}

static void list_del_subscriber(ps_subscriber_t *su) {
    ps_subscriber_t *ptr = &subs_list_head;
    ps_subscriber_t *ptr_next = ptr->next;
    while (ptr_next != NULL) {
        if (ptr_next == su) {
            ptr->next = ptr_next->next;
            break;
        }
        ptr = ptr_next;
        ptr_next = ptr_next->next;
    }
}

static void list_del_subscribers(void) {
    ps_subscriber_t *ptr = subs_list_head.next;
    ps_subscriber_t *ptr_next = ptr->next;
    while (ptr_next != NULL) {
        if (ptr != NULL) {
            free(ptr);
        }
        ptr = ptr_next;
        ptr_next = ptr->next;
    }
    if (ptr != NULL) {
        free(ptr);
    }
}
ps_subscriber_t *ps_create_subscriber(void) {
    ps_subscriber_t *su = calloc(1, sizeof(ps_subscriber_t));
    if (!su) return NULL;
    __sync_add_and_fetch(&stat_live_subscribers, 1);
#ifdef USING_LIST_HEAD
    list_add_tail(&su->list, &subs_list);
#else
    add_new_subscriber(su);
#endif
    return su;
}

ps_subscriber_t *ps_new_subscriber(u32 *subs) {
    ps_subscriber_t *su = calloc(1, sizeof(ps_subscriber_t));

    ps_subscribe_many(su, subs);
    __sync_add_and_fetch(&stat_live_subscribers, 1);
#ifdef USING_LIST_HEAD
    list_add_tail(&su->list, &subs_list);
#else
    add_new_subscriber(su);
#endif
    return su;
}

void ps_free_subscriber(ps_subscriber_t *su) {
    ps_unsubscribe_all(su);

#ifdef SUBSCRIBE_QUEUE
    ps_flush(su);
    if (su->q) ps_free_queue(su->q);
#endif
#ifdef USING_LIST_HEAD
    list_del(&su->list);
#else
    list_del_subscriber(su);
#endif
    if (su) {
        free(su);
        su = NULL;
    }
    __sync_sub_and_fetch(&stat_live_subscribers, 1);
}
void ps_set_cb(ps_subscriber_t *su,
               int (*new_msg_cb)(void *argv, ps_msg_t *msg)) {
    su->new_msg_cb = new_msg_cb;
}

void clean_all_subscribers(void) {
#ifdef USING_LIST_HEAD
    ps_subscriber_t *su = NULL, *tmp = NULL;
    list_for_each_entry_safe(su, tmp, &subs_list, list) {
        if (su) ps_free_subscriber(su);
    }
#else
    list_del_subscribers();
#endif
}

int ps_stats_live_subscribers(void) {
    return __sync_fetch_and_add(&stat_live_subscribers, 0);
}

int ps_subscribe_many(ps_subscriber_t *su, u32 *subs) {
    int n = 0;
    size_t idx = 0;
    for (idx = 0; idx < sizeof(subs) / sizeof(uint32_t); idx++) {
        if (ps_subscribe(su, subs[idx]) == 0) n++;
    }
    return n;
}

int ps_subscribe(ps_subscriber_t *su, const u32 topic) {
    int ret = 0;
    topic_map_t *tm;
    subscriber_list_t *sl;
    subscriptions_list_t *subs;

    PORT_LOCK
    tm = fetch_topic_create_if_not_exist(topic);
    DL_SEARCH_SCALAR(tm->subscribers, sl, su, su);
    if (sl != NULL) {
        ret = -1;
        goto exit_fn;
    }
    sl = calloc(1, sizeof(*sl));
    sl->su = su;
    DL_APPEND(tm->subscribers, sl);
    subs = calloc(1, sizeof(*subs));
    subs->tm = tm;
    DL_APPEND(su->subs, subs);

exit_fn:
    PORT_UNLOCK
    return ret;
}

int ps_unsubscribe(ps_subscriber_t *su, const u32 topic) {
    int ret = 0;
    topic_map_t *tm;
    subscriber_list_t *sl;
    subscriptions_list_t *subs;

    PORT_LOCK
    tm = fetch_topic(topic);
    if (tm == NULL) {
        ret = -1;
        goto exit_fn;
    }
    DL_SEARCH_SCALAR(tm->subscribers, sl, su, su);
    if (sl == NULL) {
        ret = -1;
        goto exit_fn;
    }
    DL_DELETE(tm->subscribers, sl);
    if (sl) {
        free(sl);
        sl = NULL;
    }
    free_topic_if_empty(tm);
    DL_SEARCH_SCALAR(su->subs, subs, tm, tm);
    if (subs != NULL) {
        DL_DELETE(su->subs, subs);
        free(subs);
        subs = NULL;
    }
exit_fn:
    PORT_UNLOCK
    return ret;
}

int ps_unsubscribe_many(ps_subscriber_t *su, u32 *subs) {
    int n = 0;

    size_t idx = 0;
    for (idx = 0; idx < sizeof(subs) / sizeof(uint32_t); idx++) {
        if (ps_unsubscribe(su, subs[idx++]) == 0) n++;
    }

    return n;
}

int ps_unsubscribe_all(ps_subscriber_t *su) {
    subscriptions_list_t *s, *ps;
    subscriber_list_t *sl;
    size_t count = 0;

    PORT_LOCK
    s = su->subs;
    while (s != NULL) {
        if (s->tm) {
            DL_SEARCH_SCALAR(s->tm->subscribers, sl, su, su);
            if (sl != NULL) {
                DL_DELETE(s->tm->subscribers, sl);
                free(sl);
                sl = NULL;
                free_topic_if_empty(s->tm);
            }
        }
        ps = s;
        s = s->next;
        if (ps) {
            free(ps);
            ps = NULL;
        }
        count++;
    }
    su->subs = NULL;
    PORT_UNLOCK
    return count;
}

int ps_num_subs(ps_subscriber_t *su) {
    int count;
    subscriptions_list_t *elt;
    DL_COUNT(su->subs, elt, count);
    return count;
}

int ps_get_topic_sticky_msg(u32 topic, ps_msg_t *msg) {
    topic_map_t *tm = NULL;
    int ret = 0;
    PORT_LOCK
    tm = fetch_topic(topic);

    if (tm == NULL) {
        ret = -1;
    } else {
        if (tm->sticky != NULL) {
            memcpy(msg, tm->sticky, sizeof(ps_msg_t));
        } else {
            ret = -2;
        }
    }
    PORT_UNLOCK
    return ret;
}
ps_msg_t *ps_dup_msg(ps_msg_t *msg) {
    if (msg == NULL) return NULL;
    ps_msg_t *dmsg = calloc(1, sizeof(ps_msg_t));
    memcpy(dmsg, msg, sizeof(ps_msg_t));
    return dmsg;
}

static void *pubsub_thread(void *pvargv)
{
    pubsub_thread_mem_t *t = (pubsub_thread_mem_t *)pvargv;
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, __func__, 0, 0, 0);
    subscriber_list_t *sl = NULL;
    size_t ret = 0;
    topic_map_t *tm = fetch_topic(t->Pubsubmsg.topic);
    DL_FOREACH(tm->subscribers, sl) {
    if (push_subscriber_queue(sl->su, &t->Pubsubmsg) == 0) {
            ret++;
        }
    }

    free(t);
    return NULL;
}

int ps_publish(ps_msg_t *msg) {
    topic_map_t *tm = NULL;
    size_t ret = 0;

    if (!msg) return 0;

    u32 topic = msg->topic;

    PORT_LOCK
    tm = fetch_topic(topic);
    if (tm == NULL) {
        tm = create_topic(topic);
    }
    if (tm->sticky != NULL) {
        ps_unref_msg(tm->sticky);
    }
    tm->sticky = ps_ref_msg(msg);

    PORT_UNLOCK
    if (tm != NULL) {
        pubsub_thread_mem_t *t = malloc(sizeof(pubsub_thread_mem_t));
        memcpy(&t->Pubsubmsg, msg, sizeof(ps_msg_t));
        if (0 != pthread_create(&t->PubsubTid, NULL, pubsub_thread, (void*)t)) {
            printf("create async msg thread fail:%s\n", strerror(errno));
            return -1;
        }
    }

    return ret;
}

int ps_subs_count(uint32_t topic) {
    topic_map_t *tm = NULL;
    subscriber_list_t *sl = NULL;
    size_t count = 0;

    PORT_LOCK
    tm = fetch_topic(topic);
    if (tm != NULL) {
        DL_FOREACH(tm->subscribers, sl) { count++; }
    }
    PORT_UNLOCK
    return count;
}
