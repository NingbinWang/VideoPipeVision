
#include <stdlib.h>
#include <string.h>
#include "MediaPs.h"
#include "Logger.h"


//符合mpeg2 part1定义的crc32码表
unsigned int PSSTREAM_CTX[257] = 
{
	0x00000000,0xb71dc104,0x6e3b8209,0xd926430d,0xdc760413,0x6b6bc517,0xb24d861a,0x0550471e,
	0xb8ed0826,0x0ff0c922,0xd6d68a2f,0x61cb4b2b,0x649b0c35,0xd386cd31,0x0aa08e3c,0xbdbd4f38,
	0x70db114c,0xc7c6d048,0x1ee09345,0xa9fd5241,0xacad155f,0x1bb0d45b,0xc2969756,0x758b5652,
	0xc836196a,0x7f2bd86e,0xa60d9b63,0x11105a67,0x14401d79,0xa35ddc7d,0x7a7b9f70,0xcd665e74,
	0xe0b62398,0x57abe29c,0x8e8da191,0x39906095,0x3cc0278b,0x8bdde68f,0x52fba582,0xe5e66486,
	0x585b2bbe,0xef46eaba,0x3660a9b7,0x817d68b3,0x842d2fad,0x3330eea9,0xea16ada4,0x5d0b6ca0,
	0x906d32d4,0x2770f3d0,0xfe56b0dd,0x494b71d9,0x4c1b36c7,0xfb06f7c3,0x2220b4ce,0x953d75ca,
	0x28803af2,0x9f9dfbf6,0x46bbb8fb,0xf1a679ff,0xf4f63ee1,0x43ebffe5,0x9acdbce8,0x2dd07dec,
	0x77708634,0xc06d4730,0x194b043d,0xae56c539,0xab068227,0x1c1b4323,0xc53d002e,0x7220c12a,
	0xcf9d8e12,0x78804f16,0xa1a60c1b,0x16bbcd1f,0x13eb8a01,0xa4f64b05,0x7dd00808,0xcacdc90c,
	0x07ab9778,0xb0b6567c,0x69901571,0xde8dd475,0xdbdd936b,0x6cc0526f,0xb5e61162,0x02fbd066,
	0xbf469f5e,0x085b5e5a,0xd17d1d57,0x6660dc53,0x63309b4d,0xd42d5a49,0x0d0b1944,0xba16d840,
	0x97c6a5ac,0x20db64a8,0xf9fd27a5,0x4ee0e6a1,0x4bb0a1bf,0xfcad60bb,0x258b23b6,0x9296e2b2,
	0x2f2bad8a,0x98366c8e,0x41102f83,0xf60dee87,0xf35da999,0x4440689d,0x9d662b90,0x2a7bea94,
	0xe71db4e0,0x500075e4,0x892636e9,0x3e3bf7ed,0x3b6bb0f3,0x8c7671f7,0x555032fa,0xe24df3fe,
	0x5ff0bcc6,0xe8ed7dc2,0x31cb3ecf,0x86d6ffcb,0x8386b8d5,0x349b79d1,0xedbd3adc,0x5aa0fbd8,
	0xeee00c69,0x59fdcd6d,0x80db8e60,0x37c64f64,0x3296005a,0x858bc97e,0x5cad8a73,0xebb04b77,
	0x560d044f,0xe110c54b,0x38368646,0x8f2b4742,0x8a7b005c,0x3d66c158,0xe4408255,0x535d4351,
	0x9e3b1d25,0x2926dc21,0xf0009f2c,0x471d5e28,0x424d1936,0xf550d832,0x2c769b3f,0x9b6b5a3b,
	0x26d61503,0x91cbd407,0x48ed970a,0xfff0560e,0xfaa01110,0x4dbdd014,0x949b9319,0x2386521d,
	0x0e562ff1,0xb94beef5,0x606dadf8,0xd7706cfc,0xd2202be2,0x653deae6,0xbc1ba9eb,0x0b0668ef,
	0xb6bb27d7,0x01a6e6d3,0xd880a5de,0x6f9d64da,0x6acd23c4,0xddd0e2c0,0x04f6a1cd,0xb3eb60c9,
	0x7e8d3ebd,0xc990ffb9,0x10b6bcb4,0xa7ab7db0,0xa2fb3aae,0x15e6fbaa,0xccc0b8a7,0x7bdd79a3,
	0xc660369b,0x717df79f,0xa85bb492,0x1f467596,0x1a163288,0xad0bf38c,0x742db081,0xc3307185,
	0x99908a5d,0x2e8d4b59,0xf7ab0854,0x40b6c950,0x45e68e4e,0xf2fb4f4a,0x2bdd0c47,0x9cc0cd43,
	0x217d827b,0x9660437f,0x4f460072,0xf85bc176,0xfd0b8668,0x4a16476c,0x93300461,0x242dc565,
	0xe94b9b11,0x5e565a15,0x87701918,0x306dd81c,0x353d9f02,0x82205e06,0x5b061d0b,0xec1bdc0f,
	0x51a69337,0xe6bb5233,0x3f9d113e,0x8880d03a,0x8dd09724,0x3acd5620,0xe3eb152d,0x54f6d429,
	0x7926a9c5,0xce3b68c1,0x171d2bcc,0xa000eac8,0xa550add6,0x124d6cd2,0xcb6b2fdf,0x7c76eedb,
	0xc1cba1e3,0x76d660e7,0xaff023ea,0x18ede2ee,0x1dbda5f0,0xaaa064f4,0x738627f9,0xc49be6fd,
	0x09fdb889,0xbee0798d,0x67c63a80,0xd0dbfb84,0xd58bbc9a,0x62967d9e,0xbbb03e93,0x0cadff97,
	0xb110b0af,0x060d71ab,0xdf2b32a6,0x6836f3a2,0x6d66b4bc,0xda7b75b8,0x035d36b5,0xb440f7b1,
	0x00000001
};

