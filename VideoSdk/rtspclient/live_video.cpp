/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2014-2019, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#include "sys_inc.h"
#include "live_video.h"
#include "media_format.h"
#include "lock.h"

#include <iostream>

/**************************************************************************************/
CLiveVideo * CLiveVideo::m_pInstance[] = {NULL, NULL, NULL, NULL};
void * CLiveVideo::m_pInstMutex = sys_os_create_mutex();

/**************************************************************************************/
void * liveVideoThread(void * argv)
{
	CLiveVideo *capture = (CLiveVideo *)argv;
	capture->captureThread();

	return NULL;
}

/**************************************************************************************/
CLiveVideo::CLiveVideo()
{
	m_nDevIndex = 0;	
	m_nWidth = 0;
	m_nHeight = 0;
	m_nFramerate = 15;
	m_nBitrate = 0;

	m_pMutex = sys_os_create_mutex();
	
	m_bInited = false;
	m_bCapture = false;
	m_hCapture = 0;
	
	m_nRefCnt = 0;
	m_bIKey = false;

	m_nIndex = -1;

	m_nKey = 0;

	m_nSps = 0;

	m_pCallbackMutex = sys_os_create_mutex();
    //m_pCallbackList = h_list_create(false);
	m_bufKey = nullptr;
	m_lsObj = new ObjectLists<RTP_BUFFER>;
	//m_rtsp = NULL;
}

CLiveVideo::~CLiveVideo()
{
	stopCapture();
	
	sys_os_destroy_sig_mutex(m_pMutex);

	//h_list_free_container(m_pCallbackList);
	
	sys_os_destroy_sig_mutex(m_pCallbackMutex);
	if (m_bufKey)
		free(m_bufKey);

	delete m_lsObj;
}

CLiveVideo * CLiveVideo::getInstance(int idx)
{
	if (idx < 0 || idx >= MAX_STREAM_NUMS)
	{
		return NULL;
	}
	
	if (NULL == m_pInstance[idx])
	{
		sys_os_mutex_enter(m_pInstMutex);

		if (NULL == m_pInstance[idx])
		{
			m_pInstance[idx] = (CLiveVideo *) new CLiveVideo;
			if (m_pInstance[idx])
			{
				m_pInstance[idx]->m_nRefCnt++;
				m_pInstance[idx]->m_nDevIndex = idx;
			}
		}
		
		sys_os_mutex_leave(m_pInstMutex);
	}
	else
	{
		sys_os_mutex_enter(m_pInstMutex);
		m_pInstance[idx]->m_nRefCnt++;
		sys_os_mutex_leave(m_pInstMutex);
	}

	return m_pInstance[idx];
}

void CLiveVideo::freeInstance(int idx)
{
	if (idx < 0 || idx >= MAX_STREAM_NUMS)
	{
		return;
	}
	
	sys_os_mutex_enter(m_pInstMutex);
	
	if (m_pInstance[idx])
	{
		m_pInstance[idx]->m_nRefCnt--;

		if (m_pInstance[idx]->m_nRefCnt <= 0)
		{
			delete m_pInstance[idx];
			m_pInstance[idx] = NULL;
		}
	}

	sys_os_mutex_leave(m_pInstMutex);
}

int CLiveVideo::getStreamNums()
{
	// todo : return the max number of streams supported, don't be more than MAX_STREAM_NUMS

	return 1;
}

