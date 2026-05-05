
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/vfs.h>    /* or <sys/statfs.h> */

#include "SysMedium.h"
#include "SysUtils.h"
#include "SysPosix.h"
#include "SysFile.h"
#include "SysTime.h"
#include "Logger.h"
#include "fdisk/mkfs.h"

int make_fat_fs(const char *dev_name, int sectors_per_cluster, int sect_size, int fat_size, int volume_name);

extern int fdisk_make_part(const char *dev_name, unsigned int parts_num);

typedef enum  SYS_MEDIUM_FS_MAGIC_E
{    
    SYS_MEDIUM_FS_MAGIC_FAT32 = 0x4d44,
    SYS_MEDIUM_FS_MAGIC_EXT4  = 0xef53,
    SYS_MEDIUM_FS_MAGIC_EXFAT = 0x65735546,
} SYS_MEDIUM_FS_MAGIC_E;

/**@fn         SysMedium_fs_type_convert_data  
 * @brief      将文件系统类型转换对于的参数内容
 * @param[in]  eFsType         文件系统类型  
 * @param[in]  strFileSystem   文件系统字符串  
 * @param[in]  uSize           字符串缓冲区大小
 * @return 成果返回OK，失败返回ERROR,
 */
INT32 SysMedium_fs_type_convert_data(SYS_MEDIUM_FS_TYPE_E eFsType,char *strData,UINT16 uSize)
{
    INT32 iRet = ERROR;
    if(!strData)
    {
        return iRet;
    }
    switch(eFsType)
    {
        case SYS_MEDIUM_FS_VFAT:
            /* code */
            strncpy(strData,"shortname=mixed,errors=continue,fmask=0077",uSize);
            break;
        case SYS_MEDIUM_FS_EXT4:
            strncpy(strData,"data=ordered",uSize);
            break;
        case SYS_MEDIUM_FS_EXFAT:
            /* code */
            strncpy(strData,"iocharset=cp936,fmask=0077",uSize);
            break;
        case SYS_MEDIUM_FS_NTFS:
            /* do nothing */
            break;
        default:
            LOG_ERROR("no support type :%d \n",eFsType);
            return iRet;
    }

    return OK;
}

/**@fn         SysMedium_fs_type_convert_string  
 * @brief      将文件系统转换对应的挂载参数
 * @param[in]  eFsType         文件系统类型  
 * @param[in]  strFileSystem   文件系统字符串  
 * @param[in]  uSize           字符串缓冲区大小
 * @return 成果返回OK，失败返回ERROR,
 */
INT32 SysMedium_fs_type_convert_string(SYS_MEDIUM_FS_TYPE_E eFsType,char *strFileSystem,UINT16 uSize)
{   
    if(!strFileSystem)
    {
        return -1;
    }

    switch(eFsType)
    {
        case SYS_MEDIUM_FS_VFAT:
            /* code */
            strncpy(strFileSystem,"vfat",uSize);
            break;
        case SYS_MEDIUM_FS_XFS:
            strncpy(strFileSystem,"xfs",uSize);
            break;
        case SYS_MEDIUM_FS_EXT:
            strncpy(strFileSystem,"ext",uSize);
            break;
        case SYS_MEDIUM_FS_EXT4:
            strncpy(strFileSystem,"ext4",uSize);
            break;
        case SYS_MEDIUM_FS_NTFS:
            strncpy(strFileSystem,"ntfs",uSize);
            break;
        case SYS_MEDIUM_FS_NFS:
            strncpy(strFileSystem,"nfs",uSize);
            break;
        case SYS_MEDIUM_FS_EXFAT:
            /* code */
            strncpy(strFileSystem,"exfat",uSize);
            break;
        default:
            LOG_ERROR("no support type :%d \n",eFsType);
            return -1;
    }
    return 0;
}

/**@fn         sys_medium_mount     
 * @brief      将存储设备的分区挂载到目录中，目前只支持一个介质一个分区
 * @param[in]  strDevPath     节点路径
 * @param[in]  eFsType     文件系统类型
 * @param[in]  strMountPath挂载路径
 * @return     成果返回OK，失败返回ERROR,
 */
