

#include <stdio.h>
#include <stdlib.h>

#include "cvi_eventhub.h"
#include "pubsub.h"

void CVI_EVENTHUB_Init() { ps_init(); }

void CVI_EVENTHUB_DeInit() { clean_all_subscribers(); }

int CVI_EVENTHUB_RegisterTopic(uint32_t topic) { return register_topic(topic); }

int CVI_EVENTHUB_UnRegisterTopic(uint32_t topic) {
    return unregister_topic(topic);
}

int CVI_EVENTHUB_GetEventHistory(uint32_t topic, ps_msg_t *msg) {
    return ps_get_topic_sticky_msg(topic, msg);
}

CVI_EVENTHUB_Subscriber_t *to_CVI_EVENTHUB_Subscriber(CVI_MW_PTR argv) {
    return (CVI_EVENTHUB_Subscriber_t *)argv;
}

int CVI_EVENTHUB_CreateSubscriber(CVI_EVENTHUB_Subscriber_t *pstSubscriber,
                                CVI_MW_PTR *ppSubscriber) {
    int ret = 0;
    ps_subscriber_t *su = NULL;

    su = ps_create_subscriber();
    if (!su) {
        ret = -1;
        goto err_create;
    }
    pstSubscriber->argv = (void *)su;
    if (pstSubscriber->new_msg_cb) ps_set_cb(su, pstSubscriber->new_msg_cb);
    *ppSubscriber = (void *)pstSubscriber;
err_create:
    return ret;
}

int CVI_EVENTHUB_DestroySubscriber(CVI_MW_PTR ppSubscriber) {
    CVI_EVENTHUB_Subscriber_t *s = to_CVI_EVENTHUB_Subscriber(ppSubscriber);
    ps_subscriber_t *su = to_ps_subscriber(s->argv);
    if (su) {
        ps_free_subscriber(su);
        su = NULL;
    }

    if (s) {
        free(s);
        s = NULL;
    }
    ppSubscriber = NULL;
    return 0;
}

int CVI_EVENTHUB_Subcribe(CVI_MW_PTR pSubscriber, CVI_TOPIC_ID topic) {
    CVI_EVENTHUB_Subscriber_t *s = to_CVI_EVENTHUB_Subscriber(pSubscriber);
    ps_subscriber_t *su = to_ps_subscriber(s->argv);

    return ps_subscribe(su, topic);
}

int CVI_EVENTHUB_UnSubcribe(CVI_MW_PTR pSubscriber, CVI_TOPIC_ID topic) {
    CVI_EVENTHUB_Subscriber_t *s = to_CVI_EVENTHUB_Subscriber(pSubscriber);
    ps_subscriber_t *su = to_ps_subscriber(s->argv);
    return ps_unsubscribe(su, topic);
}

int CVI_EVENTHUB_Publish(CVI_EVENT_S *pEvent) { return ps_publish(pEvent); }
