#include "VideoStream/VideoStream.h"
#include "Media.h"
#include <iostream>
#include "Logger.h"
#include "autoconf.h"
#include "MediaDec.h"
#include "lvgl/lvgl.h"
#include "ResourcePool.h"
#include "SysTime.h"
#include "SysMemory.h"
#include "MediaManager.h"

using namespace Page;
static lv_image_dsc_t videoimg ={0};
static uint32_t framebuf[720 * 1080] = {0};

VideoStream::VideoStream()
{


}

VideoStream::~VideoStream()
{
	if(this->mFramebuf)
		SysMemory_free(this->mFramebuf);
}

void VideoStream::onCustomAttrConfig()
{
	SetCustomCacheEnable(true);
	SetCustomLoadAnimType(PageManager::LOAD_ANIM_OVER_BOTTOM, 500, lv_anim_path_bounce);
}

void VideoStream::onViewLoad()
{
	View.Create(root);
	lv_label_set_text(View.ui.labelTitle, Name);

	AttachEvent(root);
	AttachEvent(View.ui.canvas);

	Model.TickSave = Model.GetData();
}

void VideoStream::onViewDidLoad()
{

}

void VideoStream::onViewWillAppear()
{
	Param_t param;
	param.color = lv_color_white();
	param.time = 100;

	PAGE_STASH_POP(param);

	lv_obj_set_style_bg_color(root, param.color, LV_PART_MAIN);

	timer = lv_timer_create(onTimerUpdate, param.time, this);
}

void VideoStream::onViewDidAppear()
{

}

void VideoStream::onViewWillDisappear()
{

}

void VideoStream::onViewDidDisappear()
{
	lv_timer_del(timer);
}

void VideoStream::onViewDidUnload()
{

}

void VideoStream::AttachEvent(lv_obj_t* obj)
{
	lv_obj_set_user_data(obj, this);
	lv_obj_add_event_cb(obj, onEvent, LV_EVENT_ALL, this);
}

void VideoStream::Update()
{

	UINT32 uLen = 0;
	videoimg.header.magic = LV_IMAGE_HEADER_MAGIC;
	videoimg.header.cf = LV_COLOR_FORMAT_ARGB8888;
	videoimg.header.flags = 0;
	videoimg.header.w = 720;
	videoimg.header.h = 1080;
	videoimg.data_size = 720*1080*4;
	uLen = GetDecStream(0,(void *)framebuf,0);
	lv_label_set_text_fmt(View.ui.labelTick, "tick = %d save = %d", Model.GetData(), Model.TickSave);
	videoimg.data = (const uint8_t *)framebuf;
	lv_img_set_src(View.ui.canvas,&videoimg);
	lv_obj_invalidate(View.ui.canvas);

}

void VideoStream::onTimerUpdate(lv_timer_t* timer)
{
	VideoStream* instance = (VideoStream*)lv_timer_get_user_data(timer);
	instance->Update();
}

void VideoStream::onEvent(lv_event_t* event)
{
	lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(event);
	lv_event_code_t code = lv_event_get_code(event);
	auto* instance = (VideoStream*)lv_obj_get_user_data(obj);

	if (code == LV_EVENT_PRESSED)
	{
		if (lv_obj_has_state(obj, LV_STATE_FOCUSED))
		{
			instance->Manager->Push("Pages/Template");
		}
	}
}