bool CLiveVideo::initCapture(int id, int channel, int codec, int width, int height, int framerate, int bitrate, const char* strUrl)
{
	CLock lock(m_pMutex);
	m_id = id;
	m_channel = channel;

	if (width != m_nWidth || height != m_nHeight)
	{
		if (!m_bufKey)
		{
			m_bufKey = (char*)malloc(sizeof(char) * 3840 * 2160 * 3 / 2);
		}
		else
			realloc(m_bufKey, m_nWidth * m_nHeight * 3 / 2);
	}
	else if (!m_bufKey)
	{
		m_bufKey = (char*)malloc(sizeof(char) * 3840 * 2160 * 3 / 2);
	}
		 
	if (m_bInited)
	{
		return true;
	}

	m_nWidth = width;
	m_nHeight= height;
	m_nFramerate = framerate;
	m_nBitrate = bitrate;

	m_vCodec = codec;
	// : here add your init code ... 
	// capture video
	
	m_rtsp = RtspInterface::Instance();
	m_nIndex = m_rtsp->openStream("zhuohe", strUrl, 1);
	//此处需要注意 设置rtp包的输入队列
	m_rtsp->m_rtsp[m_nIndex]->m_lsObj = m_lsObj;
	m_bInited = true;

	log_print(ZH_LOG_DEBUG, "[%s %d], strUrl[%s] index[%d] id=%d, int channel=%d \n", __FUNCTION__, __LINE__, strUrl, m_nIndex, id, channel);
	return true;
}

char * CLiveVideo::getAuxSDPLine(int rtp_pt)
{
	return NULL;
}

bool CLiveVideo::startCapture()
{
	CLock lock(m_pMutex);
	
	if (m_hCapture)
	{
		return true;
	}
	//printf("---!!!!before sys_os_create_thread captureThread \r\n");
	m_bCapture = true;
	m_hCapture = sys_os_create_thread((void *)liveVideoThread, this);
	printf("---!!!!after sys_os_create_thread captureThread %d\r\n", m_hCapture);

	log_print(ZH_LOG_TRACE, "%s %d \n", __FUNCTION__, __LINE__);
	return m_hCapture ? true : false;
}

void CLiveVideo::stopCapture()
{
//	CLock lock(m_pMutex);
	
	m_bCapture = false;
	
	while (m_hCapture)
	{
		usleep(10*1000);
	}

	// : here add your uninit code ...
	//ok capture video
	if (m_rtsp != nullptr)
	{
		m_rtsp->closeStream(m_nIndex);
	}

	log_print(ZH_LOG_DEBUG, "%s %d, index[%d] \n", __FUNCTION__, __LINE__, m_nIndex);

	m_bInited = false;
}

bool CLiveVideo::captureThread()
{
	// : when get the encoded data, call procData 
	// capture video from rtsp
	//printf("---!!!!before captureThread \r\n");
	//nDataType 帧类型  1== i帧, 2==帧, 3==b帧, 4==sps/pps信息, 0==强制关键帧
	while (m_bCapture)
	{
        //ok : here add the capture handler ... 
		//关键帧 先发sps pps

		if (m_bIKey)
		{
			uint8_t* bufsps = new uint8_t[m_nSps];
			memcpy(bufsps, m_bufSps, m_nSps);
			procData(bufsps, m_nSps, 4);
			uint8_t* bufKey = new uint8_t[m_nKey];
			memcpy(bufKey, m_bufKey, m_nKey);
			procData(bufKey, m_nKey, 0);
			m_bIKey = false;

			//log_print(ZH_LOG_TRACE, "captureThread m_bIKey \n", m_nKey);

			//printf("---!!!!captureThread m_bIKey \r\n");
			continue;
		}
		//printf("---!!!!while before popObj \r\n");
		RTP_BUFFER obj;
		//printf("---!!!!while popObj %p \r\n", m_lsObj);
		if (!m_lsObj->popObj(obj))
		{
			//log_print(ZH_LOG_TRACE, "!m_lsObj->popObj \n");
			usleep(1000);
			continue;
		}

		//restart 在设置SetFirstFrameParams的时候设置camIndex
		if (obj.uLen == -1)
		{
			usleep(1000);
			continue;
		}
		if (obj.uFrameType == 1)
			procData(obj.buf, obj.uLen, 1);
		else
			procData(obj.buf, obj.uLen, 2);

		//保存I帧//if (pkt.flags == 1){
		if (obj.uFrameType == 1)
		{
			if (obj.uLen > sizeof(m_bufKey))
			{
				log_print(ZH_LOG_ERROR, "i key frame size is %d \r\n", obj.uLen);
			}
			memcpy(m_bufKey, obj.buf, obj.uLen);
			m_nKey = obj.uLen;
		}
		//sps pps
		if (obj.uFrameType == 11)
		{
			if (obj.uLen > sizeof(m_bufSps))
			{
				log_print(ZH_LOG_ERROR, "sps pps info size is %d \r\n", obj.uLen);
			}
			memcpy(m_bufSps, obj.buf, obj.uLen);
			m_nSps = obj.uLen;
		}

		delete[] obj.buf;

		usleep(1000);
	}

	m_hCapture = 0;
	
	return true;
}

