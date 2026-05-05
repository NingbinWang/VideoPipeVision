#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>

#include "Common.h"
#include "SysErrno.h"
#include "SysNetlink.h"
#include "SysNetlinkGenericInner.h"
#include "Logger.h"
#include "SysMemory.h"
typedef struct {
    struct nlmsghdr stNlh;
    struct genlmsghdr stGnlh;
    UINT8 aMsg[0];
} GENCMSG_HEADER_T;

#define NLA_DATA(nla)        ((void *)((char*)(nla) + NLA_HDRLEN))
#define NLA_NEXT(nla, len) ((len) -= NLA_ALIGN((nla)->nla_len), \
                        (struct nlattr*)((char*)(nla) + NLA_ALIGN((nla)->nla_len)))

static INT32 genlnetlink_msg_check(GENCMSG_HEADER_T *pMsg, INT32 iReplen)
{
    struct nlmsgerr *pErr = NULL;

    if (pMsg->stNlh.nlmsg_type == NLMSG_ERROR || !NLMSG_OK((&pMsg->stNlh), iReplen)) {
        pErr = (struct nlmsgerr *)NLMSG_DATA(pMsg);
        LOG_ERROR("fatal reply error %d nlmsg_type(%u) nlmsg_len(%u) replen(%d)\n",pErr->error, pMsg->stNlh.nlmsg_type, pMsg->stNlh.nlmsg_len, iReplen);
        return -1;
    }

    return 0;
}

/**
 * nla_ok - check if the netlink attribute fits into the remaining bytes
 * @nla: netlink attribute
 * @remaining: number of bytes remaining in attribute stream
 */
static INT32 nla_ok(const struct nlattr *pNla, INT32 iRemaining)
{
    return iRemaining >= (INT32) sizeof(*pNla) &&
            pNla->nla_len >= sizeof(*pNla) &&
            pNla->nla_len <= iRemaining;
}

INT32 SysGenlNetlink_create_nl_socket(UINT32 uPid)
{
    INT32 iSockfd, iRetval;
    struct sockaddr_nl stSrcaddr;

    /* Create a socket */
    iSockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if(iSockfd == -1){
        LOG_ERROR("error getting socket:errno[%d:%s]\n", errno, strerror(errno));
        return -1;
    }

    /* To prepare binding */
    memset(&stSrcaddr, 0, sizeof(stSrcaddr));
    stSrcaddr.nl_family = AF_NETLINK;       /**< 地址族固定为AF_NETLINK */
    stSrcaddr.nl_pid = uPid;                /**< 本机pid，可以根据getpid()获取，但是同一个pid不能重复bind绑定，因此根据情况用户自己传入 */
    /* 绑定情况下nl_pid为0时则说明让内核自动分配一个唯一的pid，但是通常情况下由用户自己指定 */

    /* Bind */
    iRetval = bind(iSockfd, (struct sockaddr *)&stSrcaddr, sizeof(stSrcaddr));
    if(iRetval < 0){
        LOG_ERROR("bind failed:errno[%d:%s]\n", errno, strerror(errno));
        close(iSockfd);
        return -1;
    }

    return iSockfd;
}

