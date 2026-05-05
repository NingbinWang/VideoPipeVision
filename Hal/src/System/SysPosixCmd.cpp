
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/if.h>   
#include <linux/if_ether.h>   
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/un.h>
#include <netdb.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stddef.h>
#include <spawn.h>
#include <sys/wait.h>
#include "SysFile.h"
#include "SysSocket.h"
#include "Logger.h"
#include "Common.h"
#include "SysPosix.h"
#define MAX_PROCESS_NAME_LEN    (32)

static INT32 uPnameSet = false;
static CHAR strPnameMapping[MAX_PROCESS_NAME_LEN];

#define SYS_SOCKET_SAFE_CLOSE(x) \
do { \
    if (-1 != (x)) \
    { \
        (VOID)sockets::socket_close((x)); \
        (x) = -1; \
    } \
}while (0)

#define SYS_CMD_SUN_PATH        "/tmp/SysCmd.sock"
#define CLI_PATH                "/tmp"                      /* +5 for pid = 14 chars */
#define CMD_BUF_MAX_LEN         1024

/* 命令头部 */
typedef struct {
    INT32 uCmdLen; /* 命令长度，包括头部 */
}SYS_CMD_HEADER;

/**@fn         SysPosix_kill
 * @brief      调用系统命令,如果命令的前缀为$，则进程在后台执行
 * @param[in]  iPid 进程ID
 * @return     无
 */
VOID SysPosix_kill(INT32 iPid)
{
    if(iPid <= 0)
    {
        return ;
    }
    kill(iPid, SIGKILL);
}

/**@fn         SysPosix_verify
 * @brief      判断进程是否存在
 * @param[in]  iPid 进程ID
 * @return     存在返回OK，不存在返回ERROR
 */
INT32 SysPosix_verify(INT32 iPid)
{
    INT32 iRet = ERROR;
    if(iPid <= 0)
    {
        return iRet;
    }
    iRet = kill(iPid, 0);
    return iRet;
}

/**@fn         SysPosix_ioctl
 * @brief      底层数据交互接口,IO控制接口
 * @param[in]  iFd      设备句柄
 * @param[in]  uCmd     命令
 * @param[out] pData    数据缓存
 * @return     存在返回OK，不存在返回ERROR
 */
INT32 SysPosix_ioctl(INT32 iFd, UINT32 uCmd,VOID *pData)
{
    return ioctl(iFd, uCmd, pData);
}
#ifndef ANDROID
/**@fn         SysPosix_cmd_call
 * @brief      调用系统命令,如果命令的前缀为$，则进程在后台执行
 * @param[in]  strCmd 系统命令字符串
 * @return     调用失败，返回-1；调用成功，若后台执行返回大于0的子进程号，前台执行则返回system接口返回值
 */