INT32 SysMedium_mount(const CHAR *strDevPath, SYS_MEDIUM_FS_TYPE_E eFsType, const CHAR *strMountPath)
{
    INT32 iRet = -1;
    CHAR strData[128] = {0};
    CHAR strFileSystem[16] = {0};

    if( NULL == strMountPath || NULL == strDevPath )
    {
        LOG_ERROR("invalid path params dev(%p) mount(%p)\n", strDevPath, strMountPath);
        return iRet;
    }

    iRet = SysMedium_fs_type_convert_string(eFsType,strFileSystem,sizeof(strFileSystem));
    if(iRet < 0)
    {
        LOG_ERROR("sys_medium_fs_type_convert_string type :%d error \n", eFsType);
        return iRet;
    }

    iRet = SysMedium_fs_type_convert_data(eFsType,strData,sizeof(strData));
    if(iRet < 0)
    {
        LOG_ERROR("convert data type :%d error \n", eFsType);
        return iRet;
    }

    if(SYS_MEDIUM_FS_NTFS == eFsType)
    {
        iRet = mount(strDevPath, strMountPath, strFileSystem, MS_MGC_VAL | MS_NOATIME, NULL);
    }
    else
    {
        //noatime,async,codepage=936,iocharset=gb2312
        iRet = mount(strDevPath, strMountPath, strFileSystem, MS_MGC_VAL | MS_NOATIME, strData);
    }
    if(iRet != OK)
    {
        LOG_ERROR("mount %s %s :%s eFsType:%d failed, %s\n", strDevPath, strMountPath,strFileSystem,eFsType, strerror(errno));
        return ERROR;
    }
    return iRet;
}

/**@fn         SysMedium_umount    
 * @brief      卸载被挂载的存储介质，目前只支持一个介质一个分区
 * @param[in]  strMountPath 挂载路径
 * @return     成果返回OK，失败返回ERROR
 */
INT32 SysMedium_umount(const CHAR *strMountPath)
{
    INT32 iRet = ERROR;
    if( NULL == strMountPath )
    {
        LOG_ERROR("invalid path params\n");
        return iRet;
    }
    if (SysPosix_access(strMountPath) < 0)
    {   
        LOG_ERROR("dev node path :%s ERROR:%s\n",strMountPath,strerror(errno));
        if(ENOTCONN == errno)
        {
            iRet = umount(strMountPath);
        }
        return iRet;
    }

    iRet = umount(strMountPath);
    if(iRet != OK)
    {
        LOG_ERROR("umount %s failed, %s\n", strMountPath, strerror(errno));
        return iRet;
    }
    return iRet;
}

/**@fn         SysMedium_make_part    
 * @brief      制作介质分区
 * @param[in]  strDevPath 介质节点  
 * @param[in]  iPartNum  分区个数  
 * @return     成功OK, 其他参考ERROR_CODE_E, 支持get_last_errno获取错误码
 */
INT32 SysMedium_make_part(const CHAR *strDevPath, INT32 iPartNum)
{
    INT32 iRet = ERROR;
    if( NULL == strDevPath )
    {
        LOG_ERROR("invalid path params\n");
        return iRet;
    }

    iRet = fdisk_make_part(strDevPath, iPartNum);
    if (iRet != OK)
    {
        LOG_ERROR("fdisk_make_part %s failed, %s\n", strDevPath, strerror(errno));
        return ERROR;
    }
    return OK;
}

/**@fn         SysMedium_make_part    
 * @brief      制作介质分区，支持多分区制作
 * @param[in]  strRootDevPath   介质根节点  
 * @param[in]  uPartNum         分区数  
 * @param[in]  pStPartInfo      分区信息，外部需要传入uPartNum对应个数
 * @return     成功OK, 其他参考ERROR_CODE_E, 支持get_last_errno获取错误码
 */
INT32 SysMedium_make_mutil_part(const CHAR *strRootDevPath, UINT32 uPartNum, SYS_MEDIUM_PARTITION_INFO_T *pStPartInfo)
{
    INT32 iRet = ERROR;
    INT32 iIndex = 0;
    UINT32 uRatio = 0;
    UINT32 uRatios[4] = {0};
    
    if(NULL == strRootDevPath || NULL == pStPartInfo)
    {
        return iRet;
    }

    /* 校验分区数，分区范围1~4 */
    if(uPartNum < 1 || uPartNum > 4)
    {
        LOG_ERROR("uPartitionNum support 1 ~ 4, but input %u\n", uPartNum);
        return iRet;
    }

    for(iIndex = 0; iIndex < uPartNum; iIndex++)
    {
        uRatio += (&pStPartInfo[iIndex])->uCapacityRatio;
        uRatios[iIndex] = (&pStPartInfo[iIndex])->uCapacityRatio;
    }

    /* 校验分区容量，总容量比必须为100 */
    if(100u != uRatio)
    {
        LOG_ERROR("all partition ratio sum not 100!!! uRatio:%u\n",uRatio);
        return iRet;
    }
    iRet = fdisk_make_part_ratio(strRootDevPath, uPartNum, uRatios);
    if (iRet != OK)
    {
        LOG_ERROR("fdisk_make_part %s failed\n", strRootDevPath);
        return ERROR;
    }
    return OK;
}

