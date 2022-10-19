#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cvi_eventhub.h"

typedef CVI_EVENT_S ps_msg_t;
//#define PS_USE_GETTIMEOFDAY // Use gettimeofday instead of monotonic
// clock_gettime

typedef struct ps_subscriber_s ps_subscriber_t;  // Private definition

/**
 * @brief ps_init initializes the publish/subscribe internal context.
 */
void ps_init(void);

/**
 * @brief ps_ref_msg increments the message reference counter
 *
 * @param msg
 * @return ps_msg_t*
 */
ps_msg_t *ps_ref_msg(ps_msg_t *msg);
/**
 * @brief ps_set_cb set cb of subscriber
 *
 * @param su  new_msg_cb
 */
void ps_set_cb(ps_subscriber_t *su,
               int (*new_msg_cb)(void *argv, ps_msg_t *msg));
/**
 * @brief ps_unref_msg decrease the message reference counter
 *
 * @param msg
 */
void ps_unref_msg(ps_msg_t *msg);

/**
 * @brief ps_msg_set_topic sets message topic
 *
 * @param msg message to set the topic
 * @param topic string with the topic
 */
void ps_msg_set_topic(ps_msg_t *msg, const uint32_t topic);

/**
 * @brief ps_msg_set_rtopic sets a response topic for the message
 *
 * @param msg message to set the reply topic
 * @param rtopic string with the path to reply
 */
void ps_msg_set_rtopic(ps_msg_t *msg, const uint32_t rtopic);

/**
 * @brief ps_new_subscriber create a new subscriber to a path.
 *
 * @param queue_size of messages to store
 * @param subs string paths to subscribe for messages (see STRLIST macro)
 * @return ps_subscriber_t*
 */
ps_subscriber_t *ps_new_subscriberi(uint32_t *subs);

/**
 * @brief ps_free_subscriber frees all data from the subscriber, including
 * message queue.
 *
 * @param s subscriber to free.
 */
void ps_free_subscriber(ps_subscriber_t *s);

/**
 * @brief ps_get get message from subscribers
 *
 * @param su subscriber from where to check if new messages
 * @param timeout in miliseconds to wait (-1 = waits forever, 0 = don't block).
 * @return ps_msg_t* message value or null if timeout reached.
 */
ps_msg_t *ps_get(ps_subscriber_t *su, int64_t timeout);

/**
 * @brief ps_subscribe adds topic to the subscriber instance
 *
 * @param su subscriber instance
 * @param topic string path of topic to subscribe
 * @return status (-1 = Error, 0 = Ok)
 */
int ps_subscribe(ps_subscriber_t *su, const uint32_t topic);

/**
 * @brief ps_subscribe_many adds several topics to the subscriber instance
 *
 * @param su subscriber instance
 * @param subs strings with the paths to subscribe
 * @return the number of unsubscribed topics.
 */
int ps_subscribe_many(ps_subscriber_t *su, uint32_t *subs);

/**
 * @brief ps_unsubscribe removes one topic from the subscribe instance
 *
 * @param su subscriber instance
 * @param topic string path to remove
 * @return state (-1 = Error, 0 = Ok)
 */
int ps_unsubscribe(ps_subscriber_t *su, const uint32_t topic);

/**
 * @brief ps_unsubscribe_many removes several topic from the subscribe instance
 *
 * @param su subscriber instance
 * @param subs strings with the paths to subscribe (see STRLIST macro)
 * @return the number of unsubscribed topics
 */
int ps_unsubscribe_many(ps_subscriber_t *su, uint32_t *subs);

/**
 * @brief ps_unsubscribe_all removes all topic from the subscribe instance
 *
 * @param su subscriber instance
 * @return the number of unsubscribed topics
 */
int ps_unsubscribe_all(ps_subscriber_t *su);

int ps_get_topic_sticky_msg(uint32_t topic, ps_msg_t *msg);

/**
 * @brief ps_flush clears all messages pending in the queue
 *
 * @param su subscribe instance
 * @return the number of freed messages
 */
int ps_flush(ps_subscriber_t *su);
/**
 * @brief convert void* to ps_subscriber
 *
 * @param argv void point to subscribe instance
 * @return point of ps_subscriber
 */
ps_subscriber_t *to_ps_subscriber(void *argv);
/**
 * @brief create ps_subscriber
 *
 * @param su point to subscribe instance
 * @param queue_sizee queue sizeof  subscribe instance
 * @return   point of ps_subscriber
 */
#ifdef SUBSCRIBE_QUEUE
ps_subscriber_t *ps_create_subscriber(size_t queue_size);
#else
ps_subscriber_t *ps_create_subscriber(void);
#endif
/**
 * @brief ps_num_subs gives the number of topics subscribed to
 *
 * @param su subscribe instance
 * @return the number of subscribed topics
 */
int ps_num_subs(ps_subscriber_t *su);

/**
 * @brief ps_subs_count gives the number of subscriptors of a topic
 *
 * @param topic string path to count subcriptors
 * @return the number of subcriptors
 */
int ps_subs_count(uint32_t topic);

/**
 * @brief ps_waiting gives the number of messages pending to read from
 * subscriber
 *
 * @param su subscribe instance
 * @return size_t number of subscribed topics
 */
int ps_waiting(ps_subscriber_t *su);

/**
 * @brief ps_overflow gives the number of messages that could not be stored in
 * the queue because it was full. Calling this function resets the overflow
 * counter.
 * @param su subscribe instance
 * @return the number of messages overflowed
 */
int ps_overflow(ps_subscriber_t *su);

/**
 * @brief ps_publish publishes a message
 *
 * @param msg message instance
 * @return the number of subscribers the message was delivered to
 */
int ps_publish(ps_msg_t *msg);

/**
 * @brief ps_call create publishes a message, generate a rtopic and waits for a
 * response.
 *
 * @param msg message instance
 * @param timeout timeout in miliseconds to wait for response (-1 = waits
 * forever)
 * @return ps_msg_t* message response or null if timeout expired
 */
ps_msg_t *ps_call(ps_msg_t *msg, int64_t timeout);

/**
 * @brief ps_wait_one waits one message without creating the subscriber instace
 *
 * @param topic string path of topic to subscribe
 * @param timeout timeout in miliseconds to wait for response (-1 = waits
 * forever)
 * @return ps_msg_t* message response or null if timeout expired
 */
ps_msg_t *ps_wait_one(const char *topic, int64_t timeout);

/**
 * @brief ps_has_topic_prefix returns true if topic of msg starts with prefix
 *
 * @param msg message instance
 * @param pre topic prefix to check
 * @return bool true if message topic starts with prefix
 */
bool ps_has_topic_prefix(ps_msg_t *msg, const char *pre);

/**
 * @brief ps_has_topic_suffix returns true if topic of msg ends with suffix
 *
 * @param msg is the message instance
 * @param suf is the topic suffix to check
 * @return bool true if message topic ends with suffix
 */
bool ps_has_topic_suffix(ps_msg_t *msg, const char *suf);

/**
 * @brief ps_has_topic returns true if topic of msg and topic are equal
 *
 * @param msg is the message instance
 * @param topic is the topic to check
 * @return bool true if message topic and topic are equal
 */
bool ps_has_topic(ps_msg_t *msg, const char *topic);

int ps_stats_live_msg(void);
int ps_stats_live_subscribers(void);
void ps_clean_sticky(const char *prefix);

void clean_all_subscribers(void);
int unregister_topic(const uint32_t topic);
int register_topic(const uint32_t topic);