bool CLiveVideo::isCallbackExist(LiveVideoDataCB pCallback, void *pUserdata)
{
	bool exist = false;
#if 0
	LiveVideoCB * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB *) p_node->p_data;
		if (p_cb->pCallback == pCallback && p_cb->pUserdata == pUserdata)
		{
			exist = true;
			break;
		}
		
		p_node = h_list_lookup_next(m_pCallbackList, p_node);
	}
	h_list_lookup_end(m_pCallbackList);
	
	sys_os_mutex_leave(m_pCallbackMutex);
#endif
	return exist;
}

void CLiveVideo::addCallback(LiveVideoDataCB pCallback, void * pUserdata)
{
	if (isCallbackExist(pCallback, pUserdata))
	{
		return;
	}
	
	LiveVideoCB * p_cb = (LiveVideoCB *) malloc(sizeof(LiveVideoCB));

	p_cb->pCallback = pCallback;
	p_cb->pUserdata = pUserdata;
	p_cb->bFirst = true;
#if 0
	sys_os_mutex_enter(m_pCallbackMutex);
	h_list_add_at_back(m_pCallbackList, p_cb);	
	sys_os_mutex_leave(m_pCallbackMutex);
#endif
}

void CLiveVideo::delCallback(LiveVideoDataCB pCallback, void * pUserdata)
{
#if 0
	LiveVideoCB * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB *) p_node->p_data;
		if (p_cb->pCallback == pCallback && p_cb->pUserdata == pUserdata)
		{		
			free(p_cb);
			
			h_list_remove(m_pCallbackList, p_node);
			break;
		}
		
		p_node = h_list_lookup_next(m_pCallbackList, p_node);
	}
	h_list_lookup_end(m_pCallbackList);

	sys_os_mutex_leave(m_pCallbackMutex);
#endif
}

void CLiveVideo::SetDataCallBack(PInterDataCallBack callback)
{
	m_pCallback = callback;
}

void CLiveVideo::procData(uint8 * data, int size, int nDataType)
{
	//todo 
#if 0 // _DEBUG
	char ssf[256] = { 0 };
	sprintf(ssf, "testsdk_%d.264", m_channel);
	FILE * fl = fopen(ssf, "ab+");
	fwrite(data, 1, size, fl);
	fclose(fl);
	//printf("procData %d \n", size);
	printf("m_pCallback m_procData m_id=%d, m_channel=%d nDataType=%d, data=%p, size=%d\n", m_id, m_channel, nDataType, data, size);

#endif
	log_print(ZH_LOG_TRACE, "m_pCallback m_procData m_id=%d, m_channel=%d nDataType=%d, data=%p, size=%d\n", m_id, m_channel, nDataType, data, size);
	//callback 
	m_pCallback(m_id, m_channel, nDataType, data, size);

#if 0
	LiveVideoCB * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	//printf("---!!!!before procData size =%d \r\n", size);
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB *) p_node->p_data;
		if (p_cb->pCallback != NULL)
		{
			//printf("---!!!!start procData size =%d \r\n", size);
			p_cb->pCallback(data, size, p_cb->pUserdata);
		}
		
		p_node = h_list_lookup_next(m_pCallbackList, p_node);
	}
	h_list_lookup_end(m_pCallbackList);

	sys_os_mutex_leave(m_pCallbackMutex);
#endif
}
