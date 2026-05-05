#include <stdarg.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <syscall.h>
#include <errno.h>
#include "SysObject.h"
#include "Logger.h"
#include "SysPthread.h"
#include "SysBitops.h"


#define STACK_ALIGN_BYTES   (64)     

#define for_each_pthread(pthread) \
    for_each_object (pthread, SYSOBJECT_CLASS_THREAD)
    
/* function wrapper structure */
typedef struct
{
    FUNCPTR entry;  /* execution entry point address for thread */
    FUNCPTR_L entry_leagcy;

    void  *arg[10]; /* arguments */
} sys_func_t;


VOID* SysPthread_self_tcb(VOID)
{
    return NULL;
}

VOID* SysPthread_find_tcb_byname (const CHAR* strName)
{
    return NULL;
}

static inline INT32 SysPthread_map_priority(UINT8 priority, UINT8 policy)
{
    INT32   max_priority = 0;
    INT32   min_priority = 0;

    if (priority > TASK_PRIORITY_15)
        priority = TASK_PRIORITY_15;

    /* get the allowable priority range for the scheduling policy */        
    min_priority = sched_get_priority_min(policy);
    if (min_priority == -1) 
        return -EINVAL; 
    
    max_priority = sched_get_priority_max(policy);
    if (max_priority == -1)
        return -EINVAL; 
        
    return ((max_priority - min_priority) / (TASK_PRIORITY_15 + 1)) * priority;
}

static INT32 SysPthread_set_attribute(pthread_attr_t *attr, 
                        INT32 priority, UINT32 stacksize, INT32 policy)
{
    struct  sched_param  params;
        
    if (pthread_attr_init(attr)) 
        return -EINVAL;
    
    if (pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED)) 
        goto failed;
    
    /* use the round robin scheduling algorithm */
    if (pthread_attr_setschedpolicy(attr, policy)) 
        goto failed;
    
    /* set the thread to be detached */
    if (pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED)) 
        goto failed;
    
    /* first get the scheduling parameter, then set the new priority */
    if (pthread_attr_getschedparam(attr, &params)) 
        goto failed;
    
    params.sched_priority = priority;    
    if (pthread_attr_setschedparam(attr, &params))
        goto failed;
        
    if (stacksize < PTHREAD_STACK_MIN)
        stacksize = PTHREAD_STACK_MIN;
    
    if (pthread_attr_setstacksize(attr, stacksize)) 
        goto failed;
    
    return 0;

failed:
    pthread_attr_destroy(attr);
    return -EINVAL;
}

static VOID *SysPthread_id2handle(TASK_ID uTskId)
{
    SysThread_t *pThread = NULL;
    SysThread_t *pList = NULL;

    object_access_start(SYSOBJECT_CLASS_THREAD);

    for_each_pthread (pList) {
        if (pList->ntid == uTskId) {
            pThread = pList;
            break;
        }
    }

    object_access_end(SYSOBJECT_CLASS_THREAD);

    return pThread;
}

void SysPthread_delete (TASK_ID sys_thread)
{
    SYSOBJECT_T* object = (SYSOBJECT_T*)SysPthread_id2handle(sys_thread);

    if (object) {
        SysObject_delete(object);
        pthread_exit(NULL);
    } else {
        LOG_ERROR("taskid(0x%lx) is not exist\n", sys_thread);
    }
}

INT32 SysPthread_number(void)
{
    return SysObject_number (SYSOBJECT_CLASS_THREAD);
}

void SysPthread_schdule(void)
{
    return;
}

void SysPthread_set_priority(TASK_ID sys_thread, UINT32 priority)
{
    INT32 policy, map_priority;
    struct sched_param param;

    pthread_getschedparam(sys_thread, &policy, &param);
    
    map_priority = SysPthread_map_priority(priority, policy);   
    if (map_priority >= 0) {
        param.sched_priority = map_priority;
        pthread_setschedparam(sys_thread, policy, &param);
    }       
}


