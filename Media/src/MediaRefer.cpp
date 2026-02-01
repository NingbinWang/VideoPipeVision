#include <string.h>
#include "MediaRefer.h"


#define RESERVED_BYTE   0xff

unsigned int MediaRefer_fill_basic_descriptor(unsigned char *buffer, MEDIAREFER_TIME_T *glb_time, unsigned int encrypt_type,unsigned int company_mark,unsigned int camera_mark)
{
    buffer[0]	= BASIC_DESCRIPTOR_TAG; //  descriptor_tag: 0x40
    buffer[1]	= BASIC_DESCRIPTOR_LEN - 2; //  descriptor_length;  
    buffer[2]	= (unsigned char)(company_mark >> 8); //  company_mark_h;     公司描述符，0x48, "H"
    buffer[3]	= (unsigned char)company_mark; //  company_mark_l;     公司描述符，0x4b, "K" 		
    buffer[4]	= (unsigned char)(DESCRIPTOR_DEF_VERSION >> 8);//  def_version_h;      描述符定义版本，第一版为0x00
    buffer[5]	= (unsigned char)(DESCRIPTOR_DEF_VERSION);       //  def_version_l;      描述符定义版本，第一版为0x01
    buffer[6]	= (unsigned char)((glb_time->year - DESCRIPTOR_YEAR_BASE) & 0xff);    //  全局时间年 8bit，1表示2001年，后面类推，不得出现0
    buffer[7]	= (unsigned char)(((unsigned char)(glb_time->month << 4) & 0xf0)      //  月4bit
                | ((unsigned char)(glb_time->date >> 1) & 0x0f));      //  日5bit
    buffer[8]	= (unsigned char)(((unsigned char)(glb_time->date << 7) & 0x80)
                | ((unsigned char)(glb_time->hour << 2) & 0x7c)       //  时5bit
                | ((unsigned char)(glb_time->minute >> 4) & 0x03));    //  分6bit
    buffer[9]	= (unsigned char)(((unsigned char)(glb_time->minute << 4) & 0xf0)
                | ((unsigned char)(glb_time->second >> 2) & 0x0f));    //  秒6bit
    buffer[10]	= (unsigned char)(((unsigned char)(glb_time->second << 6) & 0xc0)     
                | (1 << 5)                                              //  插入1bit
                | ((unsigned char)(glb_time->msecond >> 5) & 0x1f));   //  毫秒10bit
    buffer[11]	= (unsigned char)(((unsigned char)(glb_time->msecond << 3) & 0xf8)
                | ((unsigned char)(encrypt_type) & 0x07));  //  加密类型 3 bit
    buffer[12]	= (unsigned char)camera_mark;
    buffer[13]	= RESERVED_BYTE;        
    buffer[14]	= RESERVED_BYTE;
    buffer[15]	= RESERVED_BYTE;
    return BASIC_DESCRIPTOR_LEN;
}


unsigned int MediaRefer_fill_stream_descriptor(unsigned char *buffer, 
										  unsigned int video_stream_type,
										  unsigned int audio_stream_type,
										  unsigned int video_frame_num)
{
    buffer[0]	= STREAM_DESCRIPTOR_TAG;			//  descriptor_tag: 0x45
    buffer[1]	= STREAM_DESCRIPTOR_LEN - 2;		//  descriptor_length;  
    buffer[2]	= (unsigned char)video_stream_type;	 
    buffer[3]	= (unsigned char)audio_stream_type;	 
	if (video_stream_type == STREAM_TYPE_UNDEF)
	{
		video_frame_num = 0;
	}
    
	buffer[4]	= (unsigned char)(video_frame_num >> 24);
	buffer[5]	= (unsigned char)(video_frame_num >> 16);
	buffer[6]	= (unsigned char)(video_frame_num >> 8);
	buffer[7]	= (unsigned char)(video_frame_num);

    buffer[8]	= RESERVED_BYTE;
    buffer[9]	= RESERVED_BYTE;        
    buffer[10]	= RESERVED_BYTE;
    buffer[11]	= RESERVED_BYTE;
    return STREAM_DESCRIPTOR_LEN;
}