/******************************************************************************
* 功  能：生成crc32校验码，参考ffmpeg
* 参  数：  buffer  - 生成crc码的缓冲区
* length  - 生成crc码的缓冲区长度
* 返回值：crc校验码
******************************************************************************/
unsigned int MediaPs_mpeg2_crc(const unsigned char *buffer, unsigned int length)
{
    const unsigned char *end= buffer+length;
    unsigned int crc = 0xffffffff;
    while(buffer < end)
    {
        crc = PSSTREAM_CTX[((unsigned char)crc) ^ *buffer++] ^ (crc >> 8);
    }
    return crc;
}

/******************************************************************************
* 功  能：根据 PES_PACK_INFO *pes 内的信息在 buffer 里生成 PES 头数据
* 参  数：  buffer      - PES 头缓冲区
*           pes         - 当前处理的pes
*           prc         - 当前处理的单元
*           pes         - 输入输出参数结构体 
* 返回值：PES 头数据的长度
******************************************************************************/
unsigned int MediaPs_create_pes_header(unsigned char *buffer, MEDIAPS_PES_PACK_INFO_T *pes, MEDIAPS_COMPLEX_PROCESS_PARAM_T *prc, MEDIAPS_PS_INFO_T *prg)
{
    unsigned int    i, pes_header_stuff_len = 0;
	unsigned int    tmp;
	unsigned int    pes_header_basic_len    = 9;
    unsigned int    pes_header_ext_len      = 0;
    unsigned int    pos, pts, stuff_flag = 0xff;
    unsigned int    pes_header_len_syt;
	unsigned int	pes_heder_fix_len;
    unsigned int    pes_priority = (prc->uFrameType != FRAME_TYPE_VIDEO_BFRAME);

    if ((pes->uRestDataLen == pes->uTotalDataLen) && prc->uIsUnitStart)
    {
        stuff_flag  &= 0xfd;
    }

    // 计算 pes 头字节和填充字节的长度
    if (pes->uAddpts)
    {
        pes_header_ext_len += 5;
    }

	pes_heder_fix_len = pes_header_basic_len + pes_header_ext_len + MIN_PES_HEAD_STUFF_LEN; 
    tmp = pes_heder_fix_len + pes->uRestDataLen;
	
    if (tmp > prg->uMaxPesLen)
    {     
		if ((tmp == prg->uMaxPesLen) || ((tmp & 3) == 0))
		{                                                              
			pes_header_stuff_len = 2;
		}
		else
		{
			pes_header_stuff_len = 2 + (4 - (tmp & 3));
		}
        pes_header_ext_len   +=  pes_header_stuff_len;
        pes->uPayloadLen     =   prg->uMaxPesLen - pes_header_basic_len - pes_header_ext_len;
        
        // 确保payload长度不为负数
        if (pes->uPayloadLen > pes->uRestDataLen)
        {
            pes->uPayloadLen = pes->uRestDataLen;
        }
    }
    else
    {
        // pes_header_stuff_len 至少为2，且要保证 pes 包 4 byte 对齐
		if ((tmp == prg->uMaxPesLen) || ((tmp & 3) == 0))
		{                                                              
			pes_header_stuff_len = 2;
		}
		else
		{
			pes_header_stuff_len = 2 + (4 - (tmp & 3));
		}
        pes_header_ext_len   +=  pes_header_stuff_len;
        pes->uPayloadLen     =   pes->uRestDataLen;

		if (prc->uIsUnitEnd)
		{
            stuff_flag &= 0xfe;
		}

		if (prc->uIsLastUnit && prc->uIsUnitEnd)
		{
			stuff_flag &= 0xfb;
		}
    }

    // 确保payload长度非负
    if (pes->uPayloadLen > pes->uRestDataLen)
    {
        pes->uPayloadLen = pes->uRestDataLen;
    }
    
    // 确保总长度不超过最大PES长度
    unsigned int total_len = pes_header_basic_len + pes_header_ext_len + pes->uPayloadLen;
    if (total_len > prg->uMaxPesLen && prg->uMaxPesLen > 0)
    {
        // 重新计算payload长度
        pes->uPayloadLen = prg->uMaxPesLen - pes_header_basic_len - pes_header_ext_len;
        if (pes->uPayloadLen > pes->uRestDataLen)
        {
            pes->uPayloadLen = pes->uRestDataLen;
        }
        if (pes->uPayloadLen > MAX_PES_PACKET_LEN)
        {
            pes->uPayloadLen = MAX_PES_PACKET_LEN;
        }
    }

    pes_header_len_syt = pes_header_basic_len + pes_header_ext_len + pes->uPayloadLen - 6;

    // 确保pes_header_len_syt不会溢出（16位）
    if (pes_header_len_syt > 0xFFFF)
    {
        pes_header_len_syt = 0xFFFF;
    }

    // 生成 pes 头数据
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x01;
    buffer[3] = (unsigned char)(pes->uStreamid);
    
    buffer[4] = (unsigned char)((pes_header_len_syt >> 8) & 0xff);
    buffer[5] = (unsigned char)(pes_header_len_syt & 0xff);
    
	buffer[6] =   (unsigned char)((1<<7)//  2 - check bits '10'
                | (( 0x03) << 4)//  2 - PES_scrambling_control(0)
                | ((pes_priority & 1) << 3)//  1 - PES_priority(0)
                | (((~stuff_flag) << 1) & 0x04)     //  1 - data_alignment_indicator(0)	
			    | 0                                 //  1 - copyright(0)
				| 0);                                //  1 - original_or_copy(0)
    
//	buffer[7] = (((unsigned char)pes->add_pts) << 7) | ((unsigned char)pes->add_user_data);
	buffer[7] = ((unsigned char)pes->uAddpts) << 7;
                    //  2 - PTS_DTS_flags()
					//  1 - ESCR_flag(0)
					//  1 - ES_rate_flag(0)
					//  1 - DSM_trick_mode_flag(0)
					//  1 - additional_copy_info_flag(0)
					//  1 - PES_CRC_flag(0)
					//  1 - PES_extension_flag()
	
    buffer[8] = (unsigned char)pes_header_ext_len;	//  8 - PES_header_data_length				

    pos = 9;
    if (pes->uAddpts)
    {
        pts = prc->uTimeStamp;
        buffer[pos++] = (pts >> 28 & 0x0e) | 0x21;	    //  4 - '0010'
		    										//  3 - PTS [32..30]
												    //  1 - marker_bit
	    buffer[pos++] = (pts >> 21);					//  8 - PTS [29..22]
	    buffer[pos++] = (pts >> 13 & 0xfe) | 0x01;	    //  7 - PTS [21..15]
												    //  1 - marker_bit
	    buffer[pos++] = (pts >>  6);					//  8 - PTS [14..7]
	    buffer[pos++] = (pts << 2  & 0xfc) | 0x01;	    //  7 - PTS [6..0]
		                							//  1 - marker_bit
    }
//    if (pes->add_user_data)
//    {
//        buffer[pos++] = 0x8e;	//PES_private_data_flag(1)
//			                    //pack_header_field_flag(0)
//			                    //program_packet_sequence_counter_flag(0)
//			                    //P STD_buffer_flag(0)
//		                        //reserved(111)
//		                        //PES_extension_flag_2(0)
//        for (i = 0; i < 16; i++)
//        {
//            buffer[pos++] = prc->userdata_buf[i];
//        }
//    }
	if (prg->uSetFrameEndFlg)
	{
		for (i = 0; i < pes_header_stuff_len - 1; i++)
		{
			buffer[pos++] = STUFF_BYTE_0XFF;
		}
        buffer[pos++] = (unsigned char)stuff_flag;
	}
	else
	{
		for (i = 0; i < pes_header_stuff_len; i++)
		{
			buffer[pos++] = STUFF_BYTE_0XFF;
		}
	}
    return pos;
}

