
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/select.h>
#include <linux/if.h>   
#include <linux/if_ether.h>   
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/route.h>
#include "SysMemory.h"
#include "SysNet.h"
#include "SysUtils.h"
#include "Logger.h"
#include "SysNetPing.h"

#define GET_IPV6ADDR 29
#define SYS_NET_IFINET6_PATH	"/proc/net/if_inet6"

/* @fn	      SysNet_set_ip	  
 * @brief	  配置IP
 * @param[in] strDevName 设备网卡名
 * @param[in] iDomain    协议族
 * @param[in] strIP      IP地址
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_set_ip(const CHAR *strDevName, INT32 iDomain, const CHAR *strIP)
{   
    INT32 iFd = -1;
    struct ifreq stIfreq;
	struct sockaddr_in stIPaddr;
    if(NULL == strDevName || NULL == strIP)
    {
        return ERROR;
    }
    if((iFd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_ERROR("create socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
    }

    snprintf(stIfreq.ifr_name, sizeof(stIfreq.ifr_name), "%s", strDevName);

    memset(&stIPaddr, 0, sizeof(struct sockaddr_in));
    stIPaddr.sin_family = PF_INET;
	inet_aton(strIP, &stIPaddr.sin_addr);

    memcpy(&stIfreq.ifr_ifru.ifru_addr, &stIPaddr, sizeof(struct sockaddr_in));
    if(ioctl(iFd, SIOCSIFADDR, &stIfreq) < 0)
    {
		LOG_ERROR("IP_set ioctl failed. error: %s!\n", strerror(errno));
		close(iFd);
		return ERROR;
	}

    close(iFd);

	LOG_INFO("== IP set OK!!! if: %s, ip: %s ==\n", strDevName, strIP);
    return OK;
}

/**@fn         SysNet_get_ip
 * @brief      获取设备IP地址
 * @param[in]  strEthName  网卡名
 * @param[in]  iDomain     协议族
 * @param[in]  strIpAddr   缓冲区地址指针
 * @param[in]  uLength     缓冲区大小长度
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_get_ip(const CHAR* strEthName, INT32 iDomain, CHAR *strIpAddr, UINT16 uLength)
{
    INT32 iFd = -1;
    struct ifreq stIfreq;
	struct sockaddr_in *pstIPaddr;
    FILE *pFileId;
    CHAR strDevName[20] = {0};
    CHAR strIpV6[8][5];
    INT32 iIndex = 0;
    INT32 iPrefix = 0;
    INT32 iScope = 0;
    INT32 iFlags = 0;
    
    if (NULL == strEthName || NULL == strIpAddr)
    {
        return ERROR;
    }
    
	if(PF_INET6 == iDomain)
	{
		pFileId = fopen(SYS_NET_IFINET6_PATH, "r");
		if (NULL == pFileId)
        {
            LOG_ERROR("fopen(%s) failed - errno[%d:%s]\n", SYS_NET_IFINET6_PATH, errno, strerror(errno));
            return ERROR;
        }     
		while (fscanf(pFileId, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
			strIpV6[0], strIpV6[1], strIpV6[2], strIpV6[3],
			strIpV6[4], strIpV6[5], strIpV6[6], strIpV6[7],
                &iIndex, &iPrefix, &iScope, &iFlags, strDevName) != EOF) {
			if (!strcmp(strDevName, strEthName))
            {
				sprintf(strIpAddr, "%s:%s:%s:%s:%s:%s:%s:%s",
					strIpV6[0], strIpV6[1], strIpV6[2], strIpV6[3],
					strIpV6[4], strIpV6[5], strIpV6[6], strIpV6[7]);
				fclose(pFileId);
                return OK;
			}
		}	
		fclose(pFileId);
		return ERROR;
	}

    if((iFd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_ERROR("create socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
    }
    snprintf(stIfreq.ifr_name, sizeof(stIfreq.ifr_name), "%s", strEthName);
    if(ioctl(iFd, SIOCGIFADDR, &stIfreq) < 0)
    {
        LOG_ERROR("get_IP:%s : ioctl failed! error: %s\n",strEthName,strerror(errno));
        close(iFd);
        return ERROR;
	}
    pstIPaddr = (struct sockaddr_in *)&stIfreq.ifr_addr;
    snprintf(strIpAddr, uLength, "%s", inet_ntoa(pstIPaddr->sin_addr));
    close(iFd); 
    return OK;
}
/**@fn         SysNet_set_mac_addr
 * @brief      设置设备mac地址
 * @param[in]  strEthName  网卡名
 * @param[in]  pStMacAddr  缓冲区地址指针
 * @param[in]  uLength     缓冲区大小长度
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_set_mac_addr(const CHAR* strEthName,UINT8 *pStMacAddr,UINT16 uLength)
{

    INT32 iRet = -1;
    INT32 iFd = -1;
    struct ifreq stIfreq;
    if (NULL == pStMacAddr || NULL == strEthName ) 
    {
        return iRet;
    }
    snprintf(stIfreq.ifr_name,sizeof(stIfreq.ifr_name),"%s",strEthName);
    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        LOG_ERROR("socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return iRet;
    }

    if (ioctl(iFd, SIOCGIFHWADDR, &stIfreq) == -1) {
        LOG_ERROR("ioctl get hwaddr - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return iRet;
    }

    memcpy(stIfreq.ifr_hwaddr.sa_data,pStMacAddr,uLength);
    iRet = ioctl(iFd, SIOCSIFHWADDR, &stIfreq, sizeof(stIfreq));
    if(iRet < 0)
    {
        LOG_ERROR("ioctl set hwaddr faield - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return iRet;
    }
    
    close(iFd);
    return iRet;
}
/**@fn         SysNet_get_mac_addr
 * @brief      获取设备mac地址
 * @param[in]  strEthName  网卡名
 * @param[in]  pStMacAddr  缓冲区地址指针
 * @param[in]  uLength     缓冲区大小长度
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_get_mac_addr(const CHAR* strEthName,UINT8 *pStMacAddr,UINT16 uLength)
{
    INT32 iRet = -1;
    INT32 iFd = -1;
    struct ifreq stIfreq;
    if (NULL == pStMacAddr || NULL == strEthName ) 
    {
        return iRet;
    }
    snprintf(stIfreq.ifr_name,sizeof(stIfreq.ifr_name),"%s",strEthName);
    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        LOG_ERROR("socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return iRet;
    }
    iRet = ioctl(iFd, SIOCGIFHWADDR, &stIfreq, sizeof(stIfreq));
    if(iRet < 0)
    {
        LOG_ERROR("ioctl get hwaddr failed - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return iRet;
    }
    memcpy(pStMacAddr, stIfreq.ifr_hwaddr.sa_data, std::min(uLength, (UINT16)sizeof(stIfreq.ifr_hwaddr.sa_data)));
    close(iFd);
    return iRet;
}
/**@fn         SysNet_set_netmask
 * @brief      设置掩码
 * @param[in]  strEthName      网卡名
 * @param[in]  pStAddr         缓冲区地址指针
 * @param[in]  uLength         缓冲区大小长度
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_set_netmask(const CHAR* strEthName,UINT8 *pStAddr,UINT16 uLength)
{
    INT32 iRet = -1;
    INT32 iFd = -1;
    struct ifreq stIfreq;
    struct sockaddr_in *pAddr;

    if (NULL == pStAddr || NULL == strEthName ) 
    {
        return iRet;
    }

    snprintf(stIfreq.ifr_name,sizeof(stIfreq.ifr_name),"%s",strEthName);
    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        LOG_ERROR("socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return iRet;
    }

    if (ioctl(iFd, SIOCGIFNETMASK, &stIfreq) == -1) {
        LOG_ERROR("ioctl get netmask - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return iRet;
    }

    pAddr = (struct sockaddr_in *)&stIfreq.ifr_netmask;
    pAddr->sin_family = AF_INET;
    inet_pton(AF_INET, (const CHAR *)pStAddr, &pAddr->sin_addr);

    iRet = ioctl(iFd, SIOCSIFNETMASK, &stIfreq, sizeof(stIfreq));
    if(iRet < 0)
    {
        LOG_ERROR("ioctl set netmask failed - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return iRet;
    }
    close(iFd);

    return iRet;
}
/**@fn         SysNet_get_netmask
 * @brief      获取掩码
 * @param[in]  strEthName      网卡名
 * @param[in]  pStAddr         缓冲区地址指针
 * @param[in]  uLength         缓冲区大小长度
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_get_netmask(const CHAR* strEthName,UINT8 *pStAddr,UINT16 uLength)
{
    INT32 iRet = -1;
    INT32 iFd = -1;
    struct ifreq stIfreq;
    struct sockaddr_in *pAddr;

    if (NULL == pStAddr || NULL == strEthName ) 
    {
        return iRet;
    }

    snprintf(stIfreq.ifr_name,sizeof(stIfreq.ifr_name),"%s",strEthName);
    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        LOG_ERROR("socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return iRet;
    }
    iRet = ioctl(iFd, SIOCGIFNETMASK, &stIfreq, sizeof(stIfreq));
    if(iRet < 0)
    {
        LOG_ERROR("ioctl get netmask failed - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return iRet;
    }
    pAddr = (struct sockaddr_in *)&stIfreq.ifr_netmask;
    snprintf((CHAR *)pStAddr, uLength, "%s", inet_ntoa(pAddr->sin_addr));
    close(iFd);

    return iRet;
}
/**@fn         SysNet_set_gateway
 * @brief      设置网关
 * @param[in]  strEthName      网卡名
 * @param[in]  pStAddr         缓冲区地址指针
 * @param[in]  uLength         缓冲区大小长度
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_set_gateway(const CHAR* strEthName,UINT8 *pStAddr,UINT16 uLength)
{
    INT32 iRet = -1;
    INT32 iFd = -1;
    struct rtentry stRoute;

    if (NULL == pStAddr) 
    {
        return iRet;
    }

    // 打开套接字
    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        LOG_ERROR("socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return iRet;
    }

    // 添加新的路由规则
    memset(&stRoute, 0, sizeof(stRoute));
    stRoute.rt_dst.sa_family = AF_INET;
    ((struct sockaddr_in *)&stRoute.rt_dst)->sin_addr.s_addr = INADDR_ANY;
    stRoute.rt_gateway.sa_family = AF_INET;
    inet_pton(AF_INET, (const CHAR *)pStAddr, &((struct sockaddr_in *)&stRoute.rt_gateway)->sin_addr);
    stRoute.rt_flags = RTF_GATEWAY;
    iRet = ioctl(iFd, SIOCADDRT, &stRoute);
    if (iRet < 0) {
        LOG_ERROR("ioctl set gateway failed - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return iRet;
    }

    close(iFd);

    return iRet;
}

/**@fn         SysNet_get_gateway
 * @brief      获取网关
 * @param[in]  strEthName      网卡名
 * @param[in]  pStAddr         缓冲区地址指针
 * @param[in]  uLength         缓冲区大小长度
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_get_gateway(const CHAR* strEthName,UINT8 *pStAddr,UINT16 uLength)
{
    FILE *pFp = NULL;
    INT32 iFirstLine = 0;
    CHAR strBuf[128];
    CHAR strName[16];
    struct in_addr stGateWay;
    UINT32 uAllLen = 0;
    UINT32 uNowLen = 0;

    if (!pStAddr || !strEthName) {
        LOG_ERROR("Invalid pStAddr(%p) strEthName(%p)\n", pStAddr, strEthName);
        return ERROR;
    }

    pFp = fopen ("/proc/net/route", "r");
    if (NULL == pFp) {
        LOG_ERROR ("can't open /proc/net/route - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
    }

    while (!feof (pFp)) {
        fgets (strBuf, 128, pFp);
        if (iFirstLine) {
            iFirstLine = 0;
            continue;
        }

        memset(&stGateWay, 0, sizeof(struct in_addr));
        sscanf (strBuf, "%s%*x%x", strName,  &stGateWay.s_addr);
        if (0 == strcmp(strName, strEthName) && stGateWay.s_addr) {
            memset(strBuf, 0, 128);
            inet_ntop(AF_INET, &stGateWay.s_addr, strBuf, 128);
            uAllLen += strlen(strBuf);
            if (uLength >= uAllLen) {
                if(uNowLen != 0) {
                    *(pStAddr+uNowLen) = '-';
                    uNowLen++;
                }
                strcpy((CHAR *)(pStAddr+uNowLen), strBuf);
                uNowLen = uAllLen;
                uAllLen++;
            }
        }
    }
 
    fclose (pFp);

    return 0;
}

/**@fn         SysNet_get_ip_mac
 * @brief      获取每个IP的mac地址
 * @param[in]  strIpAddr       IP地址
 * @param[in]  strNetCartName  网卡名
 * @param[out] strMac          mac地址
 * @param[in]  uLength         缓冲区长度
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_get_ip_mac(CHAR *strIpAddr, const CHAR *strNetCartName, UINT8 *strMac, UINT8 uLength)
{
    INT32               iSockfd = -1;
    UINT8              *ptr     = NULL;
    struct arpreq       arpreq;
    struct sockaddr_in *sin = NULL;

    memset(&arpreq, 0x00, sizeof(arpreq));

    if (!strIpAddr || !strMac || !strNetCartName)
    {
        LOG_ERROR("invaild param \n");
        return -1;
    }

    iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSockfd < 0)
    {
        LOG_ERROR("socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return -1;
    }

    sin                  = (struct sockaddr_in *)&arpreq.arp_pa;
    sin->sin_family      = AF_INET;
    sin->sin_addr.s_addr = inet_addr(strIpAddr);
    strncpy((char *)arpreq.arp_dev, (char *)strNetCartName, sizeof(arpreq.arp_dev) - 1);
    arpreq.arp_ha.sa_family = AF_UNSPEC;
    if (ioctl(iSockfd, SIOCGARP, &arpreq) < 0)
    {
        close(iSockfd);
        LOG_ERROR("ioctl SIOCGARP :%s :%s - errno[%d:%s]\n", strIpAddr, strNetCartName, errno, strerror(errno));
        return -1;
    }
    ptr = (UINT8 *)arpreq.arp_ha.sa_data;
    memcpy(strMac, ptr, 6);
    /* LOG_INFO("ip :%s device:%s %02x:%02x:%02x:%02x:%02x:%02x \n", strIpAddr, strNetCartName, strMac[0], strMac[1],
                    strMac[2], strMac[3], strMac[4], strMac[5]); */
    close(iSockfd);

    return 0;
}

