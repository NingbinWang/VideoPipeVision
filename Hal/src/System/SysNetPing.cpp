#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <errno.h>

#include "SysNetPing.h"
#include "Logger.h"

typedef struct {
    struct iphdr stIpHdr;        /*<< ip报文头部 */
    struct icmphdr stIcmpHdr;    /*<< icmp报文头部 */
    UINT8 aData[ICMP_DATA_SIZE]; /*<< icmp报文数据段 */
} ICMP_PING_MSG_T;

/*
 * icmp报文16位校验和计算方法:
 * 每2字节(16位)相加得到相加和，如果字节数奇数则将最后一个字节做高位补0合成一个16为数据(该组件中总数据量为偶数所以没有增加该逻辑)
 * 相加和如果超过16位则将高16位和底16位相加直到得到一个16位的和
 * 最后将得到的16位数据取反即为 icmp报文校验和
 */
static UINT16 icmp_checksum(UINT16 *pData, INT32 iLen)
{
    UINT32 uSum = 0;

    while (iLen > 1) {
        uSum += *pData++;
        iLen -= 2;
    }
    uSum = (uSum >> 16) + (uSum & 0xffff);
    uSum += (uSum >> 16);
    return (UINT16)~uSum;
}

static INT32 icmp_msg_recv(INT32 iSockFd, VOID *pBuffer, UINT32 uLen, UINT32 uWaitTime)
{
    INT32 iRet = 0;

    fd_set stReadSets;
    fd_set stErrSets;
    struct timeval stWaitTime;
    struct timeval *pWaitTime = NULL;

    FD_ZERO(&stReadSets);
    FD_ZERO(&stErrSets);
    FD_SET(iSockFd, &stReadSets);
    FD_SET(iSockFd, &stErrSets);

    if(WAIT_FOREVER == uWaitTime)
    {
        pWaitTime = NULL;
    } else {
        if (0 == uWaitTime) {
            LOG_WARNING ("Timeout cannot be 0, use default(2000ms)\n");
            uWaitTime = 2000;
        }
        memset(&stWaitTime, 0, sizeof(stWaitTime));
        stWaitTime.tv_sec = uWaitTime / 1000;
        stWaitTime.tv_usec  = (uWaitTime % 1000) * 1000;
        pWaitTime = &stWaitTime;
    }

    iRet = select(iSockFd + 1, &stReadSets, NULL, &stErrSets, pWaitTime);
    if (iRet > 0 && FD_ISSET(iSockFd, &stReadSets))
    {
        iRet = recv(iSockFd, pBuffer, uLen, 0);
        if (iRet < 0) {
            LOG_ERROR ("recv failed, ret(%d) errno[%d]: %s\n", iRet, errno, strerror(errno));
            return -errno;
        }
    } else {
        LOG_ERROR ("select failed, ret(%d) errno[%d]: %s\n", iRet, errno, strerror(errno));
        return -errno;
    }
    return 0;
}
/* @fn              SysNet_ping
 * @brief           ping对端地址是否连通
 * @param[in]       pDstIpAddr 对端ip地址
 * @param[in/out]   pTimeout   超时时间(unit: ms)/成功返回数据往返响应时间(unit: us)
 * @param[out]      pTtl       协议包生存时间
 * @return 成功返回0;错误，返回 -1
 */