/******************************************************************************
* 功  能：根据 byte_rate 和 scr 信息在 buffer 里生成 PS 头数据
* 参  数：  buffer      - PS 头缓冲区
*           byte_rate   - PS流码率
*           scr         - 系统参考时钟
*			frame_num	- 帧号
* 返回值：PS 头数据的长度
******************************************************************************/
int MediaPs_fill_PSH(unsigned char *buffer, MEDIAPS_COMPLEX_PROCESS_PARAM_T *prc, MEDIAPS_PS_INFO_T* prg)
{
	unsigned int mux_rate     = 0;                
	//unsigned int stuffing_len = 0; //del warning
	unsigned int stream_num   = 4;
	int	         pos          = 0;
	//int          i            = 0; //del warning
    
	mux_rate = prg->uMaxByteRate / 50 + 1;
	
	// 限制mux_rate在合理范围内（22位）
	if (mux_rate > 0x3FFFFF)
	{
		mux_rate = 0x3FFFFF;
	}
	
	buffer[pos++]	= 0x00;
	buffer[pos++]	= 0x00;
	buffer[pos++]	= 0x01;
	buffer[pos++]	= PROGRAM_STREAM_PSH_TAG;		    //32bits	0x 00 00 01 BA
	
	buffer[pos++]	= 0x40					            //2bits	MPEG2标志 "01"
				    | ((prc->uSysClkRef>>26) & 0x38) 	//3bits	system_clock_reference_base [32..30] 
				    | 0x04					            //1bits	marker_bit				 	
				    | ((prc->uSysClkRef>>28) & 0x03);	//2bits	system_clock_reference_base [29..28]
	buffer[pos++]	= (prc->uSysClkRef>>20) & 0xff;    //8bits	system_clock_reference_base [27..20]
	buffer[pos++]	= ((prc->uSysClkRef>>12) & 0xf8)	//5bits	system_clock_reference_base [19..15]
				    | 0x04					            //1bits	marker_bit
				    | ((prc->uSysClkRef>>13) & 0x03);	//2bits	system_clock_reference_base [14..13]
	buffer[pos++]	= (prc->uSysClkRef>>5) & 0xff;		//8bits	system_clock_reference_base [12..5]
	buffer[pos++]	= ((prc->uSysClkRef<<3) & 0xf8)	//5bits	system_clock_reference_base [4..0]
				    | 0x04					//1bits	marker_bit
				    | 0x00;					//2bits	system_clock_reference_ext [8..7]
	buffer[pos++]	= 0x01;					//7bits	system_clock_reference_ext [6..0]
										    //1bits	marker_bit
	buffer[pos++]	= (mux_rate>>14) & 0xff;//8bits	mux_rate [21..14]
	buffer[pos++]	= (mux_rate>>6) & 0xff;	//8bits	mux_rate [13..6]
	buffer[pos++]	= (mux_rate<<2)			//6bits	mux_rate [5..0]
				    | 0x03;					//2bits	marker_bit
    
	buffer[pos++]	= 0xFE;					//5bits	reserved, 3bits stuffing_len = 6;				
	buffer[pos++]	= STUFF_BYTE_0XFF;		//stuffing_byte				
	buffer[pos++]	= STUFF_BYTE_0XFF;		//stuffing_byte	
	
	if (prg->uSetFrameEndFlg)
	{
		buffer[pos++]	= (prc->uFrameNum >> 24) & 0xff;
		buffer[pos++]	= (prc->uFrameNum >> 16) & 0xff;
		buffer[pos++]	= (prc->uFrameNum >> 8) & 0xff;
		buffer[pos++]	= prc->uFrameNum & 0xff;
	}
	else
	{
		buffer[pos++]	= STUFF_BYTE_0XFF;
		buffer[pos++]	= STUFF_BYTE_0XFF;
		buffer[pos++]	= STUFF_BYTE_0XFF;
		buffer[pos++]	= STUFF_BYTE_0XFF;
	}

	if (prc->uIsKeyFrame)
	{
		//system header

		buffer[pos++] = 0x00;
		buffer[pos++] = 0x00;
		buffer[pos++] = 0x01;
		buffer[pos++] = 0xbb;						        //32bits 0x000001BB

		buffer[pos++] = (6 + 3 * stream_num) >> 8;	        //8bits	header_length_high_8_bits
		buffer[pos++] = (6 + 3 * stream_num) & 0xff;        //8bits	header_length_low_8_bits

		buffer[pos++] = 0x80						        //1bits marker_bit
			          |((mux_rate >> 15) & 0x7f);             //7bits	rate_bound [21..15]
		buffer[pos++] = (mux_rate >> 7) & 0xff;	            //8bits	rate_bound [14..7]
		buffer[pos++] = ((mux_rate << 1) & 0xfe)	            //7bits	rate_bound [6..0]
			          |0x01;						        //1bits marker_bit

		buffer[pos++] = 0x04						        //6bits	audio_bound
			          |0x00						            //1bits	fixed_flag
			          |0x00;						        //1bits	csps_flag
		buffer[pos++] = 0x80						        //1bits	sys_audio_lock_flag
			          |0x40						            //1bits	sys_video_lock_flag
			          |0x20						            //1bits	marker_bit
			          |0x01;						        //5bits	video_bound
		buffer[pos++] = 0x00						        //1bits packet_rate_restriction_flag
			          |0x7f;						        //7bits	reserved

		buffer[pos++] = 0xe0;                               //stream_id	
		buffer[pos++] = (0xc0                               //2 bits "11"
			          | 0x20);                              //1 bit P-STD_buffer_bound_scale, video must '1'
		buffer[pos++] = 0x80;                               //13 bits P-STD_buffer_size_bound, if bound_scale is '0', unit is 128 bytes, or unit is 1024 bytes

		buffer[pos++] = 0xc0;                               //stream_id
		buffer[pos++] = (0xc0                               //2 bits "11"
			          | 0x00);                              //1 bit P-STD_buffer_bound_scale, audio must '0'
		buffer[pos++] = 0x08;                               //13 bits P-STD_buffer_size_bound, if bound_scale is '0', unit is 128 bytes, or unit is 1024 bytes

		buffer[pos++] = 0xbd;                               //stream_id	
		buffer[pos++] = (0xc0                               //2 bits "11"
			          | 0x20);                              //1 bit P-STD_buffer_bound_scale, video must '1'
		buffer[pos++] = 0x80;                               //13 bits P-STD_buffer_size_bound, if bound_scale is '0', unit is 128 bytes, or unit is 1024 bytes

		buffer[pos++] = 0xbf;                               //stream_id	
		buffer[pos++] = (0xc0                               //2 bits "11"
			          | 0x20);                              //1 bit P-STD_buffer_bound_scale, video must '1'
		buffer[pos++] = 0x80;                               //13 bits P-STD_buffer_size_bound, if bound_scale is '0', unit is 128 bytes, or unit is 1024 bytes

	}

	return pos;
}