/* API for app compatible */
static void SysPthread_wrapper(void *arg)
{
    SysThread_t* thread = (SysThread_t*)arg;  

    prctl(PR_SET_NAME, SysObject_getname ((SYSOBJECT_T*)thread));

    thread->entry (thread->arg[0], thread->arg[1], thread->arg[2], thread->arg[3], thread->arg[4], \
        thread->arg[5], thread->arg[6], thread->arg[7], thread->arg[8], thread->arg[9]);

    SysPthread_delete ((TASK_ID)thread->ntid);           
}

static void SysPthread_wrapper_leagcy(void *arg)
{
    SysThread_t* thread = (SysThread_t*)arg;  

    prctl(PR_SET_NAME, SysObject_getname ((SYSOBJECT_T*)thread));

    thread->entry_leagcy (thread->arg[0]);

    SysPthread_delete ((TASK_ID)thread->ntid);               
}

INT32 SysPthread_create_policy(TASK_ID *pStTid, CHAR *strTaskName, UINT32 uPriority, UINT32 uStackSize,
        FUNCPTR pFunc, UINT8 policy,
        unsigned args, ...)
{
    va_list ap;
    INT32 ret, index = 0;
    INT32 map_priority; 
    pthread_attr_t  attr;   
    SysThread_t* thread;
    const char* thread_name;

    if (!pFunc || args >= 10) {
        LOG_ERROR ("Invaild params pFunc %p or args %d\n", 
            pFunc, args);
        return -EINVAL;
    }   

    thread_name = strTaskName ? strTaskName : "anonymous_thread";
    thread = (SysThread_t*)SysObject_allocate (SYSOBJECT_CLASS_THREAD, thread_name);
    if (!thread) {
        LOG_ERROR ("Failed to allocate thread object\n");
        return -ENOSPC;         
    }

    thread->entry = pFunc;

    va_start(ap, args);
    
    for (index = 0; index < args; index++)  
       thread->arg[index] = va_arg(ap, void *);        

    va_end(ap);

    map_priority = SysPthread_map_priority(uPriority, policy);
    if (map_priority < 0) {
        LOG_ERROR("pthread map task priority failed\n");
        return -EINVAL;
    }

    ret = SysPthread_set_attribute(&attr, map_priority, 
                    ROUND_UP (uStackSize, STACK_ALIGN_BYTES), policy);
    if (ret < 0) {
        LOG_ERROR("pthread setup task attribute failed\n");
        return ret;
    }

    ret = pthread_create(&thread->ntid, &attr, (void *(*)(void *))SysPthread_wrapper, thread);
    if (ret) {
        LOG_ERROR ("pthread_create failed - errno[%d:%s]\n", errno, strerror(errno));
        return ret;
    }

    if (pStTid)
        *pStTid = (TASK_ID)thread->ntid; 

    pthread_attr_destroy(&attr);

    return 0;
}        

INT32 SysPthread_create(TASK_ID *pStTid, CHAR *strTaskName, UINT32 uPriority, UINT32 uStackSize,
        FUNCPTR pFunc,
        unsigned args, ...)
{
    va_list ap;
    INT32 ret, index = 0;
    INT32 map_priority; 
    pthread_attr_t  attr;   
    SysThread_t* thread;
    const char* thread_name;

    if (!pFunc || args > 10) {
        LOG_ERROR ("Invaild params pFunc %p or args %d\n", 
            pFunc, args);
        return -EINVAL;
    }

    thread_name = strTaskName ? strTaskName : "anonymous_thread";
    thread = (SysThread_t*)SysObject_allocate (SYSOBJECT_CLASS_THREAD, thread_name);
    if (!thread) {
        LOG_ERROR ("Failed to allocate thread object\n");
        return -ENOSPC;         
    }

    thread->entry = pFunc;

    va_start(ap, args);    
    
    for (index = 0; index < args; index++)  
       thread->arg[index] = va_arg(ap, void *);        

    va_end(ap);

    map_priority = SysPthread_map_priority(uPriority, SCHED_OTHER);
    if (map_priority < 0) {
        LOG_ERROR("pthread map task priority failed\n");
        return -EINVAL;
    }
                            
    ret = SysPthread_set_attribute(&attr, map_priority, 
                    ROUND_UP (uStackSize, STACK_ALIGN_BYTES), SCHED_OTHER);
    if (ret < 0) {
        LOG_ERROR("pthread setup task attribute failed\n");
        return ret;
    }

    ret = pthread_create(&thread->ntid, &attr, (void *(*)(void *))SysPthread_wrapper, thread);
    if (ret) {
        LOG_ERROR ("pthread_create failed - errno[%d:%s]\n", errno, strerror(errno));
        return ret;
    }

    if (pStTid)
        *pStTid = (TASK_ID)thread->ntid; 

    pthread_attr_destroy(&attr);

    return 0;
}        