INT32 SysPosix_cmd_call(const CHAR *strCmd)
{
    INT32 iCliFd = -1;
    INT32 iRet = -1;
    INT32 iLen = 0;
    CHAR strCmdBuf[CMD_BUF_MAX_LEN] = {0};
    SYS_CMD_HEADER stHeader = {0};
    struct sockaddr_un stClientAddr = {0};
    struct sockaddr_un stServerAddr = {0};
    if(!strCmd)
    {
        LOG_ERROR("invaild param \n");
        return -1;
    }
    stHeader.uCmdLen = sizeof(SYS_CMD_HEADER) + strlen(strCmd);
    if(stHeader.uCmdLen >= CMD_BUF_MAX_LEN)
    {
        LOG_ERROR("cmd is too long!\n");
        return -1;
    }
    /* 填充本机IP及端口 */
    memset(&stClientAddr, 0, sizeof(stClientAddr));
    stClientAddr.sun_family =AF_LOCAL;
    snprintf(stClientAddr.sun_path, sizeof(stClientAddr.sun_path), "%s/callcmd_%05d", CLI_PATH, (int)pthread_self());
    if ((iCliFd = socket(AF_UNIX,SOCK_STREAM, 0)) < 0)
    {
        LOG_ERROR("socket create unix socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return -1;
    }

    unlink(stClientAddr.sun_path);
    iLen = offsetof(struct sockaddr_un, sun_path) + strlen(stClientAddr.sun_path);

    if(bind(iCliFd, (struct sockaddr *)&stClientAddr, iLen) < 0)
    {
        LOG_ERROR("bind client unix address %s failed - errno[%d:%s]\n", stClientAddr.sun_path, errno, strerror(errno));
        close(iCliFd);
        return -1;
    }
    if(chmod(stClientAddr.sun_path, S_IRWXU) < 0)
    {
        LOG_ERROR("chmod client unix address %s failed - errno[%d:%s]\n", stClientAddr.sun_path, errno, strerror(errno));
        close(iCliFd);
        return -1;
    }
    stServerAddr.sun_family =AF_LOCAL;
    snprintf(stServerAddr.sun_path, sizeof(stServerAddr.sun_path), "%s", SYS_CMD_SUN_PATH);
    iLen = offsetof(struct sockaddr_un, sun_path) + strlen(stServerAddr.sun_path);
    if(connect(iCliFd, (struct sockaddr *)&stServerAddr, iLen) < 0)
    {
        LOG_ERROR("unix connect to %s failed - errno[%d:%s]\n", SYS_CMD_SUN_PATH, errno, strerror(errno));
        close(iCliFd);
        return -1;
    }

    memcpy(strCmdBuf, &stHeader, sizeof(SYS_CMD_HEADER));
    memcpy(strCmdBuf + sizeof(SYS_CMD_HEADER), strCmd, strlen(strCmd));
   // LOG_ERROR("%d %d %x \n",stHeader.uCmdLen,strlen(strCmd),strCmd[0]);
    if(write(iCliFd, strCmdBuf, stHeader.uCmdLen) != stHeader.uCmdLen) 
    {
        LOG_ERROR("write cmd fail\n");
        close(iCliFd);
        return -1;
    }

    if(read(iCliFd, &iRet, 4) != 4)
    {
        LOG_ERROR("recv from execSystemCmd failed - errno[%d:%s]\n", errno, strerror(errno));
        close(iCliFd);
        return -1;
    }

    close(iCliFd);
    return iRet;
}
#else
/**@fn         SysPosix_cmd_call
 * @brief      调用系统命令,如果命令的前缀为$，则进程在后台执行
 * @param[in]  strCmd 系统命令字符串
 * @return     调用失败，返回-1；调用成功，若后台执行返回大于0的子进程号，前台执行则返回system接口返回值
 */
INT32 SysPosix_cmd_call(const CHAR *strCmd)
{

    INT32 iRet = ERROR;
    if(NULL == strCmd)
    {
        return iRet;
    }
    iRet = system(strCmd);
    if (ERROR == iRet) {
        LOG_ERROR("system Cmd(%s) failed - errno[%d:%s]\n", strCmd, errno, strerror(errno));
    }
    return iRet;
}
#endif

/**@fn         SysPosix_cmd_call_safety
 * @brief      安全方式调用系统命令
 * @param[in]  strCmd 系统命令字符串
 * @return     调用失败，返回-1；调用成功，返回0
 * @note       注意默认要执行的工具在/bin/路径下, 不在的要指定绝对路径
 */
INT32 SysPosix_cmd_call_safety(const CHAR *strCmd)
{
    pid_t uPid;
    INT32 iExitCode, iStatus;
    INT32 iArgCount = 0;
    CHAR  strExecPath[64] = {0};
    CHAR *aArgs[10] = {0};
    CHAR *pSavePtr = NULL;
    CHAR *pCmdCopy = NULL;
    CHAR *pToken = NULL;

    if (!strCmd || strlen(strCmd) == 0) {
        LOG_ERROR("Invalid command string\n");
        return ERROR;
    }
    
    /* 1. 使用strtok_r进行安全的命令解析 */
    pCmdCopy = strdup(strCmd);
    if (!pCmdCopy) {
        LOG_ERROR("strdup failed\n");
        return ERROR;
    }
    
    pToken = strtok_r(pCmdCopy, " ", &pSavePtr);
    while (pToken && iArgCount < 10 - 1) {
        aArgs[iArgCount++] = pToken;
        pToken = strtok_r(NULL, " ", &pSavePtr);
    }
    aArgs[iArgCount] = NULL;
    
    /* 确定可执行文件路径 */
    if (strchr(aArgs[0], '/')) {
        /* 绝对路径或相对路径 */
        if (strlen(aArgs[0]) >= 64) {
            LOG_ERROR("Exec path too long\n");
            free(pCmdCopy);
            return ERROR;
        }
        strncpy(strExecPath, aArgs[0], 64 - 1);
    } else {
        /* 在标准路径中查找 */
        if (snprintf(strExecPath, 64, "/bin/%s", aArgs[0]) >= 64) {
            LOG_ERROR("Exec path too long\n");
            free(pCmdCopy);
            return ERROR;
        }
    }

    /* 检查可执行文件是否存在 */
    if(access(strExecPath, F_OK)) {
        LOG_ERROR("%s is not exist\n", strExecPath);
        free(pCmdCopy);
        return ERROR;
    }

    /* 2. 执行命令*/
    iStatus = posix_spawn(&uPid, strExecPath, NULL, NULL, aArgs, NULL);
    free(pCmdCopy);

    if (iStatus != 0) {
        LOG_ERROR("posix_spawn failed - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
    }
    
    /* 3. 等待子进程 */
    if (waitpid(uPid, &iStatus, 0) == -1) {
        LOG_ERROR("waitpid - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
    }
    
    if (WIFEXITED(iStatus)) {
        iExitCode = WEXITSTATUS(iStatus);
        if (iExitCode != 0) {
            LOG_ERROR("cmd run failed - exit code(%d)\n", iExitCode);
            return ERROR;
        }
    } else if (WIFSIGNALED(iStatus)) {
        LOG_ERROR("Child process terminated by signal(%d)\n", WTERMSIG(iStatus));
        return ERROR;
    } else {
        LOG_ERROR("cmd terminated abnormally\n");
        return ERROR;
    }

    return 0;
}

/**@fn         SysPosix_move
 * @brief      移动文件 
 * @param[in]  strSrcPath  待移动文件全路径
 * @param[in]  strDstPath  目的路径,全路径
 * @return     成功返回 0   错误返回 其他, 参考ERROR_CODE_E, 支持通过get_last_errno获取错误码
 */
INT32 SysPosix_move(const CHAR *strSrcPath, const CHAR *strDstPath)
{
    INT32 iRet = ERROR;
    CHAR strSysCmd[256] = {0};
    if(NULL == strSrcPath || NULL == strDstPath)
    {
        return iRet;
    }
    snprintf(strSysCmd,sizeof(strSysCmd) -1 ,"mv %s %s",strSrcPath,strDstPath);
    iRet = SysPosix_cmd_call(strSysCmd);
    if (OK != iRet)
    {
        LOG_ERROR("move file error  :%s  \n",strSysCmd);
    }
    return iRet;
}



/**@fn         SysPosix_cp
 * @brief      拷贝文件 
 * @param[in]  strSrcPath  待移动文件全路径
 * @param[in]  strDstPath  目的路径,全路径
 * @return     成功返回 0   错误返回 其他, 参考ERROR_CODE_E, 支持通过get_last_errno获取错误码
 */
INT32 SysPosix_cp(const CHAR *strSrcPath, const CHAR *strDstPath)
{
    INT32 iRet = ERROR;
    CHAR strSysCmd[256] = {0};
    if(NULL == strSrcPath || NULL == strDstPath)
    {
        return iRet;
    }
    snprintf(strSysCmd,sizeof(strSysCmd) -1 ,"cp %s %s",strSrcPath,strDstPath);
    iRet = SysPosix_cmd_call(strSysCmd);
    if (OK != iRet)
    {
        LOG_ERROR("move file error  :%s  \n",strSysCmd);
    }
    return iRet;
}


/**@fn         SysPosix_mkdir
 * @brief      创建目录
 * @param[in]  strPath 路径
 * @return     成功返回 0 错误返回 其他, 参考ERROR_CODE_E, 支持通过get_last_errno获取错误码
 */
INT32 SysPosix_mkdir(const CHAR *strPath)
{
    INT32 iRet = ERROR;
    if(NULL == strPath)
    {
        return ERROR;
    }
    iRet = SysPosix_access(strPath);
    if(iRet == OK)
    {
        return OK;
    }
    iRet = mkdir(strPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (OK != iRet)
    {
        LOG_ERROR("strPathName:%s failed - errno[%d:%s]\n",strPath, errno, strerror(errno));
    }
    return iRet;
}

/**@fn         SysPosix_rmdir
 * @brief      删除目录
 * @param[in]  strPath 路径
 * @return     成功返回 0 错误返回 其他, 参考ERROR_CODE_E, 支持通过get_last_errno获取错误码
 */
INT32 SysPosix_rmdir(const CHAR *strPath)
{
    INT32 iRet = ERROR; 
    if(NULL == strPath)
    {
        return ERROR;
    }
    
    iRet = rmdir(strPath);
    if (OK != iRet)
    {
        LOG_ERROR("strPathName :%s failed - errno[%d:%s]\n",strPath, errno, strerror(errno));
    }
    return iRet;
}

/**@fn         SysPosix_access
 * @brief      判断文件是否存在
 * @param[in]  strPath 路径
 * @return     存在返回 0 失败返回 其他
 */    
INT32 SysPosix_access(const CHAR *strPath)
{
    INT32 iRet = ERROR;
    if(!strPath)
    {
        return iRet;
    }
    iRet = access(strPath, F_OK);
    return iRet; 
}

/**@fn         SysPosix_rm
 * @brief      删除
 * @param[in]  strPath 文件路径
 * @return     成功返回 0 错误返回 其他, 参考ERROR_ID_E
 */
INT32 SysPosix_rm(const CHAR *strPath)
{
    INT32 iRet = ERROR;
    if(NULL == strPath)
    {
        return ERROR;
    } 
    iRet = SysPosix_access(strPath);
    if(iRet != OK)
    {
        return OK;
    }
    iRet = unlink(strPath);
    if (iRet < 0)
    {
        LOG_ERROR("error:%s iRet:%d \n",strerror(errno),iRet);
        return iRet;
    }
    return OK;
}

/**@fn          SysPosix_sync      
 * @brief       强制写所有文件到存储介质
 * @return      成功 0  失败 -1 
 */
VOID SysPosix_sync(VOID)
{
    sync();
}

/**@fn         SysPosix_rename
 * @brief      重命名文件
 * @param[in]  strOldPath 旧文件路径
 * @param[in]  strNewPath 新文件路径
 * @return     成功返回 0  错误返回 其他, 参考ERROR_ID_E
 */
INT32 SysPosix_rename(const CHAR *strOldPath, const CHAR *strNewPath)
{
    INT32 iRet = ERROR;
    if(NULL == strOldPath || NULL == strNewPath)
    {
        return ERROR;
    }

    iRet = rename(strOldPath, strNewPath);
    if (OK != iRet)
    {
        LOG_ERROR("strOldPath :%s :%s failed - errno[%d:%s]\n",strOldPath, strNewPath, errno, strerror(errno));
    }
    
    return iRet;
}

/**@fn         SysPosix_fcntl
 * @brief      设置套接字属性，参考man fcntl
 * @param[in]  iFd      套接字句柄
 * @param[in]  iType    操作层面，比如是套接字层面(SOL_SOCKET)还是协议层面
 * @param[in]  iOptions 操作名称
 * @return     成功返回对应的值；失败参考ERROR_ID_E
 */
INT32 SysPosix_fcntl(INT32 iFd, UINT32 iType, UINT32 iOptions)
{
    return fcntl(iFd, ((iType == SYS_SOL_SOCKET) ? SOL_SOCKET : iType),  iOptions);
}

/**@fn         SysPosix_select
 * @brief      select封装。
 * @param[in]  iFds  , 最大iFd + 1
 * @param[in]  pStReadFdSet  , 可读集合
 * @param[in]  pStWriteFdSet  , 可写集合
 * @param[in]  pStExceptionFdSet  , 异常集合
 * @param[in]  pStTimeOut  , 超时时间
 * @return     > 0 有事件(不代表有事件的iFd个数)； = 失败返回ERROR
 * @return     = 0 无事件 @return  ERROR , 失败，通过get_last_errno获取错误码
 * @note 并非所有的系统都支持读、写、异常集合,
 *    比如安霸就只支持读集合
 */
INT32 SysPosix_select(INT32 iFds, SYS_FD_SET_T *pStReadFdSet, SYS_FD_SET_T *pStWriteFdSet,
                  SYS_FD_SET_T *pStExceptionFdSet, SYS_TIMEVAL_T *pStTimeOut)
{
    INT32 iRet = ERROR;
    iRet = select(iFds,(fd_set*)pStReadFdSet, (fd_set *)pStWriteFdSet,(fd_set *)pStExceptionFdSet,(struct timeval *)pStTimeOut);
    if (ERROR == iRet) {
        LOG_ERROR("select failed - errno[%d:%s]\n", errno, strerror(errno));
    }
    return iRet;
}


/**@fn         SysPosix_get_last_errno      
 * @brief      获取套接字错误码
 * @param[in]  无
 * @return     详见errno 
 */
INT32 SysPosix_get_last_errno()
{
    return errno;
}

/**
 * @brief      获取当前进程的进程/CPU名称
 * @param[out] strPname     进程/CPU名称
 * @param[in]  uNameLen     进程/CPU名称长度
 * @return     0表示成功, 其他值表示失败
 */
INT32 SysPosix_processname_get (CHAR *strPname, UINT32 uNameLen)
{
    INT32 iRet;
    static CHAR strProcessPath[HAL_PATH_MAX];

    if (!strPname || !uNameLen) {
        LOG_ERROR ("Invaild input params pname %p, len %d\n", 
            strPname, uNameLen);
        return -EINVAL;
    }

    if (uPnameSet) {
        strncpy(strPname, strPnameMapping, MIN(MAX_PROCESS_NAME_LEN, uNameLen));
        return 0;
    }

    iRet = readlink("/proc/self/exe", strProcessPath, HAL_PATH_MAX - 1);
    if (iRet < 0) {
        LOG_ERROR ("Failed to readlink \"/proc/self/exe\" : %s\n", 
            strerror (errno));
        return -errno;
    }

    strProcessPath[iRet] = '\0';
    
    strncpy(strPname, strrchr(strProcessPath, '/') + 1, MIN(MAX_PROCESS_NAME_LEN, uNameLen));

    return 0;
}

/**
 * @brief      设置当前进程的进程/CPU名称
 * @param[in]  strPname     进程/CPU名称
 * @return     0表示成功, 其他值表示失败
 */
INT32 SysPosix_processname_set (const CHAR *strPname)
{
    if (!strPname) {
        LOG_ERROR ("Invaild input params pname %p\n", strPname);
        return -EINVAL;
    }

    strncpy(strPnameMapping, strPname, MAX_PROCESS_NAME_LEN - 1);

    uPnameSet = TRUE;

    return 0;
}