INT32 SysGenlNetlink_get_id(INT32 iSockfd, SYS_GENLNETLINK_ID_T *pGenlId)
{
    UINT8 aBuf[1024];
    INT32 iId = 0;
    INT32 iRepLen, iPayloadLen, iRemaining, iGrpAttrlen;
    struct sockaddr_nl stDestaddr;
    struct nlmsghdr *pNlh = NULL;
    struct genlmsghdr *pGenlh = NULL;
    struct nlattr *pAttr =NULL;
    struct nlattr *pNla = NULL;
    struct nlattr *pSubNla = NULL;
    struct nlattr *pGrpNla = NULL;
    struct nlattr *aAttrs[CTRL_ATTR_MAX + 1] = {};
    struct nlattr *aGrpAttrs[CTRL_ATTR_MCAST_GRP_MAX + 1] = {};
    GENCMSG_HEADER_T *pGenHeader = NULL;

    if (NULL == pGenlId) {
        LOG_ERROR("Invaild params pGenlId(%p)\n", pGenlId);
        return -EINVAL;
    } else {
        if (NULL == pGenlId->pFamily) {
            LOG_ERROR("Invaild params pFamily(%p)\n", pGenlId->pFamily);
            return -EINVAL;
        } 
    }

    /*******************netlink消息构成***********************/
    /* | nlmsghdr |               nlmsg payload            | */ /**< netlink消息 包含一个 nlmsg头 和 nlmsg数据 信息 */
    /*            | genlmsghdr |      genlmsg payload      | */ /**< nlmsg数据信息 包含 genlmsg头 和 genlmsg数据 信息 */
    /*                         | attr | attr | attr | attr | */ /**< genlmsg数据信息 中包含 一个或多个 attr信息 */
    /*                         | nlattr |   attr payload   | */ /**< attr信息 包含 attr消息属性 和 attr消息数据 */
    /*********************************************************/

    /* 1. 构造请求消息(netlink消息头-通用netlink消息头-netlink属性-属性数据) */
    memset(aBuf, 0, sizeof(aBuf));
    pGenHeader = (GENCMSG_HEADER_T *)aBuf;

    /* netlink消息头 */
    pGenHeader->stNlh.nlmsg_type = GENL_ID_CTRL;        /**< 消息类型(即家族id)为 家族id 控制 */
    pGenHeader->stNlh.nlmsg_flags = NLM_F_REQUEST;      /**< 表示用户空间发往内核空间的请求 */
    pGenHeader->stNlh.nlmsg_seq = 0;                    /**< 发送序列号 */
    pGenHeader->stNlh.nlmsg_pid = getpid();             /**< 发送方pid */

    /* 通用netlink消息头 */
    pGenHeader->stGnlh.cmd = CTRL_CMD_GETFAMILY;        /**< 控制命令，配合nlmsg_type(GENL_ID_CTRL)表示获取 家族id */

    /* netlink属性 */
    pAttr = (struct nlattr *)pGenHeader->aMsg;
    pAttr->nla_type = CTRL_ATTR_FAMILY_NAME;                    /**< 添加属性类型-家族名称 */
    pAttr->nla_len = NLA_HDRLEN + strlen(pGenlId->pFamily) + 1; /**< 属性数据长度 */

    /* 属性数据(payload) */
    strcpy((CHAR *)pAttr + NLA_HDRLEN, pGenlId->pFamily);       /**< 对应属性数据(家族名称 配合类型CTRL_ATTR_FAMILY_NAME使用) */

    /* 消息总长度计算填充 */
    pGenHeader->stNlh.nlmsg_len = NLMSG_HDRLEN + GENL_HDRLEN + pAttr->nla_len;

    /* 目标地址填充 */
    memset(&stDestaddr, 0, sizeof(stDestaddr));
    stDestaddr.nl_family = AF_NETLINK;
    stDestaddr.nl_pid = 0;              /**< pid为0表示目标为内核 */

    /* 2. 发送请求 */
    if (sendto(iSockfd, pGenHeader, pGenHeader->stNlh.nlmsg_len, 0, (struct sockaddr *)&stDestaddr, sizeof(stDestaddr)) < 0) {
        LOG_ERROR("sendto error - [%d:%s]\n", errno, strerror(errno));
        return -1;
    }

    /* 3. 清空数据后接收 */
    memset(aBuf, 0, sizeof(aBuf));
    iRepLen = recv(iSockfd, aBuf, sizeof(aBuf), 0);
    if (iRepLen < 0 || genlnetlink_msg_check((GENCMSG_HEADER_T *)aBuf, iRepLen)) {
        LOG_ERROR("recv error - [%d:%s]\n", errno, strerror(errno));
        return -1;
    }

    if (pGenHeader->stNlh.nlmsg_type != GENL_ID_CTRL) {
        LOG_ERROR("msg nlmsg_type(%d) is not familyid(%d)\n", pGenHeader->stNlh.nlmsg_type, GENL_ID_CTRL);
        return -1;
    }

    /* 4. 解析响应 */
    /* 如果有多个netlink消息则逐个解析这些消息，在当前情景下只处理家族id控制类型的消息 */
    for (pNlh = (struct nlmsghdr *)aBuf; NLMSG_OK(pNlh, iRepLen); pNlh = NLMSG_NEXT(pNlh, iRepLen)) {
        pGenlh = (struct genlmsghdr *)NLMSG_DATA(pNlh);
        if (pNlh->nlmsg_type != GENL_ID_CTRL) {
            continue;
        }

        /* 解析属性 */
        pAttr = (struct nlattr *)(pGenlh + 1);
        iPayloadLen = NLMSG_PAYLOAD(pNlh, GENL_HDRLEN);

        /* 逐个解析attr消息 */
        for ( ; nla_ok(pAttr, iPayloadLen); pAttr = NLA_NEXT(pAttr, iPayloadLen)) {
            /* 解析 家族id， 属性非家族id则存储其属性指针到数组中 */
            if (pAttr->nla_type == CTRL_ATTR_FAMILY_ID) {
                iId = *(UINT16 *) ((CHAR *)pAttr + NLA_HDRLEN);
                pGenlId->uFamilyId = iId;
            }

            if (pAttr->nla_type <= CTRL_ATTR_MAX) {
                aAttrs[pAttr->nla_type] = pAttr;
            }
        }

        /*******************多组播数据构成*********************/
        /* |nlattr |              attr payload              | */ /**< 一条attr消息由 attr消息属性 和 attr消息数据(CTRL_ATTR_MCAST_GROUPS) */
        /*         | sub nlattr |        attr payload       | */ /**< 多组播数据中的attr消息数据由 多个子attr消息组成(一个CTRL_ATTR_MCAST_GROUPS中包含多个子属性数据) */
        /*                      | grp nlattr | attr payload | */ /**< 子attr消息由 多个组attr消息组成(一个子消息由多个类型的attr消息组成, 根据子消息中的CTRL_ATTR_MCAST_GRP_NAME类型消息匹配得到CTRL_ATTR_MCAST_GRP_ID类型的组id) */
        /******************************************************/
        /* 检查多播组属性，当多组播属性消息指针不为空时对其进行解析 */
        if (aAttrs[CTRL_ATTR_MCAST_GROUPS]) {
            pNla = aAttrs[CTRL_ATTR_MCAST_GROUPS];
            iRemaining = pNla->nla_len - NLA_HDRLEN;
            pSubNla = (struct nlattr*)((CHAR *)pNla + NLA_HDRLEN);

            /* 逐个解析多组播属性消息中的子属性消息 */
            for ( ; nla_ok(pSubNla, iRemaining); pSubNla = NLA_NEXT(pSubNla, iRemaining)) {
                pGrpNla = (struct nlattr*)((char*)pSubNla + NLA_HDRLEN);
                iGrpAttrlen = pSubNla->nla_len - NLA_HDRLEN;

                /* 逐个解析子属性消息中的组消息并将其指针放入数组，解析组名称类型消息与传入组消息相同则将其子属性消息中对应的组id提取 */
                for ( ; nla_ok(pGrpNla, iGrpAttrlen); pGrpNla = NLA_NEXT(pGrpNla, iGrpAttrlen)) {
                    if (pGrpNla->nla_type <= CTRL_ATTR_MCAST_GRP_MAX) {
                        aGrpAttrs[pGrpNla->nla_type] = pGrpNla;
                    }
                }

                /* 当传入组名不为空时 匹配该组消息中的组名称类型消息数据和传入的组名称 */
                if (pGenlId->pGroup && aGrpAttrs[CTRL_ATTR_MCAST_GRP_NAME] && 
                    strcmp((char*)NLA_DATA(aGrpAttrs[CTRL_ATTR_MCAST_GRP_NAME]), pGenlId->pGroup) == 0) {
                    /* 匹配成功则将该组的组id消息解析放入结构体中 */
                    if (aGrpAttrs[CTRL_ATTR_MCAST_GRP_ID]) {
                        iId =  *(UINT32 *)NLA_DATA(aGrpAttrs[CTRL_ATTR_MCAST_GRP_ID]);
                        pGenlId->uGroupId = iId;
                        break;
                    }
                }
            }
        }
    }

    return 0;
}