INT32 SysPthread_create_leagcy_policy(TASK_ID *pStTid, CHAR *strTaskName, UINT32 uPriority, UINT32 uStackSize,
        FUNCPTR_L pFunc, UINT8 policy,
        void* arg)
{
    INT32 ret;
    INT32 map_priority; 
    pthread_attr_t  attr;   
    SysThread_t* thread;
    const char* thread_name;

    if (!pFunc) {
        LOG_ERROR ("Invaild params pFunc %p\n", 
            pFunc);
        return -EINVAL;
    }   

    thread_name = strTaskName ? strTaskName : "anonymous_thread";
    thread = (SysThread_t*)SysObject_allocate (SYSOBJECT_CLASS_THREAD, thread_name);
    if (!thread) {
        LOG_ERROR ("Failed to allocate thread object\n");
        return -ENOSPC;         
    }
    
    thread->entry_leagcy = pFunc;
    thread->arg[0] = arg;
    
    map_priority = SysPthread_map_priority(uPriority, policy);
    if (map_priority < 0) {
        LOG_ERROR("pthread map task priority failed\n");
        return -EINVAL;
    }
                            
    ret = SysPthread_set_attribute(&attr, map_priority, 
                    ROUND_UP (uStackSize, STACK_ALIGN_BYTES), policy);
    if (ret < 0) {
        LOG_ERROR("pthread setup task attribute failed\n");
        return ret;
    }

    ret = pthread_create(&thread->ntid, &attr, (void *(*)(void *))SysPthread_wrapper_leagcy, thread);
    if (ret) {
        LOG_ERROR ("pthread_create failed - errno[%d:%s]\n", errno, strerror(errno));
        return ret;
    }

    if (pStTid)
        *pStTid = (TASK_ID)thread->ntid; 

    pthread_attr_destroy(&attr);

    return 0;
}        

INT32 SysPthread_create_leagcy(TASK_ID *pStTid, CHAR *strTaskName, UINT32 uPriority, UINT32 uStackSize,
        FUNCPTR_L pFunc,
        void* arg)
{
    return SysPthread_create_leagcy_policy(pStTid, strTaskName, 
                    uPriority, uStackSize, pFunc, SCHED_OTHER, arg);
}

INT32 SysPthread_join(TASK_ID stTaskId)
{
    if (!stTaskId) {
        LOG_ERROR ("Invaild pStTid(0x%lx)\n", stTaskId);
        return -EINVAL;
    }

    return pthread_join(stTaskId, NULL);
}

INT32 SysPthread_cancel(TASK_ID stTaskId)
{
    if (!stTaskId) {
        LOG_ERROR ("Invaild pStTid(0x%lx)\n", stTaskId);
        return -EINVAL;
    }

    if (NULL == SysPthread_id2handle(stTaskId)) {
        LOG_WARNING ("pthread is already unexit\n");
        return 0;
    }

    return pthread_cancel(stTaskId);
}

INT32 SysPthread_verify(TASK_ID *pStTid)
{
    if (!pStTid) {
        LOG_ERROR ("Invaild pStTid(%p)\n", pStTid);
        return -EINVAL;
    }

    if (!*pStTid) {
        LOG_ERROR ("Invaild *pStTid(0x%lx)\n", *pStTid);
        return -EINVAL;
    }

    if (NULL == SysPthread_id2handle(*pStTid)) {
        return -1;
    }

    return 0;
}

INT32 SysPthread_suspend(TASK_ID *pStTid)
{
    if (!pStTid) {
        LOG_ERROR ("Invaild pStTid(%p)\n", pStTid);
        return -EINVAL;
    }

    if (!*pStTid) {
        LOG_ERROR ("Invaild *pStTid(0x%lx)\n", *pStTid);
        return -EINVAL;
    }

    return pthread_kill(*pStTid, SIGSTOP); 
}

