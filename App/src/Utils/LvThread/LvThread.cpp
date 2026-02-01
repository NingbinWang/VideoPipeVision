#include "lvgl/lvgl.h"
#include "Factory.h"
#include "PageManager.h"
#include "Thread.h"
#include "LvThread.h"
#include "ResourcePool.h"
#include "Logger.h"
#include "StatusBar.h"
#include "LvIndev.h"
#include "LvDisplay.h"
#include "Common.h"
#include <unistd.h>
#include "PagesNotify.h"
#include "SysPthread.h"
#include <pthread.h>

static void ShowLvglVersion(void)
{
    LOG_INFO("lvglversion:%d.%d.%d-%s\n",LVGL_VERSION_MAJOR,LVGL_VERSION_MINOR,LVGL_VERSION_PATCH,LVGL_VERSION_INFO);
}

static int LvLinuxPorting(void)
{
   lv_init();
   ShowLvglVersion();
   /*Linux display device init*/
   LvLinuxDispInit();
   LvLinuxIndevInit();
   return 0;
}

void* LvThreadProcess(void* UserData)
{
	/*Handle LVGL tasks*/
	  while(1) {
		  lv_timer_handler();
		  usleep(5000);
	  }
}


int LvThreadInit(void)
{
	static Factory factory;
    static PageManager manager(&factory);
	pthread_t id;
	LvLinuxPorting();
	PagesNotify_Init();
    Resource.Init();
    /*----------------------- Pages Init -----------------------*/
    StatusBar::Init(lv_layer_top());
    manager.Install("Startup", "Pages/Startup");
	manager.Install("Template", "Pages/Template");
	manager.Install("VideoStream", "Pages/VideoStream");
    manager.SetGlobalLoadAnimType(PageManager::LOAD_ANIM_OVER_TOP, 500);
    manager.Push("Pages/Startup");
    pthread_create(&id,NULL,LvThreadProcess,NULL);
}
