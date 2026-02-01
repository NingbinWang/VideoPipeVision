#ifndef _SYSSEM_H_
#define _SYSSEM_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/** 空信号量 */
#define SEM_EMPTY               (0)
/** 满信号量 */
#define SEM_FULL                (1)

/* ========================================================================== */
/*                             Type Declaration                               */
/* ========================================================================== */

/**
 *  @struct SYS_SEMAPHORE_STATE_T
 *  @brief  Semaphore state, use for debug
 */
typedef struct {
    CHAR    strName[32];         /**< 信号量名称 */
    UINT32  uMode;               /**< 模式 */
    UINT32  uInitCount;          /**< 初始化的值 */
    UINT32  uCurCount;           /**< 当前的值 */
    UINT32  uTakeCount;          /**< 获取次数 */
    UINT32  uGiveCount;          /**< 给与次数 */
    UINT32  uTakeFailedCount;    /**< 获取失败次数 */
    UINT32  uTakeTimeoutCount;   /**< 获取超时次数 */
    UINT32  uReserved;           /**< 保留 */
} SYS_SEMAPHORE_STATE_T;

/** SEM_ID 仅仅一个指针值，具体指向什么各平台内部处理 */
typedef LONG  SEM_ID;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * @brief      创建无名信号量
 * @param[in]  pStSem        信号量句柄指针
 * @param[in]  strName       信号量名称
 * @param[in]  uInitValue    信号量初始值
 * @return     成功返回 0    错误返回 其他, 参考ERROR_CODE_E
 */
INT32 SysSem_create(SEM_ID *pStSem, const CHAR *strName, UINT32 uInitValue);

/**
 * @brief      创建有名信号量
 * @param[in]  pStSem        信号量句柄指针
 * @param[in]  strName       信号量名称
 * @param[in]  uInitValue    信号量初始值
 * @return     成功返回 0    错误返回 其他, 参考ERROR_CODE_E
 */
INT32 SysSem_create_named(SEM_ID *pStSem, const CHAR *strName, UINT32 uInitValue);

/**
 * @brief      释放一个信号量资源
 * @param[in]  pStSem 信号量句柄
 * @return     成功返回 0 错误返回 其他, 参考ERROR_CODE_E
 */
INT32 SysSem_post(SEM_ID *pStSem);

/**
 * @brief      获取一个信号量资源
 * @param[in]  pStSem 信号量句柄
 * @param[in]  uWaitTime 阻塞时长
 * @return     成功返回 0 错误返回 其他, 参考ERROR_CODE_E
 */
INT32 SysSem_wait(SEM_ID *pStSem, UINT32 uWaitTime);

/**
 * @brief      销毁信号量
 * @param[in]  pStSem 信号量句柄
 * @return     成功返回 0 错误返回 其他, 参考ERROR_CODE_E
 */
INT32 SysSem_destroy(SEM_ID *pStSem);

/**
 *  @brief      获取当前信号量计数/Get current semaphore count
 *  @param[in]  pStSem   先前创建的信号量对象的句柄/
 *                       Handle of a previously-created Semaphore instance object
 *  @return     当前信号量计数/Current semaphore count
 */
INT32 SysSem_get_count(SEM_ID *pStSem);

/**
 *  @brief      获取要创建的信号量对象的数量/
 *              Get number of semaphore object be created
 *  @return     创建的信号量对象的数量/
 *              Number of semaphore object be created
 */
INT32 SysSem_number (VOID);

/**
 * @brief       获取指定信号量状态 
 * @param[in]   pStSem      信号量句柄
 * @param[out]  pState      指向信号量状态结构体的指针, @ref SYS_SEMAPHORE_STATE_T 
 * @return      返回 0 表示成功，负值表示失败
 */
INT32 SysSem_state(SEM_ID *pStSem, SYS_SEMAPHORE_STATE_T *pState);

/**
 * @brief       获取所有信号量的状态
 * @param[out]  pState      指向信号量状态结构体数组的指针, @ref SYS_SEMAPHORE_STATE_T 
 * @return      成功返回已创建信号量的个数，返回负值表示不成功
 */
INT32 SysSem_status(SYS_SEMAPHORE_STATE_T *pState);

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif

