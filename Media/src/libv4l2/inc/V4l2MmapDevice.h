/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2MmapDevice.h
** 
** V4L2 source using mmap API
**
** -------------------------------------------------------------------------*/


#pragma once
#include "V4l2Device.h"
#ifdef MEDIARKMPP
#include "rockchip/rk_mpi.h" 
#endif

#define V4L2MMAP_NBBUFFER 3
#define FMT_NUM_PLANES 1

class V4l2MmapDevice : public V4l2Device
{	
	protected:	
		size_t writeInternal(char* buffer, size_t bufferSize);
		bool   startPartialWrite();
		size_t writePartialInternal(char*, size_t);
		bool   endPartialWrite();
		size_t readInternal(char* buffer, size_t bufferSize);
#ifdef MEDIARKMPP
		MppBuffer readFromQueue(unsigned int *index);
		bool      putFrameQueue(int index);
#endif		
	public:
		V4l2MmapDevice(const V4L2DeviceParameters & params, v4l2_buf_type deviceType);		
		virtual ~V4l2MmapDevice();

		virtual bool init(unsigned int mandatoryCapabilities);
		virtual bool isReady() { return  ((m_fd != -1)&& (n_buffers != 0)); }
		virtual bool start();
		virtual bool stop();
	
	protected:
		unsigned int  n_buffers;
	
		struct buffer 
		{
			void *                  start;
			size_t                  length;
			int                     export_fd;
		};
		buffer m_buffer[V4L2MMAP_NBBUFFER];
};