/******************************************************************************
* 功  能：根据 PROGRAM_INFO *prg 信息在 buffer 里生成 PSM 数据
* 参  数：  buffer      - PSM 缓冲区
*           prg         - 节目流参考信息
*           prc         - 处理参数，主要用到全局时间
* 返回值：PSM 数据的长度
******************************************************************************/
int MediaPs_fill_PSM(unsigned char * buffer,  MEDIAPS_PS_INFO_T *prg, MEDIAPS_COMPLEX_PROCESS_PARAM_T *prc)
{
	int pos, tmp_pos, tmp_len = 0;
	unsigned int crc;  // 修改为unsigned int以匹配MediaPs_mpeg2_crc返回类型
	int len_pos;

    buffer[0] = 0x00;
	buffer[1] = 0x00;
	buffer[2] = 0x01;
	buffer[3] = PROGRAM_STREAM_PSM_TAG;

    //暂时填充pes包头长度，后面修改1
	buffer[4] = STUFF_BYTE_0XFF;
	buffer[5] = STUFF_BYTE_0XFF;

	//buffer[6] = 0xe0 | (unsigned char)((prg->PSM_version++) & 0x1f);
	buffer[6] = 0xe0 | 0x1f;
	buffer[7] = STUFF_BYTE_0XFF;

	if (prg->uDscptSets & INCLUDE_BASIC_DESCRIPTOR)
	{
		tmp_len += BASIC_DESCRIPTOR_LEN;
	}
	if (prg->uDscptSets & INCLUDE_DEVICE_DESCRIPTOR)
	{
		tmp_len += DEVICE_DESCRIPTOR_LEN;
	}

    
	buffer[8]   = (unsigned char)(tmp_len >> 8);
	buffer[9]   = (unsigned char)(tmp_len);
    pos         =   10;

	if (prg->uDscptSets & INCLUDE_BASIC_DESCRIPTOR)
	{
		pos +=  MediaRefer_fill_basic_descriptor(&buffer[pos], &prc->stGlobalTime, 0, prc->uCompanyMark, prc->uCameraMark);
	}

	if (prg->uDscptSets & INCLUDE_DEVICE_DESCRIPTOR)
	{
		memcpy(&buffer[pos], prg->strDeviceDsc, DEVICE_DESCRIPTOR_LEN);
		pos +=  DEVICE_DESCRIPTOR_LEN;
	}
    //暂时填充pes包头长度，后面修改2
    tmp_pos     = pos;
	buffer[pos++] = STUFF_BYTE_0XFF;
	buffer[pos++] = STUFF_BYTE_0XFF;

	if (prg->uStreamMode & INCLUDE_VIDEO_STREAM)
	{
		buffer[pos++] = prg->uVideoStreamType;
		buffer[pos++] = prg->uVideoStreamid;
		len_pos = pos;
		tmp_len = 0;
		pos += 2;

		if (prg->uDscptSets & INCLUDE_VIDEO_DESCRIPTOR)
		{
			tmp_len += VIDEO_DESCRIPTOR_LEN;
			memcpy(&buffer[pos], prg->strVideoDsc, VIDEO_DESCRIPTOR_LEN);
			pos +=  VIDEO_DESCRIPTOR_LEN;
		}			
		if (prg->uVideoClip || (prg->uDscptSets & INCLUDE_VIDEO_CLIP_DESCRIPTOR))
		{
			tmp_len += VIDEO_CLIP_DESCRIPTOR_LEN;
			memcpy(&buffer[pos], prg->strVideoClipDsc, VIDEO_CLIP_DESCRIPTOR_LEN);
			pos         +=  VIDEO_CLIP_DESCRIPTOR_LEN;
		}
		if (prg->uDscptSets & INCLUDE_TIMING_HRD_DESCRIPTOR)
		{
			tmp_len += TIMING_HRD_DESCRIPTOR_LEN;
			memcpy(&buffer[pos], prg->strTimingHrdDsc, TIMING_HRD_DESCRIPTOR_LEN);
			pos += TIMING_HRD_DESCRIPTOR_LEN;
		}
		buffer[len_pos++] = tmp_len >> 8;
		buffer[len_pos]   = tmp_len;
	}
	if (prg->uStreamMode & INCLUDE_AUDIO_STREAM)
	{
		buffer[pos++] = prg->uAudioStreamType;
		buffer[pos++] = prg->uAudioStreamid;
		if (prg->uDscptSets & INCLUDE_AUDIO_DESCRIPTOR)
		{
			buffer[pos++] = (unsigned char)(AUDIO_DESCRIPTOR_LEN >> 8);
			buffer[pos++] = (unsigned char)(AUDIO_DESCRIPTOR_LEN);
			memcpy(&buffer[pos], prg->strAudioDsc, AUDIO_DESCRIPTOR_LEN);
			pos         +=  AUDIO_DESCRIPTOR_LEN;
		}
		else
		{
			buffer[pos++] = 0;
			buffer[pos++] = 0;
		}
	}
    if (prg->uStreamMode & INCLUDE_PRIVT_STREAM)
    {
		buffer[pos++] = prg->uPrivtStreamType;
		buffer[pos++] = prg->uPrivtStreamid;
		buffer[pos++] = 0;
		buffer[pos++] = 0;
    }

    //暂时填充修改2
    if (pos - tmp_pos - 2 <= 0xFFFF)
    {
        buffer[tmp_pos]     = (unsigned char)((pos - tmp_pos - 2) >> 8); 
        buffer[tmp_pos+1]   = (unsigned char)(pos - tmp_pos - 2);
    }
    else
    {
        // 长度超出16位范围，截断
        buffer[tmp_pos]     = 0xFF; 
        buffer[tmp_pos+1]   = 0xFF;
    }

    //暂时填充修改1
    int psm_length = pos - 6; // PSM长度不包括前6个字节(0x000001BC + length)
    if (psm_length <= 0xFFFF)
    {
        buffer[4] = (psm_length >> 8) & 0xFF;
        buffer[5] = psm_length & 0xFF;
    }
    else
    {
        // 长度超出16位范围，截断
        buffer[4] = 0xFF;
        buffer[5] = 0xFF;
    }

	crc = MediaPs_mpeg2_crc(buffer, pos);
    
    buffer[pos++] = (unsigned char)(crc >> 24); 
    buffer[pos++] = (unsigned char)(crc >> 16); 
    buffer[pos++] = (unsigned char)(crc >> 8); 
    buffer[pos++] = (unsigned char)(crc); 

    return(pos);
}