INT32 SysGenlNetlink_send_msg(INT32 iSockfd, VOID *pBuffer, UINT32 uBufferLen)
{
    INT32 iRet, iMsgLen, iLoopCnt;
    UINT32 uBufferLenTmp = uBufferLen;
    struct nlattr *pNla;
    GENCMSG_HEADER_T *pMsg;
    struct sockaddr_nl stDstaddr;
    SYS_GENLNETLINK_ATTR_T *pAttrConf;
    SYS_GENLNETLINK_MSG_CONFIG_T *pConfig = (SYS_GENLNETLINK_MSG_CONFIG_T *)pBuffer;

    if (NULL == pConfig || iSockfd <= 0 || uBufferLen < sizeof(SYS_GENLNETLINK_MSG_CONFIG_T)) {
        LOG_ERROR("invalid params sockfd(%d) pConfig(%p) uBufferLen(%u)\n", iSockfd, pConfig, uBufferLen);
        return -EINVAL;
    }

    if (uBufferLen < sizeof(SYS_GENLNETLINK_MSG_CONFIG_T) + pConfig->uAttrNr * sizeof(SYS_GENLNETLINK_ATTR_T)) {
        LOG_ERROR("invalid params buffer len is too small(%u) - attrnum(%u) need(%lu)\n",
            uBufferLen, pConfig->uAttrNr, sizeof(SYS_GENLNETLINK_MSG_CONFIG_T) + pConfig->uAttrNr * sizeof(SYS_GENLNETLINK_ATTR_T));
        return -EINVAL;
    }

    if (pConfig->iFamilyId <= 0 || !pConfig->uAttrNr) {
        LOG_ERROR("invalid familyid(%d) attr num(%u)\n", pConfig->iFamilyId, pConfig->uAttrNr);
        return -EINVAL;
    }

    /* 消息长度计算(nlmsg头+genlmsg头+nlattr头+发送数据长度) */
    iMsgLen = NLMSG_HDRLEN + GENL_HDRLEN;  
    for (iLoopCnt = 0, pAttrConf = (SYS_GENLNETLINK_ATTR_T *)pConfig->aAttrMsg;
            iLoopCnt < pConfig->uAttrNr && uBufferLenTmp > sizeof(SYS_GENLNETLINK_ATTR_T);
            iLoopCnt++) {
        if (0 == pAttrConf->uDataLen || uBufferLenTmp < (sizeof(SYS_GENLNETLINK_ATTR_T) + pAttrConf->uDataLen)) {
            LOG_WARNING("Invalid uDataLen(%u) uBufferLenTmp(%u)\n", pAttrConf->uDataLen, uBufferLenTmp);
            break;
        }
        iMsgLen += NLA_HDRLEN + NLA_ALIGN(pAttrConf->uDataLen);
        pAttrConf = (SYS_GENLNETLINK_ATTR_T *)((UINTPTR)pAttrConf + sizeof(SYS_GENLNETLINK_ATTR_T) + NLA_ALIGN(pAttrConf->uDataLen));
        uBufferLenTmp -= sizeof(SYS_GENLNETLINK_ATTR_T) + NLA_ALIGN(pAttrConf->uDataLen);
    }

    pMsg = (GENCMSG_HEADER_T *)SysMemory_malloc(iMsgLen);
    if (!pMsg) {
        LOG_ERROR("malloc (%d)bytes failed errno[%d:%s]\n", iMsgLen, errno, strerror(errno));
        return -1;
    }
    memset(pMsg, 0, iMsgLen);

    /* 1. 构造请求消息(netlink消息头-通用netlink消息头-netlink属性-属性数据) */
    /* netlink消息头 */
    pMsg->stNlh.nlmsg_len = iMsgLen;                /**< netlink消息长度 */
    pMsg->stNlh.nlmsg_type = pConfig->iFamilyId;    /**< 消息类型(即家族id) */
    pMsg->stNlh.nlmsg_flags = NLM_F_REQUEST;        /**< 表示用户空间发往内核空间的请求 */
    pMsg->stNlh.nlmsg_seq = 0;                      /**< 发送序列号 */
    pMsg->stNlh.nlmsg_pid = getpid();               /**< 发送方pid */

    /* 通用netlink消息头 */
    pMsg->stGnlh.cmd = pConfig->uGenlCmd;           /**< 家族控制命令 */
    pMsg->stGnlh.version = pConfig->uGenlVer;       /**< 家族版本 */

    /* 属性消息头 */
    for (iLoopCnt = 0, pAttrConf = (SYS_GENLNETLINK_ATTR_T *)pConfig->aAttrMsg, pNla = (struct nlattr *)pMsg->aMsg;
            iLoopCnt < pConfig->uAttrNr && uBufferLenTmp > sizeof(SYS_GENLNETLINK_ATTR_T);
            iLoopCnt++) {
        if (0 == pAttrConf->uDataLen || uBufferLenTmp < (sizeof(SYS_GENLNETLINK_ATTR_T) + pAttrConf->uDataLen)) {
            LOG_WARNING("Invalid uDataLen(%u) uBufferLenTmp(%u)\n", pAttrConf->uDataLen, uBufferLenTmp);
            break;
        }

        pNla->nla_type = pAttrConf->uNlaType;                           /**< 属性数据类型 */
        pNla->nla_len = NLA_HDRLEN + NLA_ALIGN(pAttrConf->uDataLen);    /**< 属性数据长度 */

        memcpy((CHAR *)pNla + NLA_HDRLEN, pAttrConf->aData, pAttrConf->uDataLen);

        pNla = (struct nlattr *)((CHAR *)pNla + NLA_ALIGN(pNla->nla_len));    /**< 循环更新属性消息头地址 */
        pAttrConf = (SYS_GENLNETLINK_ATTR_T *)((UINTPTR)pAttrConf + sizeof(SYS_GENLNETLINK_ATTR_T) + NLA_ALIGN(pAttrConf->uDataLen));
        uBufferLenTmp -= sizeof(SYS_GENLNETLINK_ATTR_T) + NLA_ALIGN(pAttrConf->uDataLen);
    }

    /* 2. 目标地址填充 */
    memset(&stDstaddr, 0, sizeof(stDstaddr));
    stDstaddr.nl_family = AF_NETLINK;
    stDstaddr.nl_pid = 0;                           /**< pid为0表示目标为内核 */
    stDstaddr.nl_groups = 0;                        /**< 单播 */

    /* 3. 数据发送 */
    while ((iRet = sendto(iSockfd, (CHAR *)pMsg, iMsgLen, 0, (struct sockaddr *) &stDstaddr, sizeof(stDstaddr))) < iMsgLen) {
        if (iRet > 0) {
            pMsg += iRet;
            iMsgLen -= iRet;
        } else if (errno != EAGAIN) {
            LOG_ERROR("genlmsg send failed - errno[%d:%s]\n", errno, strerror(errno));
            free(pMsg);
            return -1;
        }
    }

    free(pMsg);

    return 0;
}

