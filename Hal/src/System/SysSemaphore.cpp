
#include <stdio.h>
#include <fcntl.h> 
#include <errno.h> 
#include <sys/stat.h>

#include <semaphore.h>
#include "Logger.h"
#include "SysSem.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "SysObject.h"



union semun {
    INT                 val;
    struct semid_ds*    buf;
    UINT16*             array;
};

void* _sys_ipc_sem_create (const char* strPath, INT32 proj_id, UINT32 uInitValue)
{
    INT32 ret;
    union semun sem_union;
    SysSemaphore_t* semaphore;

    semaphore = (SysSemaphore_t*)SysObject_allocate (SYSOBJECT_CLASS_SEMAPHORE, "ipc_sem");
    if (!semaphore) {
        LOG_ERROR ("Failed to allocate semphr object\n");
        return NULL;            
    }

    semaphore->key = ftok (strPath, proj_id);       
    if (semaphore->key == -1) {
        LOG_ERROR ("Failed to create ipc key by ftok: %s\n", 
            strerror (errno));
        SysObject_delete((SYSOBJECT_T *)semaphore);           
        return NULL; 
    }
    
    semaphore->semid = semget(semaphore->key, 1, 0666 | IPC_CREAT);
    if (semaphore->semid == -1) {
        LOG_ERROR ("Failed to create get sem: %s\n", 
            strerror (errno));
        SysObject_delete((SYSOBJECT_T *)semaphore);           
        return NULL;     
    }
            
    LOG_DEBUG ("Create ipc semaphore proj_id 0x%x, sem 0x%x !\n",
        proj_id, semaphore->semid);
    
    sem_union.val = 0;
    ret = semctl(semaphore->semid, uInitValue, SETVAL, sem_union);
    if (ret == -1) {
        LOG_ERROR ("Failed to create set sem value: %s\n", 
            strerror (errno));
        SysObject_delete((SYSOBJECT_T *)semaphore);               
        return NULL;
    }

    semaphore->ipc = true;

    return semaphore;       
}

void* _sys_sem_create (const CHAR *strName, UINT32 uInitValue)
{
    INT32 ret;
    const char* name;
    SysSemaphore_t* semaphore;

    name = strName ? strName : "anonymous_semphr";

    semaphore = (SysSemaphore_t*)SysObject_allocate(SYSOBJECT_CLASS_SEMAPHORE, name);
    if (!semaphore) {
        LOG_ERROR ("Failed to allocate semphr object\n");
        return NULL;            
    }

    ret = sem_init(&semaphore->sem, 0, uInitValue);
    if (ret) {
        LOG_ERROR ("Failed to init semaphore, ret %d: %s\n", ret, strerror(errno));     
        SysObject_delete((SYSOBJECT_T*)semaphore);
        return NULL;
    }

    semaphore->named = false;
    semaphore->uInitCnt = uInitValue;

    return semaphore;
}

INT32 _sys_sem_destroy(void* pSem)
{
    sem_t* sem;
    SYSOBJECT_T* object = (SYSOBJECT_T*)pSem;
    SysSemaphore_t* semaphore = (SysSemaphore_t*)pSem;

    if (!semaphore || object->type != SYSOBJECT_CLASS_SEMAPHORE)  {
        LOG_ERROR ("Invaild params for semaphore %p\n",
            semaphore);
        return -EINVAL;
    }

    if (semaphore->ipc)
        goto object_delete;

    sem = semaphore->sem_named ? semaphore->sem_named : &semaphore->sem;

    if (semaphore->sem_named)
        sem_close (sem);
    else 
        sem_destroy (sem);

object_delete:
    SysObject_delete(object);

    return 0;   
}

INT32 _sys_sem_wait (void* pSem, UINT32 uWaitTime)
{
    INT32 ret;
    sem_t* sem;
    UINT64 sec, msec, nsec;
    struct sembuf sem_b;    
    struct timespec ts;
    SYSOBJECT_T* object = (SYSOBJECT_T*)pSem;
    SysSemaphore_t* semaphore = (SysSemaphore_t*)pSem;

    if (!semaphore || object->type != SYSOBJECT_CLASS_SEMAPHORE)  {
        LOG_ERROR ("Invaild params for semaphore %p\n",
            semaphore);
        return -EINVAL;
    }

    if (semaphore->ipc) {
        sem_b.sem_num = 0;
        sem_b.sem_op = -1; //P operation
        sem_b.sem_flg = 0;
        ret = semop(semaphore->semid, &sem_b, 1);
        if (ret == -1) {
            LOG_ERROR ("IPC semaphore pend failed semid 0x%x: %s\n",
                    semaphore->semid, strerror (errno));
            return ret;
        }

        return 0;
    }
    
    sem = semaphore->sem_named ? semaphore->sem_named : &semaphore->sem;

    if (uWaitTime == NO_WAIT)
        return sem_trywait(sem);
    else if (uWaitTime == WAIT_FOREVER)
        return sem_wait(sem);
    else {
#if __GLIBC_MINOR__ >= 30
        /* ts.tv_nsec will overflow if ts.tv_nsec == 1000 * 1000 * 1000 */
        clock_gettime(CLOCK_MONOTONIC, &ts);
#else
        /* ts.tv_nsec will overflow if ts.tv_nsec == 1000 * 1000 * 1000 */
        clock_gettime(CLOCK_REALTIME, &ts);
#endif
        sec  = uWaitTime / 1000;
        msec = uWaitTime % 1000;
        nsec = msec * 1000 * 1000 + ts.tv_nsec;
        
        ts.tv_sec  += (sec + nsec / (1000 * 1000 * 1000));
        ts.tv_nsec = nsec % (1000 * 1000 * 1000);
#if __GLIBC_MINOR__ >= 30
        ret = sem_clockwait(sem, CLOCK_MONOTONIC, &ts);
#else
        ret = sem_timedwait(sem, &ts);
#endif
        if (ret)
            return -ETIMEDOUT;
    }
    return 0;   
}