int MediaPs_PrivHead_Modify(MEDIAPS_PS_INFO_T *prg, MEDIAPS_PES_PACK_INFO_T *pes)
{
	if (pes->uAddpts && prg->uMaxPesLen)
	{
		prg->PrivteHeader[2] = (((pes->uPayloadLen + 8 - PS_PRIVATE_HEAD_LEN) / 4) >> 8);
		prg->PrivteHeader[3] = ((pes->uPayloadLen + 8 -PS_PRIVATE_HEAD_LEN) / 4);
		prg->PrivteHeader[7] = (pes->uTotalDataLen / prg->uMaxPesLen) + 1;
		prg->PrivteHeader[8] = 0;
	}
	else
	{
		// +8 私有头除type、len字段还有8字节
		prg->PrivteHeader[2] = (((pes->uPayloadLen + 8) / 4) >> 8);
		prg->PrivteHeader[3] = ((pes->uPayloadLen + 8) / 4);
		prg->PrivteHeader[8]++;
		//prg->privte_header[8] = prg->privte_header[8]++;
	}

	return 1;
}

/******************************************************************************
* 功  能：获取所需内存大小
* 参  数：param - 参数结构指针
* 返回值：返回错误码
* 备  注：参数结构中 buffer_size变量用来表示所需内存大小
******************************************************************************/
int MediaPs_GetMemSize(MEDIAPS_COMPLEX_PARAM_T *param)
{
	if (param == NULL)
    {
		return -1;
    }
	param->uBufferSize = sizeof(MEDIAPS_PS_INFO_T);
	return 0;
}


