#include "Record.h"
#include "Media.h"
#include "MediaFrame.h"
#include "Thread.h"
#include "MediaManager.h"
#include "Logger.h"
#include "mp4v2/mp4v2.h"
/*
//   年月日     时分秒 自动计数值
文件格式:ch0-20260114-165306-0000001.mp4
方式一 直接获取处理 
方式二 解PS流处理
*/

#include <iostream>

int RecordH264Mp4(const CHAR* strFilePath,PUINT8 pData,UINT32 u32Length)
{
 
    // 1. 创建 MP4 文件
    MP4FileHandle mp4File = MP4Create(strFilePath);
    if (mp4File == MP4_INVALID_FILE_HANDLE) {
        LOG_ERROR("Failed to create MP4 file.");
        return ERROR;
    }

    // 2. 设置时间尺度（单位：毫秒）
    MP4SetTimeScale(mp4File, 90000);

    // 3. 创建视频轨道（假设为 H.264 码流）
    const char* videoType = "avc1"; // H.264 类型
    MP4TrackId videoTrack = MP4AddH264VideoTrack(
        mp4File,
        90000,  // 时间尺度
        0,      // 默认样本时长（0 表示由写入数据决定）
        1920,   // 视频宽度
        1080,   // 视频高度
        0, // AVCC 配置数据（需根据实际编码器设置）
        0,       // AVCC 数据长度
        0,
        0
    );

    if (videoTrack == MP4_INVALID_TRACK_ID) {
        LOG_ERROR("Failed to create video track.");
        MP4Close(mp4File);
        return ERROR;
    }

    // 4. 写入样本数据（示例：假设已有编码好的 H.264 数据）
    MP4Duration sampleDuration = 3000; // 样本时长（根据帧率计算，如 30fps 则约为 3000/90000 秒）
    MP4WriteSample(mp4File, videoTrack, pData, u32Length, sampleDuration);

    // 5. 关闭文件
    MP4Close(mp4File);
    LOG_INFO("MP4 file created successfully: %s\n",strFilePath);
    return 0;
}