INT32 SysNet_ping(CHAR *pDstIpAddr, UINT32 *pTimeout, UINT32 *pTtl)
{
    INT32 iLen =1, iSockFd, iIcmpId;
    struct sockaddr_in stDestAddr;
    struct timeval stRecvtime;
    struct timeval *pSendtime;
    ICMP_PING_MSG_T stIcmpPingMsg;
    CHAR strDestAddr[32] = {0};

    iSockFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (iSockFd < 0) {
        LOG_ERROR ("Failed to create socket errno[%d]: %s\n", errno, strerror(errno));
        return errno;
    }

    /**
     * IPPROTO_IP表示设置协议层为IP层, IP_HDRINCL表示数据包是否包含ip头，len=1表示包含ip头: 
     * 没有该操作会导致ping包解析异常，通过wareshark抓包可看到比正常ping报文多出20字节的ip头(ip头没有被解析去除导致被当做icmp头处理)
     */
    if (setsockopt(iSockFd, IPPROTO_IP, IP_HDRINCL,&iLen,sizeof(iLen))) {
        LOG_ERROR ("setsockopt failed,errno[%d]: %s\n", errno, strerror(errno));
        close(iSockFd);
        return -errno;
    }

    /* 目标地址信息填充 */
    memset(&stDestAddr, 0, sizeof(stDestAddr));
    stDestAddr.sin_family = AF_INET;
    stDestAddr.sin_port = 0;
    inet_aton(pDstIpAddr, &stDestAddr.sin_addr);

    /* 清除buffer(stIcmpPingMsg)中的数据 */
    memset(&stIcmpPingMsg, 0, sizeof(stIcmpPingMsg));
    iIcmpId = getpid();

    /* 设置IP头信息 */
    stIcmpPingMsg.stIpHdr.ihl = 5;                              /*<< 长度/头部校验和字段: 20/4=5*/
    stIcmpPingMsg.stIpHdr.version = 4;                          /*<< IPv4 */
    stIcmpPingMsg.stIpHdr.tos = 0;                              /*<< 类型服务 */
    stIcmpPingMsg.stIpHdr.tot_len = sizeof(stIcmpPingMsg);      /*<< 总长度 */
    stIcmpPingMsg.stIpHdr.id = 0;                               /*<< 初始化报文标识 */
    stIcmpPingMsg.stIpHdr.frag_off = 0;                         /*<< 设置flag标记为0 */
    stIcmpPingMsg.stIpHdr.ttl = 255;                            /*<< 生存时间 */
    stIcmpPingMsg.stIpHdr.protocol = IPPROTO_ICMP;              /*<< 协议号 (ICMP) */
    stIcmpPingMsg.stIpHdr.daddr = stDestAddr.sin_addr.s_addr;   /*<< 目标IP地址 */

    /* 设置ICMP头信息 */
    stIcmpPingMsg.stIcmpHdr.type = ICMP_ECHO;                    /*<< 报文类型 (回显请求) */
    stIcmpPingMsg.stIcmpHdr.code = 0;                            /*<< 错误码 */
    stIcmpPingMsg.stIcmpHdr.checksum = 0;                        /*<< 校验和初始化为0, 将由用户计算后填充 */
    stIcmpPingMsg.stIcmpHdr.un.echo.id = iIcmpId;                /*<< 请求ID */
    stIcmpPingMsg.stIcmpHdr.un.echo.sequence = 0;                /*<< 序列号 */

    LOG_DEBUG("iphdr(%ld) icmphdr(%ld) datalen(%ld) totlen(%d)\n", 
            sizeof(stIcmpPingMsg.stIpHdr), sizeof(stIcmpPingMsg.stIcmpHdr), sizeof(stIcmpPingMsg.aData), stIcmpPingMsg.stIpHdr.tot_len);

    /*<< 获取报文发送时间 */
    gettimeofday((struct timeval *)&stIcmpPingMsg.aData, NULL);

    /* 计算并填充校验和: 校验和计算需要先将buffer中checksum置零并填充好icmp报文数据后进行计算 */
    stIcmpPingMsg.stIcmpHdr.checksum = icmp_checksum((uint16_t*)&stIcmpPingMsg.stIcmpHdr, sizeof(stIcmpPingMsg.stIcmpHdr) + ICMP_DATA_SIZE);

    /* 发送数据 */
    if (sendto(iSockFd, &stIcmpPingMsg, sizeof(stIcmpPingMsg), 0, (struct sockaddr*)&stDestAddr, sizeof(stDestAddr)) < 0) {
        LOG_ERROR ("sendto icmp msg failed,errno[%d]: %s\n", errno, strerror(errno));
        close(iSockFd);
        return -errno;
    }

    /* 清除buffer(stIcmpPingMsg)中的数据 */
    memset(&stIcmpPingMsg, 0, sizeof(stIcmpPingMsg));
    memset(&stDestAddr, 0, sizeof(stDestAddr));
    iLen = sizeof(stDestAddr);

    /* 接收数据 */
    if (icmp_msg_recv(iSockFd, &stIcmpPingMsg, sizeof(stIcmpPingMsg), *pTimeout) < 0) {
        LOG_ERROR ("icmp_msg_recv failed\n");
        close(iSockFd);
        return -errno;
    }

    /* 获取报文接收时的时间 */
    gettimeofday(&stRecvtime, NULL);

    /* 检查接收到的报文 */
    sprintf(strDestAddr, "%d.%d.%d.%d", ntohl(stIcmpPingMsg.stIpHdr.saddr) >> 24, 
            (ntohl(stIcmpPingMsg.stIpHdr.saddr) >> 16) & 0xFF, 
            (ntohl(stIcmpPingMsg.stIpHdr.saddr) >> 8) & 0xFF, 
            ntohl(stIcmpPingMsg.stIpHdr.saddr) & 0xFF);
    if (strcmp(strDestAddr, pDstIpAddr)) {
        LOG_ERROR ("dest ip addr(%s) failed\n", strDestAddr);
        close(iSockFd);
        return -1;
    }

    if(icmp_checksum((uint16_t *)&stIcmpPingMsg.stIcmpHdr, ntohs(stIcmpPingMsg.stIpHdr.tot_len) - (stIcmpPingMsg.stIpHdr.ihl << 2))) {
        LOG_ERROR ("icmp checksum failed\n");
        close(iSockFd);
        return -1;
    }

    if(stIcmpPingMsg.stIcmpHdr.un.echo.id != iIcmpId) {
        LOG_ERROR ("icmp id(%d) failed\n", stIcmpPingMsg.stIcmpHdr.un.echo.id);
        close(iSockFd);
        return -1;
    }

    if(stIcmpPingMsg.stIcmpHdr.un.echo.sequence != 0) {
        LOG_ERROR ("icmp seq_id(%d) failed\n", stIcmpPingMsg.stIcmpHdr.un.echo.sequence);
        close(iSockFd);
        return -1;
    }

    if(stIcmpPingMsg.stIcmpHdr.type != ICMP_ECHOREPLY) {
        LOG_ERROR ("icmp type(%d) failed\n", stIcmpPingMsg.stIcmpHdr.type);
        close(iSockFd);
        return -1;
    }

    if(stIcmpPingMsg.stIcmpHdr.code != 0) {
        LOG_ERROR ("icmp code(%d) failed\n", stIcmpPingMsg.stIcmpHdr.code);
        close(iSockFd);
        return -1;
    }

    pSendtime = (struct timeval *)&stIcmpPingMsg.aData; /*发送时间*/
    *pTimeout = ((&stRecvtime)->tv_sec-pSendtime->tv_sec)*1000000 + ((&stRecvtime)->tv_usec-pSendtime->tv_usec);
    *pTtl = stIcmpPingMsg.stIpHdr.ttl;
    LOG_DEBUG ("%d bytes from %s:icmp_seq=%u ttl=%d rtt=%.3f ms\n",
            ntohs(stIcmpPingMsg.stIpHdr.tot_len) - (stIcmpPingMsg.stIpHdr.ihl << 2),
            strDestAddr,
            stIcmpPingMsg.stIcmpHdr.un.echo.sequence,
            stIcmpPingMsg.stIpHdr.ttl,
            (*pTimeout) / 1000.0);

    close(iSockFd);

    return 0;
}