int MediaPs_Create(MEDIAPS_COMPLEX_PARAM_T *param, void **handle)
{
	MEDIAPS_PS_INFO_T *prg = (MEDIAPS_PS_INFO_T *)param->pBuffer;
	if (param == NULL)
    {
		return -1;
    }
	if (param->pBuffer == NULL)
    {
		return -1;
    }

    MediaPs_ResetStreamInfo((void *)prg, &(param->stInfo));

	prg->uVideoStreamid = PES_VIDEO_BASE;
	prg->uAudioStreamid = PES_AUDIO_BASE;
    prg->uPrivtStreamid = PES_PRIVT_BASE;

	*handle	= (void *)prg;
	return 0;
} 

/******************************************************************************
* 功  能：重置参考数据
* 参  数：handle - 句柄(handle由PSMUX_Create返回)
*         info   - 参考数据句柄
* 返回值：返回错误码
******************************************************************************/
int MediaPs_ResetStreamInfo(void *handle, MEDIAPS_ES_INFO_T *info)
{
	MEDIAPS_PS_INFO_T *prg = (MEDIAPS_PS_INFO_T *)handle;
    int frame_rate;

	if (handle == NULL || info == NULL)
    {
		return -1;
    }

    prg->uStreamMode    = info->uStreamMode;
    prg->uVideoClip     = info->stStreamInfo.video_info.play_clip;
	prg->uMaxByteRate  = info->uMaxByteRate;	    /* 码率，以byte为单位*/
    prg->uMaxPesLen    = info->uMaxPacketLen;        /* 最大pes长度*/

    prg->uVideoStreamType     = info->uVideoStreamType;  /* 输入视频流类型 */
    prg->uAudioStreamType     = info->uAudioStreamType;  /* 输入音频流类型 */
	prg->uPrivtStreamType     = info->uPrivtStreamType;	/* 输入私有流类型 */
	prg->uDscptSets            = info->uDscptSets;
	prg->uBframeAudioSetPsh  = info->uBframeAudioSetPsh;
	prg->uSetFrameEndFlg     = info->uSetFrameEndFlg;

	if (prg->uDscptSets & INCLUDE_DEVICE_DESCRIPTOR)
	{
		MediaRefer_fill_device_descriptor(prg->strDeviceDsc, info->stStreamInfo.dev_chan_id);
	}

    if (info->uStreamMode & INCLUDE_VIDEO_STREAM)
    {
        if (prg->uDscptSets & INCLUDE_VIDEO_DESCRIPTOR)
		{
			MediaRefer_fill_video_descriptor(prg->strVideoDsc, &info->stStreamInfo.video_info);
		}
        if (prg->uVideoClip || (prg->uDscptSets & INCLUDE_VIDEO_CLIP_DESCRIPTOR))
        {
           MediaRefer_fill_video_clip_descriptor(prg->strVideoClipDsc, &info->stStreamInfo.video_info);
        }
		if (prg->uDscptSets & INCLUDE_TIMING_HRD_DESCRIPTOR)
		{
			if (info->stStreamInfo.video_info.time_info != 0)
			{
				frame_rate = 90000 / info->stStreamInfo.video_info.time_info;
			}
			else
			{
				frame_rate = 25;
			}
			MediaRefer_fill_timing_hrd_descriptor(prg->strTimingHrdDsc, frame_rate, info->stStreamInfo.video_info.width_orig,
		    info->stStreamInfo.video_info.height_orig);
		}
    }
    if (info->uStreamMode & INCLUDE_AUDIO_STREAM)
    {
        if (prg->uDscptSets & INCLUDE_AUDIO_DESCRIPTOR)
		{
			MediaRefer_fill_audio_descriptor(prg->strAudioDsc, &info->stStreamInfo.audio_info);
		}

    }

	//最大pes包长度不能超过65496，且要为4的整数倍
    if (info->uMaxPacketLen > MAX_PES_PACKET_LEN)
    {
        info->uMaxPacketLen = MAX_PES_PACKET_LEN;
    }
    prg->uMaxPesLen = info->uMaxPacketLen - (info->uMaxPacketLen & 3) - PS_PRIVATE_HEAD_LEN;

    return 0;
}