/**@fn         SysMedium_format
 * @brief      格式化介质
 * @param[in]  strDevPath节点路径
 * @param[in]  eFsType   文件系统类型 
 * @param[in]  unit size 分配单元大小
 * @return     成功OK, 其他参考ERROR_CODE_E, 支持get_last_errno获取错误码
 */
INT32 SysMedium_format(const CHAR *strDevPath, SYS_MEDIUM_FS_TYPE_E eFsType, UINT32 uUintSize)
{
    INT32 iRet = -1;
    char strFileSystem[8] ={0};
    char strExtendCmd[32] = {0};
    char strSysFormatCmd[64] = {0};
    UINT32 uBlockSize = 512;
    UINT32 uBlockNum = 0;
    uBlockNum =  uUintSize / uBlockSize;

    if( NULL == strDevPath )
    {
        LOG_ERROR("invalid path params\n");
        return ERROR;
    }
    switch(eFsType)
    {
        case SYS_MEDIUM_FS_VFAT:
            if(uBlockNum == 0)
            {
                strncpy(strExtendCmd,"-F 32",sizeof(strExtendCmd));    //FAT 32 默认 
            }
            snprintf(strExtendCmd, sizeof(strExtendCmd) - 1 ,"-F 32 -s %d", uBlockNum);
            break;
        case SYS_MEDIUM_FS_EXT:
        case SYS_MEDIUM_FS_EXT4:
            strncpy(strExtendCmd,"-m 0.05 -s 256",sizeof(strExtendCmd)); //ext预留分区 百分之5
            break;
        case SYS_MEDIUM_FS_EXFAT:
            if(uBlockNum == 0)
            {
                 strncpy(strExtendCmd,"-s 8192",sizeof(strExtendCmd)); //ext预留分区 百分之5,默认簇大小4MB
            }
            snprintf(strExtendCmd, sizeof(strExtendCmd) - 1 ,"-s %d", uBlockNum);
            break;
        default:
            break;
    }
#ifndef BUSYBOX_VERSION_1_26_2
    iRet = SysMedium_fs_type_convert_string(eFsType,strFileSystem,sizeof(strFileSystem));
    if(iRet < 0)
    {
        LOG_ERROR("sys_medium_fs_type_convert_string type :%d error \n", eFsType);
        return iRet;
    }
#else
    strncpy(strFileSystem,"fat",sizeof(strFileSystem) - 1);
    iRet = OK;
#endif
    
    //fat 32 16  12 格式化为FAT分区,
    //0代表全盘格式 其他数字代表分区格式
    snprintf(strSysFormatCmd,sizeof(strSysFormatCmd),"mkfs.%s %s %s",strFileSystem,strExtendCmd,strDevPath);
    iRet = SysPosix_cmd_call(strSysFormatCmd);
    if(iRet < 0)
    {
        LOG_ERROR("strSysFormatCmd %s error \n", strSysFormatCmd);
        return iRet;
    }
    sync();
    return iRet;
}
/* @brief       设备节点检查  不同平台不一致
 * @param[in]  strDevNodePath 设备节点
 * @return 成果返回OK，失败返回ERROR
 */
INT32 SysMedium_access_dev_node(const CHAR* strDevNodePath)
{
    INT32 iRet = ERROR;
    if(!strDevNodePath)
    {
        return iRet;
    }
    
    iRet = open(strDevNodePath,O_RDWR | O_NOCTTY | O_NDELAY);
    if(iRet < 0)
    {
        LOG_ERROR("open %s failed, %s\n", strDevNodePath, strerror(errno));
        return iRet;
    }
    
    close(iRet);
    return OK;
}
/**@brief      判断设备节点是否存在
 * @param[in]  strDevPath   节点路径  
 * @return     成果返回OK，失败返回ERROR,
 */