/* @brief	  检测网络设备接口结点是否存在
 * @param[in] sDevName 设备名称
 * @return 成功返回 OK
 * @return 错误返回 ERROR, 不支持通过get_last_errno获取错误码
 */
INT32 SysNet_dev_node_check(const CHAR *strDevName)
{
    INT32 iRet = ERROR;
    INT32 iSocketFd = -1;
	struct ifreq ifr;
    if(!strDevName)
    {
        return iRet;
    }
    iSocketFd = socket(PF_INET, SOCK_STREAM, 0);
	if(iSocketFd < 0)
	{
		LOG_ERROR("create socket failed - errno[%d:%s]\n", errno, strerror(errno));
		return ERROR;
	}
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", strDevName);
    if(ioctl(iSocketFd, SIOCGIFFLAGS, &ifr) < 0)
    {
        LOG_ERROR("ioctl get flags failed. dev may not exist - errno[%d:%s]\n", errno, strerror(errno));
        close(iSocketFd);
        return ERROR;
    }

    if(!(ifr.ifr_ifru.ifru_flags&(IFF_UP)))
    {
        LOG_INFO("%s is down!\n", strDevName);
        close(iSocketFd);
        return OK;
    }
    close(iSocketFd);
    iRet = 1;
    return iRet;
}

