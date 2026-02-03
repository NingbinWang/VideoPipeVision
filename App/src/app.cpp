#include "app.h"
#include "Common.h"
#include "Media.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include "Logger.h"
#include "UsageEnvironment.h"
#include "ThreadPool.h"
#include "EventScheduler.h"
#include "Event.h"
#include "RtspServer.h"
#include "MediaSession.h"
#include "InetAddress.h"
#include "H264RtpSink.h"
#include "Common.h"
#include "autoconf.h"
#include "VideoMediaSource.h"
#include <sys/resource.h>
#include "SysFile.h"
#include <fcntl.h>      // 定义 O_RDWR, O_CREAT 等标志
#include <unistd.h>     // 定义 open, read, write, close 等函数原型
#include <sys/types.h>  // 通常也需要，定义 off_t 等类型（有时 unistd.h 会包含它）
#include "SysTime.h"
#ifdef USE_LVGL
#include "LvThread.h"
#endif
#ifdef USE_AI
#include "AiModel.h"
#endif
#include "MediaManager.h"
#include <pthread.h>

UsageEnvironment* env = NULL;

void* AppRtspServerFun(void* UserData)
{
	/*Handle LVGL tasks*/
	  while(1) {
		  env->scheduler()->loop();
	  }
}

INT32 AppRtspServer(const char* strIp)
{
    EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT);//创建调度器
    ThreadPool* threadPool = ThreadPool::createNew(2);//创建线程池
    env = UsageEnvironment::createNew(scheduler, threadPool);//创建环境变量

    Ipv4Address ipAddr(strIp, 5001);//创建IP
    RtspServer* server = RtspServer::createNew(env, ipAddr);//创建对应的RTSPServer
    MediaSession* session = MediaSession::createNew("live");//创建一个session
    MediaSource* mediaSource = VideoMediaSource::createNew(env); 
    RtpSink* rtpSink = H264RtpSink::createNew(env, mediaSource);
	//MediaSource* audioSource = AlsaMediaSource::createNew(env);
    //RtpSink* audioRtpSink = AACRtpSink::createNew(env, audioSource);
    
    session->addRtpSink(MediaSession::TrackId0, rtpSink);
   // session->addRtpSink(MediaSession::TrackId1, audioRtpSink);
    server->addMeidaSession(session);
    server->start();
    std::cout<<"Play the media using the URL \""<<server->getUrl(session)<<"\""<<std::endl;
    return 0;
}


/* @fn      sys_core_dump_open
 * @brief   system core dump open.
 * @brief   Author/Date huangjiangsheng/2013-08-01.
 * @param   [in] pCorePid  : coredump pid mode.
 * @param   [in] pCorePath : coredump path.
 * @param   [out] N/A.
 * @return  OK/ERROR.
 */
static int sys_core_dump_open(char *pCorePid, char *pCorePath)
{
    int iRet = -1;
    int iFd1 = -1;
    int iFd2 = -1;
    struct rlimit limit;
    struct rlimit limit_set;

    do{
        /* 1, set core ulimit */
     	if (getrlimit(RLIMIT_CORE, &limit))
        {
            LOG_ERROR("get resource limit fail!\n");
     		break;
     	}
     	limit_set.rlim_cur = limit_set.rlim_max = RLIM_INFINITY;
     	if (setrlimit(RLIMIT_CORE, &limit_set))
        {
			limit_set.rlim_cur = limit_set.rlim_max = limit.rlim_max;
			if (limit.rlim_max != RLIM_INFINITY)
			{
				LOG_ERROR("CORE: cur=0x%x, max=0x%x\n",
				(UINT)limit.rlim_cur, (UINT)limit.rlim_max);
			}
			if (setrlimit(RLIMIT_CORE, &limit_set))
			{
				LOG_ERROR("set core ulimited fail!\n");
				break;
			}
     	}
        /* 2, set core use pid */
        if (pCorePid && strlen(pCorePid) > 0)
        {
            iFd1 = open("/proc/sys/kernel/core_uses_pid", O_RDWR|O_NDELAY|O_TRUNC);
            if (iFd1 < 0)
            {
                LOG_ERROR("open core_uses_pid fail! 0x%x\n", iFd1);
                break;
            }
            if (strlen(pCorePid) != (unsigned int)write(iFd1, pCorePid, strlen(pCorePid)))
            {
                LOG_ERROR("set core_uses_pid fail!\n");
                break;
            }
        }

        /* 3, set core pattern */
        if (pCorePath && strlen(pCorePath) > 0)
        {
            iFd2 = open("/proc/sys/kernel/core_pattern", O_RDWR|O_NDELAY|O_TRUNC);
            if (iFd2 < 0)
            {
                LOG_ERROR("open core_pattern fail! %d\n", iFd1);
                break;
            }
            if (strlen(pCorePath) != (unsigned int)write(iFd2, pCorePath, strlen(pCorePath)))
            {
                LOG_ERROR("set core_pattern fail!\n");
                break;
            }
        }

        /* 4, set core dump open succ */
        iRet = OK;
    }while(0);

    /* source free */
    close(iFd1);
    close(iFd2);

    return iRet;
}

static void set_core_dump_enable(void)
{
	char szCorePid[32] = {0};
	char szCorePath[128]={0};

	/*为了唯一区分core文件,文件名格式追加IP地址及设备序列号*/
	strcpy(szCorePid, "1");

	/*core保存目录*/
	strcpy(szCorePath, "/tmp/");

	/*core 系统参数信息 */
	strcat(szCorePath, "coreDump-%s-%e-%p-%t");

	if (OK != sys_core_dump_open(szCorePid, szCorePath))
	{
		LOG_ERROR("========= gdb core dump open fail! =========\n\n");
	}
	else
	{
		LOG_INFO("sys gdb core open succ ~~~ \n");
	}
}



int app_main(void)
{
  
   //Logger::setLogFile("xxx.log");
   pthread_t id;
   Logger::setLogLevel(Logger::LogDebug);
#ifdef USE_AI
   //AiModelInit();
#endif
	MediaManagerInit();
#ifdef USE_LVGL
   LvThreadInit();
#endif
	set_core_dump_enable();
	SysTime_sleep_ms(500);
	AppRtspServer("192.168.0.14");
	pthread_create(&id,NULL,AppRtspServerFun,NULL);
	while(1)
	{
	  SysTime_sleep_ms(100);
	}
   return 0;
}