/*
static MUTEX_ID mtxCond1;
ret = sys_mutex_create(&mtxCond1, 0);
sys_pthread_create_leagcy(
                        NULL, 
                        "cond_task1",
                        TASK_PRIORITY_8,                                                
                        SIZE_64KB,
                        cond_test_task1,
                        (void*)&uCond1)

VOID app_backupd(VOID* pArg)
{
    return;
}
sys_cond_wait(pCond, &mtxCond1, 2000);

sys_pthread_create(NULL, "app_backupd", TASK_PRIORITY_0, 128*1024, (FUNCPTR) app_backupd, 0);
ret = sys_mutex_lock(&handle, 1000);
 if (ret == -ETIMEDOUT) {
	 PR_INF("%s%p lock mutex timeout, try again ...\n", __FUNCTION__, arg);  
	 continue;
 } else if (ret < 0) {
	 PR_ERR ("Failed to lock mutex, ret = %d\n", ret);
	 mutextest++;
	 return ;
 } else {
	 PR_INF("%s%p lock mutex ok.\n", __FUNCTION__, arg);
 }

 PR_INF("%s%p doing ...\n", __FUNCTION__, arg);

 sys_pthread_sleep(5000);

 PR_INF("%s%p unlock mutex ...\n", __FUNCTION__, arg);

 ret = sys_mutex_unlock(&handle);


static void Tsk_SaveRecordFile(UINT arg)
{
    ES_STREAM_ELEMENT ele;
    UINT len1,len2,curLen=0;
    UINT32 * pData = NULL;
    int r = 0, w = 0;
    REC_POOL_INFO *pPool = NULL;
    int bHaveData = 0;
    int chan;
    
    while (1)
    {
        OsalSem_pend(recordFlag, 0, OSAL_WAIT_FOREVER);
        
        if (0 == bHaveData)
        {
            OsalTask_sleepMs(10);
        }
		
        bHaveData = 0;
		for (chan = 0; chan < MAX_ENC_CHAN + MAX_USB_CHAN_PER_SOC; chan++)
        {
            pPool=g_pDspInitPara->RecPool+chan;

            r=pPool->rIdx;
            w=pPool->wIdx;
            
            //get data length in share memory  
            if (w >= r)
            {
                len1 = w - r;
                len2 = 0;
            }
            else
            {
                len1 = pPool->totalLen - r;
                len2 = w;
            }
            curLen = len1+len2;
            if(curLen < sizeof(ES_STREAM_ELEMENT))
            {
                continue;
            }

            if(curLen >= sizeof(ES_STREAM_ELEMENT))
            {          
                if((void*)pPool->addr[0] == NULL)
                {
                    TEPRT("[chan%d] invalid addr[0] !\n",chan);
                    continue;
                }
                
                pData = (UINT32 *)((PUINT8)pPool->addr[0] + r);

                // 验证magic  
				if(len1 >= sizeof(ES_STREAM_ELEMENT))
				{				    
                    memcpy(&ele,pData, sizeof(ES_STREAM_ELEMENT));
                    ele.dataLen  = (ele.dataLen  + 3) & (~3); //4字节对齐
					if (ele.magic != STREAM_ELEMENT_MAGIC)
					{
						TEPRT("<mux1>  2 chan%d stream ele.magic err:0x%x w:%d r:%d\n", chan, ele.magic, w, r);
						continue;
					}
				}
				else
				{
					memcpy(&ele,pData, len1);
					memcpy((PUINT8)(&ele)+len1,(PUINT8)pPool->addr[0], sizeof(ES_STREAM_ELEMENT)-len1);

					if (ele.magic != STREAM_ELEMENT_MAGIC)
					{
						TEPRT("<mux2> chan%d stream ele.magic err:0x%x w:%d r:%d\n", chan, ele.magic, w, r);
						continue;
					}
				}
                
				// 由于索引数据包含前后两部分，所以将前后两部分整合到一个包送入到录像池
				//原始索引数据结构如下:前半部分信息+填充信息+后半部分信息(mdat信息)
				//说明:
				//	 前半部分信息:该部分包含索引的有效数据，随着帧数越多，该长度越大
				//	 填充信息:预留索引时的填充区域，固定为00,随着帧数越多，该长度越小
				//	 后半部分信息:该部分包含mdat的长度等信息，在数据的前面，在整个索引区域的最后面，一般为8-16个字节
				//	 整个索引空间大小不变，总大小为创建MP4打包句柄时指定的大小
				//重新打包后的结构:
				//	 前半部分信息
				//	 后半部分信息
				
                 //fix by chenzongyi coverity 3418499
                if((ele.type == STREAM_ELEMENT_VIDEO_I) || (ele.type == STREAM_ELEMENT_VIDEO_P))
                {
                   if (NULL == recFile[chan])
                    {
                       frameCnt[chan] = 0;
                       //sprintf(fileName,"/opt/DCIM/stm_%d_%d.mp4",chan,fileCnt[chan]);
					   sprintf(fileName,"/mnt/mmc01/stm_%d_%d.mp4",chan,fileCnt[chan]);
                       recFile[chan] = fopen(fileName, "wb");
                       fwrite(&hikMedia,sizeof(HIK_MEDIAINFO),1,recFile[chan]); //lx_test
                       isNewFile[chan] = 1;
                       fileCnt[chan]++;
                    }
       
                    if(frameCnt[chan]++ >= 1000)
                    {
                        if(ele.type == STREAM_ELEMENT_VIDEO_I)
                        {
                            if (NULL == recFile[chan])
                            {
                                continue;
                            }
                        
                            frameCnt[chan] = 0;
                            fclose(recFile[chan]);
                            //sprintf(fileName,"/opt/DCIM/stm_%d_%d.mp4",chan,fileCnt[chan]);
							sprintf(fileName,"/mnt/mmc01/stm_%d_%d.mp4",chan,fileCnt[chan]);
                            recFile[chan] = fopen(fileName, "wb");
                            fwrite(&hikMedia,sizeof(HIK_MEDIAINFO),1,recFile[chan]); //lx_test
                            TEPRT("[Demo] [chan%d]Create New File : %s\n",chan,fileName);                        
                            isNewFile[chan] = 1;
                            fileCnt[chan]++;
                        }
                    }
                }

                ele.dataLen  = (ele.dataLen  + 3) & (~3);

                if((isNewFile[chan]) && (ele.type != STREAM_ELEMENT_VIDEO_I))
                {
                    pPool->rIdx = (pPool->rIdx + sizeof(ES_STREAM_ELEMENT)+ele.dataLen) % pPool->totalLen;
                    continue;
                }
                
                if(len1 >= sizeof(ES_STREAM_ELEMENT))
                {
                	//if(ele.type == STREAM_ELEMENT_AUDIO_F)
            		//{
            			if (len1>=(ele.dataLen+sizeof(ES_STREAM_ELEMENT)))
	                    {
	                        if (recFile[chan])
	                        {
	                        	//if(ele.type == STREAM_ELEMENT_AUDIO_F)
								fwrite((PUINT8)pData+sizeof(ES_STREAM_ELEMENT),ele.dataLen,1,recFile[chan]);
	                        }
	                    }
	                    else
	                    {							
	                        if (recFile[chan])
	                        {
	                        	//if(ele.type == STREAM_ELEMENT_AUDIO_F)
                        		//{
                        			fwrite((PUINT8)pData+sizeof(ES_STREAM_ELEMENT),len1-sizeof(ES_STREAM_ELEMENT),1,recFile[chan]);
	                            	fwrite((PUINT8)pPool->addr[0],ele.dataLen-(len1-sizeof(ES_STREAM_ELEMENT)),1,recFile[chan]);
                        		//}
	                            
	                        }
	                    }
	                    RecordWriteLen[chan]+=ele.dataLen;
	                    bHaveData |= 1<<chan;
	            //	}
                    
                }
                else
                {
	            		if (recFile[chan])
	                    {
	                    	//if(ele.type == STREAM_ELEMENT_AUDIO_F)
							fwrite((PUINT8)pPool->addr[0]+sizeof(ES_STREAM_ELEMENT)-len1,ele.dataLen,1,recFile[chan]);
	                    }
	                    RecordWriteLen[chan]+=ele.dataLen;
                    
                }
                pPool->rIdx = (pPool->rIdx + sizeof(ES_STREAM_ELEMENT)+ele.dataLen) % pPool->totalLen;

                if(ele.type == STREAM_ELEMENT_VIDEO_I)
                {
                     //PRT("[DEMO] [chan%d] Rev I Frm  time=%d:%d!\n",chan,ele.absTime.minute,ele.absTime.second);
                }
                else if(ele.type == STREAM_ELEMENT_AUDIO_F)
                {
                   // TEPRT("[DEMO] [chan%d] Rev A Frm !\n",chan);
                }

              // TEPRT("[DEMO] RecPool[%d] w=%d r=%d len=%d\n",chan,pPool->wIdx,pPool->rIdx,ele.dataLen);
                
            }
            
			if (recFile[chan])
			{		
                fflush(recFile[chan]);
                fsync(fileno(recFile[chan]));
			}
			
			isNewFile[chan] = 0;
        }
		
		OsalSem_post(recordFlag, 0);
		//OsalTask_sleepMs(2);
    }
}
*/