/**@fn        SysNet_ifconfig_up
 * @brief	  启动网卡
 * @param[in] sDevName 网卡名
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_ifconfig_up(const CHAR *strName)
{
    INT32 iFd, iRtn;
    struct ifreq ifr;        

    if (!strName) {
        return -1;
    }

    iFd = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( iFd < 0 ) {
        LOG_ERROR("socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return -1;
    }
    
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, (const char *)strName, IFNAMSIZ - 1 );

    if ( (iRtn = ioctl(iFd, SIOCGIFFLAGS, &ifr) ) == 0 ) 
    { 
        ifr.ifr_flags |= IFF_UP;
    }

    if ( (iRtn = ioctl(iFd, SIOCSIFFLAGS, &ifr) ) != 0) {
        LOG_ERROR("ioctl set flags failed - errno[%d:%s]\n", errno, strerror(errno));
    }

    close(iFd);

    return iRtn;
}
/**@fn        SysNet_ifconfig_down
 * @brief	  关闭网卡
 * @param[in] sDevName 网卡名
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_ifconfig_down(const CHAR *strName)
{
    int iFd, iRtn;
    struct ifreq ifr;        

    if (!strName) {
        return -1;
    }

    iFd = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( iFd < 0 ) {
        LOG_ERROR("socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return -1;
    }
    
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, (const char *)strName, IFNAMSIZ - 1 );

    if ( (iRtn = ioctl(iFd, SIOCGIFFLAGS, &ifr) ) == 0 ) 
    {
        ifr.ifr_flags &= ~IFF_UP;
    }

    if ( (iRtn = ioctl(iFd, SIOCSIFFLAGS, &ifr) ) != 0) 
    {
        LOG_ERROR("ioctl set failed - errno[%d:%s]\n", errno, strerror(errno));
    }

    close(iFd);

    return iRtn;
}

/* @fn	      SysNet_ifconfig	  
 * @brief	  配置网卡IP、网关、掩码
 * @param[in] strDevName 设备网卡名
 * @param[in] strIP      IP地址
 * @param[in] strNetMask 掩码
 * @param[in]strGateWay  网关
 * @return 成功返回0错误，返回 -1
 */
