#ifndef _SYSOBJECT_H_
#define _SYSOBJECT_H_


#include "Common.h"
#include "SysPthread.h"
#include "SysMutex.h"
//#include "SysTimer.h"
//#include "SysSem.h"
#include "SysList.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <mqueue.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define OBJECT_NAME_MAX 32

typedef enum
{
    SYSOBJECT_CLASS_NULL          = 0x00,      /**< The object is not used. */
    SYSOBJECT_CLASS_THREAD        = 0x01,      /**< The object is a thread. */
    SYSOBJECT_CLASS_SEMAPHORE     = 0x02,      /**< The object is a semaphore. */
    SYSOBJECT_CLASS_MUTEX         = 0x03,      /**< The object is a mutex. */
    SYSOBJECT_CLASS_EVENT         = 0x04,      /**< The object is a event. */
    SYSOBJECT_CLASS_MAILBOX       = 0x05,      /**< The object is a mail box. */
    SYSOBJECT_CLASS_TIMER         = 0x06,      /**< The object is a timer. */
    SYSOBJECT_CLASS_COND          = 0x07,      /**< The object is a cond. */
    SYSOBJECT_CLASS_MQUEUE        = 0x08,     /**< The object is a mqueue. */
    SYSOBJECT_CLASS_SHMEM         = 0x09,     /**< The object is a share memory. */       
    SYSOBJECT_CLASS_UNKNOWN       = 0x0d,      /**< The object is unknown. */
} SYSOBJECT_CLASS_TYPE_E;

#define SYSOBJECT_POOL_FLAG_NOLOCK    (1 << 1)

typedef struct  {
    char   name[OBJECT_NAME_MAX];   /**< name of kernel object */
    UINT8  type;    /**< type of kernel object */
    UINT8  flag;    /**< flag of kernel object */
    UINT32 pos;     /**< pos of object in object bitmap */
    struct list_head node;
} SYSOBJECT_T;

typedef struct {
    unsigned long elem_size;
    unsigned long* bitmap;
    unsigned long depth;
    void* objects;
    struct list_head list;
    MUTEX_ID lock;
    unsigned long flag;
} SYSOBJECT_POOL_T;


#define OBJECT_DYNAMIC_ALLOC
//#define OBJECT_STATIC_ALLOC

typedef struct {
    SYSOBJECT_T object;
    pthread_t ntid;
    FUNCPTR entry;  /* execution entry point address for thread */
    FUNCPTR_L entry_leagcy;
    void  *arg[10]; /* arguments */ 
} SysThread_t;

typedef struct {
    SYSOBJECT_T object;
    sem_t  sem;
    bool   named;
    sem_t* sem_named;
    key_t  key;
    INT32  semid;
    bool   ipc;
    UINT32 uInitCnt;
} SysSemaphore_t;

typedef struct {
    SYSOBJECT_T object;
    pthread_mutex_t mutex;
    CHAR strHolder[32];
} SysMutex_t;


typedef struct {
    SYSOBJECT_T object;
    timer_t stTimerId;
    UINT32 uClockType;
} SysTimer_t;

typedef struct {
    SYSOBJECT_T object;
    mqd_t  stMqId;
    UINT32 uMsgMax;
    UINT32 uMsgSize;
} SysMqueue_t;



extern SYSOBJECT_POOL_T object_container[SYSOBJECT_CLASS_UNKNOWN];

const char* SysObject_getname(SYSOBJECT_T* object);

SYSOBJECT_T* SysObject_allocate(SYSOBJECT_CLASS_TYPE_E type, const char *name);

void SysObject_delete (SYSOBJECT_T* object);

INT32 SysObject_number (SYSOBJECT_CLASS_TYPE_E type);

#define object_access_start(type) \
    SYSOBJECT_POOL_T* __pool = &object_container[type]; \
    if (__pool->lock && !(__pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK)) \
        SysMutex_lock(&__pool->lock, (INT32)WAIT_FOREVER); \

#define object_access_end(type) \
    SYSOBJECT_POOL_T* ___pool = &object_container[type]; \
    if (___pool->lock && !(___pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK)) \
        SysMutex_unlock(&___pool->lock); \

#ifdef OBJECT_STATIC_ALLOC

#define for_each_object(object, type) \
    UINT32 _bit; \
    SYSOBJECT_POOL_T* _pool = &object_container[type]; \
    for ((_bit) = find_first_bit((_pool->bitmap), (_pool->depth)); \
         ((_bit) < (_pool->depth)) && \
         (object = (typeof(*object)*)((char*)_pool->objects + _bit * _pool->elem_size)); \
         (_bit) = find_next_bit((_pool->bitmap), (_pool->depth), (_bit) + 1)) \

#else 

#define for_each_object(object, type) \
    SYSOBJECT_T* _object; \
    SYSOBJECT_POOL_T* _pool = &object_container[type]; \
    for (_object = list_first_entry(&_pool->list, typeof(*_object), node); \
         (&_object->node != (&_pool->list)) && \
         (object = (typeof(*object)*)_object);   \
         _object = list_next_entry(_object, node)) \

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OBJECT_H__ */