INT32 SysMedium_check_dev_node(const CHAR *strDevPath)
{
    /*规避措施 后续还是要去掉*/
	INT32 iRet = ERROR;
    INT32 iCnt = 0;

	if( NULL == strDevPath )
	{
		return ERROR;
	}
    /*规避措施 后续还是要去掉*/
	for(iCnt = 0; iCnt < 2; iCnt++)        /* 当前尝试两次 */
	{
		if(SysMedium_access_dev_node(strDevPath) == OK)
		{
			LOG_INFO("check dev node:%s successful\n", strDevPath);
			return 0;
		}

		LOG_INFO("check dev node:%s failed and try again, times = %d\n", strDevPath, iCnt);
		SysTime_sleep_ms(100);
	}
    return iRet;
}

/**@fn         sys_medium_get_info
 * @brief      medium获取介质信息
 * @param[in]  strPath       节点路径
 * @param[out] pStMediumInfo 介质信息指针  
 * @return     成功 ok 失败 ERROR
 */
INT32 sSysMedium_get_info(const CHAR *strMountPath, SYS_MEDIUM_INFO_T *pStMediumInfo)
{
    INT32 iRet = -1;
    UINT32 uDiskTotalSize = 0;
    UINT32 uDiskFreeSize = 0;
    struct statfs stInfo = {0};
    UINT32 uSysBlk = 0;
    
    if(!pStMediumInfo || !strMountPath)
    {
        LOG_ERROR("invalid params path(%p) info(%p)\n", strMountPath, pStMediumInfo);
        return iRet;
    }
    if (statfs(strMountPath, &stInfo) < 0)
    {
        pStMediumInfo->uTotalSize = 0;
        pStMediumInfo->uRemainSize = 0;
        pStMediumInfo->eType = SYS_MEDIUM_FS_TYPE_UNKNOWN;
        LOG_ERROR("statfs failed - errno[%d:%s]\n", errno, strerror(errno));
        return iRet;
    }

    if(stInfo.f_blocks < 1)
    {
        pStMediumInfo->uTotalSize = 0;
        pStMediumInfo->uRemainSize = 0;
        pStMediumInfo->eType = SYS_MEDIUM_FS_TYPE_UNKNOWN;
        return iRet;
    }
    
    switch (stInfo.f_type)
    {
        case SYS_MEDIUM_FS_MAGIC_EXFAT:
            /**
             * 当exfat使用fuseblk类型挂载时, f_blocks信息为(设备总大小/簇大小) f_files信息为(实际可使用大小(总大小-DBR-FAT)/簇大小)
             * 根据f_blocks和f_files得到DBR+FAT表占用的簇大小
             */
            uSysBlk += stInfo.f_blocks - stInfo.f_files;
        case SYS_MEDIUM_FS_MAGIC_FAT32:
            /* 根目录占用一个簇 */
            uSysBlk += 1;
            pStMediumInfo->eType = SYS_MEDIUM_FS_VFAT;
            break;
        case SYS_MEDIUM_FS_MAGIC_EXT4:
            pStMediumInfo->eType = SYS_MEDIUM_FS_EXT4;
            break;
        default:
            break;
    }

    /* 计算总容量时屏蔽根目录所占的一个簇的大小 */
    if(stInfo.f_bsize >= 1024)
    {
       uDiskTotalSize = (UINT32)((stInfo.f_bsize / 1024 ) * (stInfo.f_blocks - uSysBlk));
       uDiskFreeSize = (UINT32)(( stInfo.f_bsize / 1024 ) * stInfo.f_bavail);
    } else {
       uDiskTotalSize = (UINT32)(((stInfo.f_blocks - uSysBlk) * stInfo.f_bsize) / 1024);
       uDiskFreeSize = (UINT32) ((stInfo.f_bavail) * stInfo.f_bsize / 1024);
    }
    
 //   LOG_INFO("stInfo.f_bsize = %u\n", stInfo.f_bsize ); 
    pStMediumInfo->uClusterSize = stInfo.f_bsize;
    pStMediumInfo->uTotalSize = uDiskTotalSize/1024;
    pStMediumInfo->uRemainSize = uDiskFreeSize/1024;
    
    // LOG_INFO("strMountPath:%s uTotalSize:%d MB uTotalSize:%dMB f_type:0x%x\n",strMountPath,pStMediumInfo->uTotalSize,pStMediumInfo->uRemainSize,(UINT32)stInfo.f_type);
    return 0;
}

