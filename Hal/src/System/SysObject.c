#include "SysObject.h"

#define OBJ_CONTAINER_LIST_INIT(c) {&(object_container[c].list), &(object_container[c].list)}

#ifdef OBJECT_STATIC_ALLOC
#define DEFINED_OBJECT_POOL(name, type, depth) \
	static unsigned long name##_bitmap[BITS_TO_LONGS(depth)]; \
    static type name##_objects[depth]; 
DEFINED_OBJECT_POOL (thread, SysThread_t, CONFIG_THREAD_POOL_DEPTH);
DEFINED_OBJECT_POOL (mutex, SysMutex_t, CONFIG_MUTEX_POOL_DEPTH);
DEFINED_OBJECT_POOL (timer, SysTimer_t, CONFIG_TIMER_POOL_DEPTH);
DEFINED_OBJECT_POOL (mqueue, SysMqueue_t, CONFIG_MQUEUE_POOL_DEPTH);
DEFINED_OBJECT_POOL (semaphore, SysSemaphore_t, CONFIG_SEMAPHORE_POOL_DEPTH);
#endif

SYSOBJECT_POOL_T object_container[SYSOBJECT_CLASS_UNKNOWN] = {
    [SYSOBJECT_CLASS_THREAD] = {
        .lock       = 0,
        .flag       = 0,
        .elem_size  = sizeof (SysThread_t),
#ifdef OBJECT_STATIC_ALLOC      
        .bitmap  = thread_bitmap,
        .objects = thread_objects,
        .depth   = CONFIG_THREAD_POOL_DEPTH,    
#else       
        .list    = OBJ_CONTAINER_LIST_INIT(SYSOBJECT_CLASS_THREAD),
#endif      
    },
    [SYSOBJECT_CLASS_SEMAPHORE] = {
        .lock       = 0,
        .flag       = 0,
        .elem_size  = sizeof (SysSemaphore_t),
#ifdef OBJECT_STATIC_ALLOC              
        .bitmap  = semaphore_bitmap,
        .objects = semaphore_objects,
        .depth   = CONFIG_SEMAPHORE_POOL_DEPTH,
#else       
        .list    = OBJ_CONTAINER_LIST_INIT(SYSOBJECT_CLASS_SEMAPHORE),   
#endif      
    },
    [SYSOBJECT_CLASS_MUTEX] = {
        .lock       = 0,
        .flag       = SYSOBJECT_POOL_FLAG_NOLOCK,
        .elem_size  = sizeof (SysMutex_t),
#ifdef OBJECT_STATIC_ALLOC                      
        .bitmap  = mutex_bitmap,
        .objects = mutex_objects,
        .depth   = CONFIG_MUTEX_POOL_DEPTH,
#else       
        .list    = OBJ_CONTAINER_LIST_INIT(SYSOBJECT_CLASS_MUTEX),             
#endif      
    }, 
    [SYSOBJECT_CLASS_TIMER] = {
        .lock       = 0,
        .flag       = 0,
        .elem_size  = sizeof (SysTimer_t),
#ifdef OBJECT_STATIC_ALLOC                                              
        .bitmap  = timer_bitmap,
        .objects = timer_objects,
        .depth   = CONFIG_TIMER_POOL_DEPTH,
#else 
        .list    = OBJ_CONTAINER_LIST_INIT(SYSOBJECT_CLASS_TIMER),                                     
#endif
    },  
    [SYSOBJECT_CLASS_MQUEUE] = {
        .lock       = 0,
        .flag       = 0,
        .elem_size  = sizeof (SysMqueue_t),
#ifdef OBJECT_STATIC_ALLOC                                              
        .bitmap  = mqueue_bitmap,
        .objects = mqueue_objects,
        .depth   = CONFIG_MQUEUE_POOL_DEPTH,
#else 
        .list    = OBJ_CONTAINER_LIST_INIT(SYSOBJECT_CLASS_MQUEUE),                                     
#endif
    },

};

SYSOBJECT_T* SysObject_allocate(SYSOBJECT_CLASS_TYPE_E type, const char *name)
{
    __maybe_unused INT32 pos;
    SYSOBJECT_T* object;
    SYSOBJECT_POOL_T* pool;

    if (!name || type >= SYSOBJECT_CLASS_UNKNOWN) {
        printf ("%s:Invaild params type %d, name %p\n",__func__, type, name);
        return NULL;
    }

    pool = &object_container[type];

    if (!pool->lock && !(pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK)) {
        pos = SysMutex_create(&pool->lock, MUTEX_NORMAL);
        if (pos) {
            printf ("%s:Failed to create mutex for object pool %d\n",__func__, type);
            return NULL;
        }
    }

#ifdef OBJECT_STATIC_ALLOC
    if (!(pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK))
        SysMutex_lock(&pool->lock, (INT32)WAIT_FOREVER);

    /* allocate from object pool */
    pos = bitmap_find_free_region (pool->bitmap, pool->depth, 0);
    if (pos < 0) {
        printf ("%s: Can not allocate object (type %d) from object pool (depth %ld)\n",
            __func__, type, pool->depth);
        return NULL;
    }

    if (!(pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK))
        SysMutex_unlock(&pool->lock);

    object = (SYSOBJECT_T*)((char*)pool->objects + pos * pool->elem_size);
#else 
    object = (SYSOBJECT_T*)malloc (pool->elem_size);
    if (!object) {
        printf ("%s:Can not allocate memory for object (type %d, size 0x%lx)\n", 
            __func__, type, pool->elem_size);
        return NULL;
    }
#endif
    memset (object, 0, pool->elem_size);

    object->type = type;
    object->flag = 0;
#ifdef OBJECT_STATIC_ALLOC  
    object->pos  = pos;
#endif

    strncpy (object->name, name, OBJECT_NAME_MAX - 1);

#ifdef OBJECT_DYNAMIC_ALLOC
    if (!(pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK))
        SysMutex_lock(&pool->lock, (INT32)WAIT_FOREVER);

    list_add_tail (&object->node, &pool->list);
    pool->depth++;

    if (!(pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK))
        SysMutex_unlock(&pool->lock);  
#endif

    return object;
}

void SysObject_delete (SYSOBJECT_T* object)
{
    SYSOBJECT_POOL_T* pool;

    if (!object || object->type >= SYSOBJECT_CLASS_UNKNOWN)
        return;
    
    pool = &object_container[object->type]; 

    if (!(pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK))
        SysMutex_lock(&pool->lock, (INT32)WAIT_FOREVER);

#ifdef OBJECT_STATIC_ALLOC  
    test_and_clear_bit(object->pos, pool->bitmap);
#else
    list_del (&object->node);
    pool->depth--;

    free (object);
#endif  

    if (!(pool->flag & SYSOBJECT_POOL_FLAG_NOLOCK))
        SysMutex_unlock(&pool->lock);  
}

INT32 SysObject_number (SYSOBJECT_CLASS_TYPE_E type)
{
    SYSOBJECT_POOL_T* pool;

    if (type >= SYSOBJECT_CLASS_UNKNOWN)
        return -22;
    
    pool = &object_container[type]; 

#ifdef OBJECT_STATIC_ALLOC
    return bitmap_weight(pool->bitmap, pool->depth);
#else 
    return pool->depth;
#endif
}

const char* SysObject_getname(SYSOBJECT_T* object)
{
    if (!object)
        return NULL;

    return (const char*)object->name;
}