INT32 SysNet_ifconfig(const CHAR *strDevName, const CHAR *strIP,const CHAR *strNetMask, const CHAR *strGateWay)
{
    /*1 set ip*/
    INT32 iFd = -1;
    struct ifreq stIfreq;
    struct sockaddr_in staddr;
	struct rtentry rt;
    if(!strDevName || !strIP || !strNetMask || !strGateWay)
    {
        LOG_ERROR("param error\n");
	    return ERROR;
    }
    if((iFd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
	    LOG_ERROR("create socket failed - errno[%d:%s]\n", errno, strerror(errno));
	    return ERROR;
    }
    snprintf(stIfreq.ifr_name, sizeof(stIfreq.ifr_name), "%s", strDevName);
    memset(&staddr, 0, sizeof(struct sockaddr_in));
    staddr.sin_family = PF_INET;
    if(inet_aton(strIP, &staddr.sin_addr)<0)
    {
        LOG_ERROR("inet_aton failed errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return ERROR;
    }
    memcpy(&stIfreq.ifr_ifru.ifru_addr, &staddr, sizeof(struct sockaddr_in));
    if(ioctl(iFd, SIOCSIFADDR, &stIfreq) < 0)
    {
	   LOG_ERROR("IP_set ioctl failed. error: %s!\n", strerror(errno));
	   close(iFd);
	   return ERROR;
    }
	
    /*2 set netmask*/
    memset(&staddr, 0, sizeof(struct sockaddr_in));
    staddr.sin_family = PF_INET;
	if(inet_aton(strNetMask, &staddr.sin_addr)<0)
    {
        LOG_ERROR("inet_aton failed - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return ERROR;
    }
    memcpy(&stIfreq.ifr_ifru.ifru_addr, &staddr, sizeof(struct sockaddr_in));
    if(ioctl(iFd, SIOCSIFNETMASK, &stIfreq) < 0)
    {
	   LOG_ERROR("IP_set ioctl failed. error: %s!\n", strerror(errno));
	   close(iFd);
	   return ERROR;
    }
	
    /*3 set gateway*/
	memset(&rt, 0, sizeof(struct rtentry));
    memset(&staddr, 0, sizeof(struct sockaddr_in));
    staddr.sin_family = PF_INET;
    staddr.sin_port = 0;
    if(inet_aton(strGateWay, &staddr.sin_addr)<0)
    {
        LOG_ERROR("inet_aton failed - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return ERROR;
    }
    memcpy(&rt.rt_gateway, &staddr, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(iFd, SIOCADDRT, &rt)<0)
    {
        LOG_ERROR("ioctl(SIOCADDRT) error in set_default_route - errno[%d:%s]\n", errno, strerror(errno));
        close(iFd);
        return ERROR;
    }
    close(iFd);

    LOG_INFO("== set OK!!! if: %s, ip: %s mask: %s ip: %s==\n", strDevName,strIP,strNetMask,strGateWay);
    return OK;
}


/* @fn              SysNet_arp_entry_exist
 * @brief           查询ARP条目是否存在
 * @param[in]       strDevName  网卡名称
 * @param[in]   	strIp       ARP条目ip地址
 * @return 存在返回 TRUE 不存在发挥 FALSE
*/
BOOL SysNet_arp_entry_exist(const CHAR* strDevName,const CHAR* strIp)
{
	INT32 iSock;
	INT32 iRet;
	struct arpreq stArpreq = {0};
	struct sockaddr_in *pstSin = NULL;
	if(strDevName == NULL || strIp == NULL) {
		  return ERROR;
	}
	/* 创建socket */
	iSock  = socket(AF_INET, SOCK_DGRAM, 0);
	if(iSock < 0)
    {
        LOG_ERROR("create socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
    }
	/* 设置要查询的IP地址*/
	pstSin = (struct sockaddr_in *)&stArpreq.arp_pa;
	pstSin->sin_family = AF_INET;
	iRet = inet_pton(AF_INET, strIp, &pstSin->sin_addr);
	if(iRet < 0)
    {
    	LOG_ERROR(" inet_pton ip to sin addr failed - errno[%d:%s]\n", errno, strerror(errno));
    	close(iSock);
        return ERROR;
    }
		
	 /* 设置接口*/
    strncpy(stArpreq.arp_dev, strDevName, sizeof(stArpreq.arp_dev)-1);
    stArpreq.arp_dev[sizeof(stArpreq.arp_dev)-1] = '\0';
    
	/* 使用ioctl获取ARP条目*/
	iRet  = ioctl(iSock, SIOCGARP, &stArpreq);
	if (iRet< 0) {
			close(iSock);
			return FALSE;
	}
	close(iSock);
	return TRUE;
}

/* @fn              SysNet_arp_entry_delete
 * @brief           删除ARP条目
 * @param[in]       strDevName  网卡名称
 * @param[in]   	strIp       ARP条目ip地址
 * @return 成功返回0;错误，返回 -1
 */
INT32 SysNet_arp_entry_delete(const CHAR* strDevName,const CHAR* strIp)
{
	INT32  iSock = ERROR;
	INT32  iRet = ERROR;
	struct arpreq stArpreq = {0};
	struct sockaddr_in *pstSin = NULL;
	if(strDevName == NULL || strIp == NULL) {
		  return ERROR;
	}

	iSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (iSock < 0) {
		LOG_ERROR("create socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
	}
	
	/*设置协议地址*/
	pstSin = (struct sockaddr_in *)&stArpreq.arp_pa;
	pstSin->sin_family = AF_INET;
	iRet = inet_pton(AF_INET, strIp, &pstSin->sin_addr);
	if(iRet < 0)
    {
    	LOG_ERROR(" inet_pton ip to sin addr failed - errno[%d:%s]\n", errno, strerror(errno));
    	close(iSock);
        return ERROR;
    }
	 /* 设置接口*/
    strncpy(stArpreq.arp_dev, strDevName, sizeof(stArpreq.arp_dev)-1);
    stArpreq.arp_dev[sizeof(stArpreq.arp_dev)-1] = '\0';
	
		
	/* 删除ARP条目 */
	if (ioctl(iSock, SIOCDARP, &stArpreq) < 0) {
		LOG_ERROR( "ioctl SIOCDARP failed\n");
		close(iSock);
		return ERROR;
	}
		
	close(iSock);
	return OK;
}

/* @fn              SysNet_arp_entry_add
 * @brief           添加ARP条目
 * @param[in]       stEntry  ARP条目
 * @return 成功返回0;错误，返回 -1
 */

INT32 SysNet_arp_entry_add(SYSNET_ARP_ENTRY_T *pstEntry)
{
	INT32  iSock = ERROR;
	INT32  iRet = ERROR;
    struct arpreq stArpreq = {0};
	struct sockaddr_in *pstSin = NULL;
    UINT8  u8Mac[6] = {0};
    iSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSock < 0) {
		LOG_ERROR("create socket failed - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
	}
    if(strlen(pstEntry->strMac) < 17)
    {
    	LOG_ERROR("Mac Address of ARP Entry is too short!\n");
        close(iSock);
        return ERROR;
    }
    /*解析MAC地址将字符串形式的转换成UINT8*/
    if (sscanf(pstEntry->strMac, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",&u8Mac[0], &u8Mac[1], &u8Mac[2], &u8Mac[3], &u8Mac[4], &u8Mac[5]) != 6 &&
		sscanf(pstEntry->strMac, "%02hhx-%02hhx-%02hhx-%02hhx-%02hhx-%02hhx",&u8Mac[0], &u8Mac[1], &u8Mac[2], &u8Mac[3], &u8Mac[4], &u8Mac[5]) != 6)
	{
        LOG_ERROR("Invalid MAC address format\n");
        close(iSock);
        return ERROR;
    }
     /* 设置硬件地址*/
    memcpy(stArpreq.arp_ha.sa_data, u8Mac, 6);
    stArpreq.arp_ha.sa_family = ARPHRD_ETHER;
    
    /* 设置协议地址*/
    pstSin = (struct sockaddr_in *)&stArpreq.arp_pa;
    pstSin->sin_family = AF_INET;
    /*ip地址进行装换*/
    iRet = inet_pton(AF_INET, pstEntry->strIP, &pstSin->sin_addr);
    if(iRet < 0)
    {
    	LOG_ERROR(" inet_pton ip to sin addr failed - errno[%d:%s]\n", errno, strerror(errno));
    	close(iSock);
        return ERROR;
    }
    /* 设置接口*/
    strncpy(stArpreq.arp_dev, pstEntry->strDevName, sizeof(stArpreq.arp_dev)-1);
    stArpreq.arp_dev[sizeof(stArpreq.arp_dev)-1] = '\0';
    
    /* 设置标志*/
    if(pstEntry->iFlags != SYS_ATF_UNKNOW){
    	stArpreq.arp_flags = pstEntry->iFlags;
    }
        
    /* 添加ARP条目*/
    iRet =ioctl(iSock, SIOCSARP, &stArpreq);
    if(iRet < 0){
        LOG_ERROR( "ioctl SIOCSARP failed\n");
		close(iSock);
		return ERROR;
    }
    
    close(iSock);
    return OK;
}