INT32 SysGenlNetlink_recv_msg(INT32 iSockfd, VOID *pBuffer, UINT32 uBufferLen)
{
    struct nlattr *pNla =NULL;
    INT32 iMsgLen, iReplen = 0, iPayloadlen;
    UINT32 uBufferLenTmp = uBufferLen;
    GENCMSG_HEADER_T *pMsg = NULL;
    SYS_GENLNETLINK_ATTR_T *pAttrMsg = NULL;
    SYS_GENLNETLINK_MSG_CONFIG_T *pConfig = (SYS_GENLNETLINK_MSG_CONFIG_T *)pBuffer;

    if (iSockfd <= 0 || NULL == pBuffer || uBufferLen < sizeof(SYS_GENLNETLINK_MSG_CONFIG_T)) {
        LOG_ERROR("invalid params iSockfd(%d) pBuffer(%p) uBufferLen(%u)\n", iSockfd, pBuffer, uBufferLen);
        return -EINVAL;
    }

    /* 数据接收buffer内存空间申请 */
    iMsgLen = uBufferLen + NLMSG_HDRLEN + GENL_HDRLEN;
    pMsg = (GENCMSG_HEADER_T *)SysMemory_malloc(iMsgLen);
    if (!pMsg) {
        LOG_ERROR("malloc (%d)bytes failed errno[%d:%s]\n", iMsgLen, errno, strerror(errno));
        return -1;
    }
    memset(pMsg, 0, iMsgLen);

    /* 数据接收 */
    iReplen = recv(iSockfd, pMsg, iMsgLen, 0);
    if(iReplen < 0 || genlnetlink_msg_check(pMsg, iReplen) < 0) {
        LOG_ERROR("recv failed errno - [%d:%s]\n", errno, strerror(errno));
        free(pMsg);
        return -1;
    }

    pConfig->iFamilyId = pMsg->stNlh.nlmsg_type;
    pConfig->uGenlCmd = pMsg->stGnlh.cmd;
    pConfig->uGenlVer = pMsg->stGnlh.version;
    uBufferLenTmp -= sizeof(SYS_GENLNETLINK_MSG_CONFIG_T);

    /* 指向第一个attr数据地址 并 计算出当前需要处理(未处理)的数据量 */
    pNla = (struct nlattr *)pMsg->aMsg;
    iPayloadlen = NLMSG_PAYLOAD(&pMsg->stNlh, sizeof(struct genlmsghdr));

    /* 逐个解析属性消息, 最多处理10条 */
    for (pAttrMsg = (SYS_GENLNETLINK_ATTR_T *)pConfig->aAttrMsg;
            nla_ok(pNla, iPayloadlen);
            pNla = NLA_NEXT(pNla, iPayloadlen), pConfig->uAttrNr++) {
        if (uBufferLenTmp < sizeof(SYS_GENLNETLINK_ATTR_T) + NLA_ALIGN(pNla->nla_len - NLA_HDRLEN)) {
            LOG_WARNING("Buffer len is too small uBufferLenTmp(%u) attrlen(%u)\n", uBufferLenTmp, pNla->nla_len);
            break;
        }

        /* attr数据解析填充 */
        pAttrMsg->uNlaType = pNla->nla_type;
        pAttrMsg->uDataLen = NLA_ALIGN(pNla->nla_len - NLA_HDRLEN);
        memcpy(pAttrMsg->aData, (CHAR *)NLA_DATA(pNla), pAttrMsg->uDataLen);
        pAttrMsg = (SYS_GENLNETLINK_ATTR_T *)((UINTPTR)pAttrMsg + sizeof(SYS_GENLNETLINK_ATTR_T) + pAttrMsg->uDataLen);
        uBufferLenTmp -= sizeof(SYS_GENLNETLINK_ATTR_T) + pAttrMsg->uDataLen;
    }

    free(pMsg);

    return 0;
}