TASK_ID SysPthread_self(VOID)
{
    return (TASK_ID)pthread_self();
}

INT32 SysPthread_reume(TASK_ID *pStTid)
{
    if (!pStTid) {
        LOG_ERROR ("Invaild pStTid(%p)\n", pStTid);
        return -EINVAL;
    }

    if (!*pStTid) {
        LOG_ERROR ("Invaild *pStTid(0x%lx)\n", *pStTid);
        return -EINVAL;
    }


    return pthread_kill(*pStTid, SIGCONT);
}

INT32 SysPthread_get_name( CHAR *strName, UINT16 uNameLen)
{
    if (!strName)
        return -EINVAL;
        
    prctl(PR_GET_NAME, (unsigned long)strName);
    
    return 0;   
}

INT32 SysPthread_set_name( CHAR *strName)
{
    if (!strName)
        return -EINVAL;

    prctl(PR_SET_NAME, (unsigned long)strName);
    
    return 0;
}

void SysPthread_sleep(UINT32 ms)
{
    usleep(ms * 1000);      
}

void SysPthread_sleep_us(UINT32 us)
{
    usleep(us);     
}

void SysPthread_suspend_all(void)
{
}

void SysPthread_resume_all(void)
{
}

INT32 SysPthread_setaffinity(TASK_ID stTaskId, ULONG uCpuset)
{
    INT32 ret;
    ULONG bit = 0;
    cpu_set_t cpuset;

    if (!stTaskId) {
        LOG_ERROR("Invaild params stTaskId(0x%lx)\n", stTaskId);
        return -EINVAL;
    }

    CPU_ZERO(&cpuset);
    
    for_each_bit(bit, &uCpuset, sizeof (uCpuset) * 8) 
        CPU_SET(bit, &cpuset);
        
    ret = pthread_setaffinity_np(stTaskId, sizeof(cpu_set_t), &cpuset);
    if (ret) {
        LOG_ERROR ("Failed to bind taskid(0x%lx) with cpuset 0x%lx, ret %d - errno[%d:%s]",
            stTaskId, uCpuset, ret, errno, strerror(errno));
        return ret;
    }

    ret = pthread_getaffinity_np(stTaskId, sizeof(cpu_set_t), &cpuset);     
    if (ret) {
        LOG_ERROR ("Failed to get affinity of taskid(0x%lx), ret %d - errno[%d:%s]",
            stTaskId, ret, errno, strerror(errno));
        return ret; 
    }

    for_each_bit(bit, &uCpuset, sizeof (uCpuset) * 8) {
        if (CPU_ISSET(bit, &cpuset))    {
            LOG_DEBUG ("Success bind taskid(0x%lx) with cpu %ld\n", 
                stTaskId, bit);               
        }
    }

    return 0;       
}

INT32 SysPthread_getaffinity(TASK_ID stTaskId, ULONG* uCpuset)
{
    INT32 ret;
    ULONG bit;
    cpu_set_t cpuset;   

    if (!uCpuset || !stTaskId) {
        LOG_ERROR ("Invaild params uCpuset(%p) stTaskId(0x%lx)\n", uCpuset, stTaskId);
        return -EINVAL;
    }

    ret = pthread_getaffinity_np(stTaskId, sizeof(cpu_set_t), &cpuset);     
    if (ret) {
        LOG_ERROR ("Failed to getaffinity of taskid(0x%lx), ret %d - errno[%d:%s]",
            stTaskId, ret, errno, strerror(errno));
        return ret; 
    }

    *uCpuset = 0;

    for (bit = 0; bit < sizeof (ULONG) * 8; bit++) {
        if (CPU_ISSET(bit, &cpuset))
            *uCpuset |= BIT (bit);      
    }
    return 0;
}

/*
 * @brief      获取线程ID
 * @return     成功返回线程ID,错误返回0
 */
UINT32 SysPthread_get_id(VOID)
{
    return syscall(SYS_gettid);
}