/**@fn         SysMedium_sync
 * @brief      强制写文件到存储介质      
 * @param[in]  strPath  盘符路径
 * @return     成功返回 0 错误返回 其他, 参考ERROR_CODE_E, 支持通过get_last_errno获取错误码
 */
INT32 SysMedium_sync(const CHAR *strPath)
{
    INT32 iRet = ERROR;
    if(NULL == strPath)
    {
        return iRet;
    } 

    sync();
    return OK;
}

/**
 * @brief       判断设备节点或指定路径是否以挂载并返回挂载路径或设备节点
 * @param[in]   pDeviceOrMountpoint   指定的设备节点或路径
 * @param[out]  pMountpointOrDevice   挂载的路径或设备节点
 * @return      值为0时表示已挂载，其余表示未挂载
 */
INT32 SysMedium_is_mounted(const CHAR *pDeviceOrMountpoint, CHAR *pMountpointOrDevice)
{
    FILE *pFp;
    CHAR strLine[128];
    CHAR strDevice[64], strMountpoint[64];

    if (!pDeviceOrMountpoint) {
        LOG_ERROR("invaild params(null)\n");
        return -EINVAL;
    }

    pFp = fopen("/proc/mounts", "r");
    if (pFp == NULL) {
        LOG_ERROR("Failed to open /proc/mounts - errno[%d:%s]\n", errno, strerror(errno));
        return ERROR;
    }

    while (fgets(strLine, sizeof(strLine), pFp)) {
        if (sscanf(strLine, "%255s %255s %*s %*s %*d %*d",
                   strDevice, strMountpoint) != 2) {
            continue;
        }

        if (strcmp(strDevice, pDeviceOrMountpoint) == 0 || 
            strcmp(strMountpoint, pDeviceOrMountpoint) == 0) {
            if(pMountpointOrDevice) {
                if (strcmp(strDevice, pDeviceOrMountpoint) == 0) {
                    strcpy(pMountpointOrDevice, strMountpoint);
                } else {
                    strcpy(pMountpointOrDevice, strDevice);
                }
            }
            fclose(pFp);
            return OK;
        }
        memset(strDevice, 0, sizeof(strDevice));
        memset(strMountpoint, 0, sizeof(strMountpoint));
        memset(strLine, 0, sizeof(strLine));
    }

    fclose(pFp);

    return ERROR;
}

/**
 * @brief      medium获取介质信息
 * @param[in]  strNodePath       设备节点，例/dev/mmcblk0 /dev/mmcblk0p1等
 * @param[out] puSize            设备节点大小 MB
 * @return     成功 ok 失败 ERROR
 */
INT32 SysMedium_get_node_size(const CHAR *strNodePath, UINT32 *puSize)
{
    INT32 iRet = ERROR;
    if(NULL == strNodePath || NULL == puSize)
    {
        LOG_ERROR("invalid param input\n");
        return iRet;
    }
    iRet = fdisk_get_dev_node_size(strNodePath, puSize);
    if(iRet < 0)
    {
        LOG_ERROR("call fdisk_get_dev_node_size error\n");
        return ERROR;
    }

    LOG_INFO("get dev node:%s size:%u\n", strNodePath, *puSize);
    
    return OK;
}

/**
 * @brief      获取根节点分区数量
 * @param[in]  strRootNode       设备根节点，不支持分区节点，例只支持/dev/mmcblk0
 * @param[out] puNum             分区数量
 * @return     成功 ok 失败 ERROR
 */
INT32 SysMedium_get_node_partition(const CHAR *strRootNode, UINT32 *puNum)
{
    INT32 iRet = ERROR;
    if(NULL == strRootNode || NULL == puNum)

    {
        LOG_ERROR("invalid param input\n");
        return iRet;
    }
    iRet = fdisk_get_dev_node_partition_num(strRootNode, puNum);
    if(iRet < 0)
    {
        LOG_ERROR("call fdisk_get_dev_node_size error\n");
        return ERROR;
    }

    LOG_INFO("get dev node:%s partition num:%u\n", strRootNode, *puNum);

    return OK;
}