INT32 _sys_sem_post(void* pSem)
{
    INT32 ret;
    sem_t* sem;
    struct sembuf sem_b;    
    SYSOBJECT_T* object = (SYSOBJECT_T*)pSem;
    SysSemaphore_t* semaphore = (SysSemaphore_t*)pSem;

    if (!semaphore || object->type != SYSOBJECT_CLASS_SEMAPHORE)  {
        LOG_ERROR ("Invaild params for semaphore %p\n",
            semaphore);
        return -EINVAL;
    }
    
    if (semaphore->ipc) {
        sem_b.sem_num = 0;
        sem_b.sem_op  = 1; //P operation
        sem_b.sem_flg = 0;
        ret = semop(semaphore->semid, &sem_b, 1);
        if (ret == -1) {
            LOG_ERROR ("IPC semaphore post failed semid 0x%x: %s\n",
                    semaphore->semid, strerror (errno));
            return ret;
        }

        return 0;
    }

    sem = semaphore->sem_named ? semaphore->sem_named : &semaphore->sem;

    return sem_post(sem);   
}

INT32 SysSem_create_named(SEM_ID *pStSem, const CHAR *strName, UINT32 uInitValue)
{
    SysSemaphore_t* semaphore;

    if (!pStSem || !strName) {
        LOG_ERROR ("Invaild params of pStSem %p or strName %p\n", 
            pStSem, strName);
        return -EINVAL;
    }

    semaphore = (SysSemaphore_t*)SysObject_allocate (SYSOBJECT_CLASS_SEMAPHORE, strName);
    if (!semaphore) {
        LOG_ERROR ("Failed to allocate semphr object\n");
        return -ENOSPC;         
    }       

    semaphore->sem_named = sem_open (strName, O_CREAT | O_RDWR, 666, uInitValue);
    if (semaphore->sem_named == SEM_FAILED) {
        LOG_ERROR ("Failed to open named semaphore [%s]: %s\n", 
            strName, strerror(errno));        
        SysObject_delete((SYSOBJECT_T*)semaphore);
        return -EFAULT;
    }

    semaphore->named = true;
    semaphore->uInitCnt = uInitValue;
    *pStSem = (SEM_ID)semaphore;
    
    return 0;
}

INT32 SysSem_create(SEM_ID *pStSem, const CHAR *strName, UINT32 uInitValue)
{   
    SysSemaphore_t* semaphore;

    if (!pStSem) {
        LOG_ERROR ("Invaild params of pStSem %p\n", pStSem);
        return -EINVAL;
    }

    semaphore = (SysSemaphore_t*)_sys_sem_create (strName, uInitValue);
    if (!semaphore)
        return -ENODEV;

    *pStSem = (SEM_ID)semaphore;

    return 0;
}

INT32 SysSem_destroy(SEM_ID *pStSem)
{
    if (!pStSem) {
        LOG_ERROR ("Invalid pStSem(%p)\n", pStSem);
        return -EINVAL;
    }

    return _sys_sem_destroy((void*)*pStSem);
}

INT32 SysSem_wait(SEM_ID *pStSem, UINT32 uWaitTime)
{
    if (!pStSem) {
        LOG_ERROR ("Invalid pStSem(%p)\n", pStSem);
        return -EINVAL;
    }
    return _sys_sem_wait ((void*)*pStSem, uWaitTime);
}

INT32 SysSem_post(SEM_ID *pStSem)
{
    if (!pStSem) {
        LOG_ERROR ("Invalid pStSem(%p)\n", pStSem);
        return -EINVAL;
    }

    return _sys_sem_post ((void*)*pStSem);
}

INT32 SysSem_number (void)
{
    return SysObject_number (SYSOBJECT_CLASS_SEMAPHORE);
}

INT32 SysSem_get_count(SEM_ID *pStSem)
{
    sem_t* sem;
    INT32 count;
    SYSOBJECT_T* object;
    SysSemaphore_t* semaphore;

    if (!pStSem) {
        LOG_ERROR ("Invalid pStSem(%p)\n", pStSem);
        return -EINVAL;
    }

    object = (SYSOBJECT_T*)(*pStSem);
    semaphore = (SysSemaphore_t*)(*pStSem);

    if (!semaphore || object->type != SYSOBJECT_CLASS_SEMAPHORE)  {
        LOG_ERROR ("Invaild params for semaphore %p\n",
            semaphore);
        return -EINVAL;
    }

    sem = semaphore->sem_named ? semaphore->sem_named : &semaphore->sem;

    sem_getvalue(sem, &count);

    return count;
}

INT32 SysSem_status(SYS_SEMAPHORE_STATE_T* pStatus)
{
    UINT32 uLoopCnt = 0u;
    SysSemaphore_t* pSemaphore = NULL;

    if (!pStatus) 
        return -EINVAL;

    object_access_start(SYSOBJECT_CLASS_SEMAPHORE);

    for_each_object (pSemaphore, SYSOBJECT_CLASS_SEMAPHORE) {
        strcpy(pStatus[uLoopCnt].strName, pSemaphore->object.name);
        pStatus[uLoopCnt].uCurCount = SysSem_get_count((SEM_ID*)&pSemaphore);
        pStatus[uLoopCnt].uInitCount = pSemaphore->uInitCnt;
        uLoopCnt++;
    }

    object_access_end(SYSOBJECT_CLASS_SEMAPHORE);

    return uLoopCnt;
}