/******************************************************************************
* 功  能：复合一段数据块
* 参  数：handle - 句柄(handle由PSMUX_Create返回)
*         param  - 处理单元参数
* 返回值：返回错误码
******************************************************************************/
int MediaPs_Process(void *handle, MEDIAPS_COMPLEX_PROCESS_PARAM_T *param)
{
	MEDIAPS_PS_INFO_T *prg = (MEDIAPS_PS_INFO_T *)handle;
    MEDIAPS_PES_PACK_INFO_T	*pes = NULL;
    unsigned int pos = 0;
    unsigned int    packed_len = 0;
    unsigned char * buffer = param->pOutBuf;
	unsigned int mod_pos = 0;
	unsigned int src_len = 0;
	unsigned int pes_mod_len = 0;

	if (param == NULL || prg == NULL)
    {
		return MEDIAPS_ERR_PARA_NULL;
    }
    
    // 添加输入缓冲区边界检查
    if (param->pUnitInBuf == NULL || param->uUnitInLen == 0)
    {
        return MEDIAPS_ERR_PARA_NULL;
    }
    
    pes = &prg->stPesCur;

    //根据输入处理 stream_id
	switch(param->uFrameType)
	{
    case FRAME_TYPE_VIDEO_IFRAME:
    case FRAME_TYPE_VIDEO_EFRAME:
    case FRAME_TYPE_VIDEO_PFRAME:
    case FRAME_TYPE_VIDEO_BFRAME:
		pes->uStreamid = prg->uVideoStreamid;
        break;
    case FRAME_TYPE_AUDIO_FRAME:
		pes->uStreamid = prg->uAudioStreamid;
        break;
    case FRAME_TYPE_PRIVT_FRAME:
        pes->uStreamid = prg->uPrivtStreamid;
        // 添加边界检查，确保输入缓冲区足够大
        if (param->uUnitInLen >= PS_PRIVATE_HEAD_LEN)
        {
            memcpy(prg->PrivteHeader, param->pUnitInBuf, PS_PRIVATE_HEAD_LEN);
        }
        else
        {
            // 如果输入缓冲区太小，用默认值填充
            memset(prg->PrivteHeader, 0, PS_PRIVATE_HEAD_LEN);
        }
        break;
    default:
        return MEDIAPS_ERR_STREAM_TYPE;
	}

    if (param->uIsFirstUnit && param->uIsUnitStart)
    {
        //当输入为视频P帧或者关键帧时，添加PSH
        if (param->uIsKeyFrame || (pes->uStreamid == prg->uVideoStreamid && param->uFrameType != FRAME_TYPE_VIDEO_BFRAME))
        {
	        pos += MediaPs_fill_PSH(buffer, param, prg);
	        if (pos >= param->uOutBufSize)
            {
		        return MEDIAPS_ERR_MEM_OVER;
		    }
        }

		if (prg->uBframeAudioSetPsh && param->uFrameType != FRAME_TYPE_VIDEO_IFRAME && param->uFrameType != FRAME_TYPE_VIDEO_PFRAME && param->uIsFirstUnit && param->uIsUnitStart)
		{
			pos += MediaPs_fill_PSH(buffer, param, prg);
			if (pos >= param->uOutBufSize)
			{
				return MEDIAPS_ERR_MEM_OVER;
			}
		}
		    
        //当输入为关键帧时，添加PSM
	    if (param->uIsKeyFrame)
	    {
		    pos += MediaPs_fill_PSM(&buffer[pos], prg, param);
	        if (pos >= param->uOutBufSize)
            {
		        return MEDIAPS_ERR_MEM_OVER;
            }
        }
    }

    pes->uAddpts         = (param->uIsFirstUnit && param->uIsUnitStart);
    //第一个 nalu 添加userdata，其余不加
/*    pes->add_user_data   = (param->add_userdata && param->is_first_unit); */
	pes->uRestDataLen	 = param->uUnitInLen;
    pes->uTotalDataLen	 = param->uUnitInLen;
    pes->uPayloadLen     = 0;

    //将输入数据打包
    do
    {
		if (FRAME_TYPE_PRIVT_FRAME == param->uFrameType
			&& param->uUnitInLen >= 2
			&& 0x01 == param->pUnitInBuf[0]
		    && 0x01 == param->pUnitInBuf[1]) // 只对温度信息有效
		{
			mod_pos = pos;
			pos += MediaPs_create_pes_header(&buffer[pos], pes, param, prg);
			
			// 检查输出缓冲区是否足够
			if (pos > param->uOutBufSize)
			{
				return MEDIAPS_ERR_MEM_OVER;
			}

			pes_mod_len = pes->uPayloadLen & 3;
			if (pes_mod_len)
			{
				pes->uPayloadLen -= pes_mod_len;
			}

			MediaPs_PrivHead_Modify(prg, pes);

			if (!pes->uAddpts)
			{
				// 检查是否有足够的空间添加私有头
				if (pos + PS_PRIVATE_HEAD_LEN > param->uOutBufSize)
				{
					return MEDIAPS_ERR_MEM_OVER;
				}
				
				memcpy(&buffer[pos], prg->PrivteHeader, PS_PRIVATE_HEAD_LEN);
				pos += PS_PRIVATE_HEAD_LEN;
				
				// 更新PES包长度，确保不超过输出缓冲区
				src_len = (buffer[mod_pos + 4] << 8) + buffer[mod_pos + 5];
				unsigned int new_len = src_len + PS_PRIVATE_HEAD_LEN - pes_mod_len;
				if (mod_pos + 6 <= param->uOutBufSize)
				{
					buffer[mod_pos + 4] = (unsigned char)((new_len >> 8) & 0xff);
					buffer[mod_pos + 5] = (unsigned char)(new_len & 0xff);
				}
			}
			else
			{
				// 覆盖原先的12字节头，需要确保输入缓冲区足够大
				if (param->uUnitInLen >= PS_PRIVATE_HEAD_LEN)
				{
					memcpy(param->pUnitInBuf, prg->PrivteHeader, PS_PRIVATE_HEAD_LEN);
				}
				else
				{
					// 输入缓冲区太小，无法安全覆盖
					return MEDIAPS_ERR_MEM_OVER;
				}
			}
		}
		else
		{
			pos += MediaPs_create_pes_header(&buffer[pos], pes, param, prg);
		}

		if (pos + pes->uPayloadLen > param->uOutBufSize)
        {
			return MEDIAPS_ERR_MEM_OVER;
        }

        // 确保不会超出输入数据长度
        unsigned int copy_len = (packed_len + pes->uPayloadLen <= pes->uTotalDataLen) ? 
                               pes->uPayloadLen : (pes->uTotalDataLen - packed_len);
                               
        if (copy_len > 0)
        {
            memcpy(&buffer[pos], &param->pUnitInBuf[packed_len], copy_len);
            packed_len  += copy_len;
            pos         += copy_len;
        }

        pes->uAddpts         = 0;//除了第一个pes包外，其他包不添加pts和userdata
/*        pes->add_user_data   = 0;*/
	    pes->uRestDataLen	= pes->uTotalDataLen - packed_len;

    } while(packed_len < pes->uTotalDataLen);

    param->uOutBufLen = pos;
	return OK;
}