unsigned int MediaRefer_fill_video_descriptor(unsigned char *buffer, MEDIAREFER_VIDEO_INFO_T *video_info)
{
    buffer[0]	= VIDEO_DESCRIPTOR_TAG;     //  descriptor_tag: 0x42
    buffer[1]	= VIDEO_DESCRIPTOR_LEN - 2; //  descriptor_length;  

    //encoder info
    buffer[2]	= (unsigned char)(video_info->encoder_version >> 8);   //编码器版本
    buffer[3]	= (unsigned char)(video_info->encoder_version);	

    buffer[4]	= (unsigned char)(((unsigned char)((video_info->encoder_year - DESCRIPTOR_YEAR_BASE) & 0x7f) << 1) // 编码器时间 年 7bit
                | ((unsigned char)(video_info->encoder_month & 0x0f) >> 3)); // 编码器时间 月 4bit
    buffer[5]	= (unsigned char)(((unsigned char)(video_info->encoder_month & 0x07) << 5)
                | ((unsigned char)video_info->encoder_date & 0x1f));    // 编码器时间 日 5bit

    //width and height
    buffer[6]	= (unsigned char)(video_info->width_orig >> 8);
    buffer[7]	= (unsigned char)(video_info->width_orig);
    buffer[8]	= (unsigned char)(video_info->height_orig >> 8);
    buffer[9]	= (unsigned char)(video_info->height_orig );

    //encoder_flag
    buffer[10]	= (unsigned char)(((unsigned char)(video_info->interlace & 0x01) << 7)
                | ((unsigned char)(video_info->b_frame_num & 0x03) << 5)
                | ((unsigned char)(video_info->is_svc_stream & 0x01) << 4)
                | ((unsigned char)(video_info->use_e_frame & 0x01) << 3)
                | ((unsigned char)video_info->max_ref_num & 0x07));

    buffer[11]	= (unsigned char)(((unsigned char)(video_info->watermark_type & 0x07) << 5)
                | ((unsigned char)(video_info->deinterlace & 0x01) << 4)
	            | ((unsigned char)(video_info->min_search_blk & 0x03) << 2)
	            | ((unsigned char)(video_info->light_storage & 0x03)));


	/* 考虑到MP4封装库使用的结构体HIK_VIDEO_INFO中没有jpeg_quality，为了保持统一性，将其去掉*/
  	buffer[12]	= RESERVED_BYTE; //(unsigned char)(video_info->jpeg_quality);
    //time_info
    buffer[13]	= (unsigned char)(video_info->time_info >> 15);
    buffer[14]	= (unsigned char)(video_info->time_info >> 7);  
    buffer[15]	= (unsigned char)(((unsigned char)(video_info->time_info << 1) & 0xfe)
                | ((unsigned char)(video_info->fixed_frame_rate & 0x01)));  
    return VIDEO_DESCRIPTOR_LEN;
}


unsigned int MediaRefer_fill_audio_descriptor(unsigned char *buffer, MEDIAREFER_AUDIO_INFO_T *audio_info)
{
    buffer[0]	= AUDIO_DESCRIPTOR_TAG;
    buffer[1]	= AUDIO_DESCRIPTOR_LEN - 2; 

    buffer[2]	= (unsigned char)(audio_info->frame_len >> 8);
    buffer[3]	= (unsigned char)(audio_info->frame_len);	
    buffer[4]	= (0x7f << 1) 
    			| ((unsigned char)(audio_info->audio_num  & 0x01));

    buffer[5]	= (unsigned char)(audio_info->sample_rate >> 14);
    buffer[6]   = (unsigned char)(audio_info->sample_rate >> 6);
    buffer[7]   = ((unsigned char)(audio_info->sample_rate << 2) & 0xfc)
                | 0x03;
    buffer[8]	= (unsigned char)(audio_info->bit_rate >> 14);
    buffer[9]	= (unsigned char)(audio_info->bit_rate >> 6);
    buffer[10]	= ((unsigned char)(audio_info->bit_rate << 2) & 0xfc)
                | 0x03;
    buffer[11]  = RESERVED_BYTE;
    return AUDIO_DESCRIPTOR_LEN;
}


unsigned int MediaRefer_fill_device_descriptor(unsigned char *buffer, unsigned char dev_chan_id[16])
{
    buffer[0]	= DEVICE_DESCRIPTOR_TAG;
    buffer[1]	= DEVICE_DESCRIPTOR_LEN - 2; 

    buffer[2]	= 0x48; //海康设备标志 "HK"
    buffer[3]	= 0x4b;	

    memcpy(&buffer[4], dev_chan_id, 16);
    return DEVICE_DESCRIPTOR_LEN;
}


unsigned int MediaRefer_fill_video_clip_descriptor(unsigned char *buffer, MEDIAREFER_VIDEO_INFO_T *video_info)
{
    buffer[0]	= VIDEO_CLIP_DESCRIPTOR_TAG;
    buffer[1]	= VIDEO_CLIP_DESCRIPTOR_LEN - 2; 

    buffer[2]	= (unsigned char)(video_info->start_pos_x >> 8);
    buffer[3]	= (unsigned char)video_info->start_pos_x;	
    buffer[4]   = (unsigned char)(video_info->start_pos_y >> 8) | 0x80;
    buffer[5]   = (unsigned char)video_info->start_pos_y;

    buffer[6]	= (unsigned char)(video_info->width_play >> 8);
    buffer[7]	= (unsigned char)video_info->width_play;	
    buffer[8]	= (unsigned char)(video_info->height_play >> 8);
    buffer[9]	= (unsigned char)video_info->height_play;
    
    buffer[10]  = RESERVED_BYTE;
    buffer[11]  = RESERVED_BYTE;

    return VIDEO_CLIP_DESCRIPTOR_LEN;
}


unsigned int MediaRefer_fill_timing_hrd_descriptor(unsigned char *buffer, int frame_rate, int width, int height)
{
	unsigned int num_units_in_ticks;

	if (frame_rate == 0) //防止除零
	{
		frame_rate = 25;
	}

	num_units_in_ticks = (unsigned int)(90000 / (frame_rate * 2));

	buffer[0] = TIMING_HRD_DESCRIPTOR_TAG;
	buffer[1] = TIMING_HRD_DESCRIPTOR_LEN - 2;
	buffer[2] = 0x7f;
	buffer[3] = 0xff;
	buffer[4] = (unsigned char)(num_units_in_ticks >> 24);
	buffer[5] = (unsigned char)(num_units_in_ticks >> 16);
	buffer[6] = (unsigned char)(num_units_in_ticks >> 8);
	buffer[7] = (unsigned char)num_units_in_ticks;
	buffer[8] = 0x1f;
	buffer[9] = 0xfe;
	buffer[10] = (unsigned char)(width >> 3);
	buffer[11] = (unsigned char)(height >> 3);

	return TIMING_HRD_DESCRIPTOR_LEN;
}

