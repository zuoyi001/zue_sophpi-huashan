#ifndef CVI_EVENTHUB_H
#define CVI_EVENTHUB_H
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define MSG_PAYLOAD_LEN 128

typedef uint32_t CVI_TOPIC_ID;
typedef void *CVI_MW_PTR;

#define CVI_EVENTHUB_SUBSCRIBE_NAME_LEN 16

typedef struct ps_msg_s {
#ifdef SUBSCRIBE_QUEUE
    uint32_t _ref;  // Ref counter
#endif
    uint32_t topic;  // Message topic
    int32_t arg1;
    int32_t arg2;
    int32_t s32Result;
    uint64_t u64CreateTime;
    uint8_t aszPayload[MSG_PAYLOAD_LEN];
} CVI_EVENT_S;

typedef struct {
    char asName[CVI_EVENTHUB_SUBSCRIBE_NAME_LEN];
    void *argv;
    int32_t (*new_msg_cb)(void *argv, CVI_EVENT_S *msg);
} CVI_EVENTHUB_Subscriber_t;

/**
 * @brief CVI_EVENTHUB_Init init EVENTHUB system
 *
 * @param none
 *
 * @return none
 */
void CVI_EVENTHUB_Init(void);
/**
 * @brief CVI_EVENTHUB_DeInit deinit EVENTHUB system
 *
 * @param none
 *
 * @return none
 */
void CVI_EVENTHUB_DeInit(void);

/**
 * @brief CVI_EVENTHUB_RegisterTopic register a topic in EVENTHUB system(not a must
 * operation)
 *
 * @param CVI_TOPIC_ID topic id
 *
 * @return 0 success / -1 failed
 */

int CVI_EVENTHUB_RegisterTopic(CVI_TOPIC_ID topic);

/**
 * @brief CVI_EVENTHUB_UnRegisterTopic unregister a topic in EVENTHUB system(not a
 * must operation)
 *
 * @param CVI_TOPIC_ID topic id
 *
 * @return 0 success / -1 failed
 */
int CVI_EVENTHUB_UnRegisterTopic(CVI_TOPIC_ID topic);

/**
 * @brief CVI_EVENTHUB_GetEventHistory get the latest msg of  published on the
 * topic
 *
 * @param CVI_TOPIC_ID topic id, CVI_EVENT_S msg point
 *
 * @return 0 success / -1 failed
 */
int CVI_EVENTHUB_GetEventHistory(CVI_TOPIC_ID topic, CVI_EVENT_S *msg);

/**
 * @brief CVI_EVENTHUB_CreateSubscriber create a subscriber
 *
 * @param CVI_EVENTHUB_Subscriber_t *pstSubscriber  CVI_SUBSCRIBER struct point
 *        CVI_MW_PTR *ppSubscriber hanlder of pstSubsciber
 *
 * @return 0 success / -1 failed
 */
int CVI_EVENTHUB_CreateSubscriber(CVI_EVENTHUB_Subscriber_t *pstSubscriber,
                                CVI_MW_PTR *ppSubscriber);

/**
 * @brief CVI_EVENTHUB_DestroySubscriber destroy a subscriber
 *
 * @param CVI_MW_PTR *ppSubscriber hanlder of pstSubsciber
 *
 * @return 0 success / -1 failed
 */
int CVI_EVENTHUB_DestroySubscriber(CVI_MW_PTR ppSubscriber);

/**
 * @brief CVI_EVENTHUB_Subcribe subscriber to the topic
 *
 * @param CVI_MW_PTR *pSubscriber hanlder of pstSubscriber, CVI_TOPIC_ID topic
 * subscribed topic
 *
 * @return 0 success / -1 failed
 */
int CVI_EVENTHUB_Subcribe(CVI_MW_PTR pSubscriber, CVI_TOPIC_ID topic);

/**
 * @brief CVI_EVENTHUB_UnSubcribe unsubscriber to the subscribed topic
 *
 * @param CVI_MW_PTR *pSubscriber hanlder of pstSubscriber, CVI_TOPIC_ID topic
 * subscribed topic
 *
 * @return 0 success / -1 failed
 */
int CVI_EVENTHUB_UnSubcribe(CVI_MW_PTR pSubscriber, CVI_TOPIC_ID topic);

/**
 * @brief CVI_EVENTHUB_Publish publish the event structure
 *
 * @param CVI_EVENT_S *pEvent the event structrue to be published
 *
 * @return 0 success / -1 failed
 */
int CVI_EVENTHUB_Publish(CVI_EVENT_S *pEvent);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
