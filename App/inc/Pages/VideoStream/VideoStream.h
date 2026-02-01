#ifndef  _VIDEOSTREAM_H_
#define  _VIDEOSTREAM_H_

#include "VideoStreamView.h"
#include "VideoStreamModel.h"

namespace Page
{

class VideoStream : public PageBase
{
public:
    typedef struct
    {
        uint16_t time;
        lv_color_t color;
    } Param_t;

public:
    VideoStream();
    virtual ~VideoStream();

    virtual void onCustomAttrConfig();
    virtual void onViewLoad();
    virtual void onViewDidLoad();
    virtual void onViewWillAppear();
    virtual void onViewDidAppear();
    virtual void onViewWillDisappear();
    virtual void onViewDidDisappear();
    virtual void onViewDidUnload();

private:
    void Update();
    void AttachEvent(lv_obj_t* obj);
    static void onTimerUpdate(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);
	unsigned int GetStream(unsigned int uChan,void *pUserData);

private:
    VideoStreamView View;
    VideoStreamModel Model;
    lv_timer_t* timer;
	char *mFramebuf=nullptr;
};

}

#endif
