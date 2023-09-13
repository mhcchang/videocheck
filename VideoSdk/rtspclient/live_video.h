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

#ifndef LIVE_VIDEO_H
#define LIVE_VIDEO_H

#include "../VideoSdkImpl.h"
#include "RtspInterface.h"
#include <functional>

/***************************************************************************************/

#define MAX_STREAM_NUMS  16

#include "pubutil.h"
#include "ObjectLists.h"

typedef void (*LiveVideoDataCB)(uint8_t *data, int size, void *pUserdata);
//typedef void (CALLBACK *PInterDataCallBack) (int id, int channel, unsigned long nDataType, unsigned char *pBuffer, unsigned long nBufSize);
typedef std::function<void(int id, int channel, unsigned long nDataType, unsigned char *pBuffer, unsigned long nBufSize)> PInterDataCallBack;

typedef struct
{
	LiveVideoDataCB pCallback;
	void *          pUserdata;
	bool            bFirst;
} LiveVideoCB;

/***************************************************************************************/

class CLiveVideo
{
public:
    virtual ~CLiveVideo();
    
    // get the support stream numbers
    static int					getStreamNums();
    							
    static CLiveVideo *			getInstance(int idx);
    							
    virtual void				freeInstance(int idx);
								
    virtual bool				initCapture(int id, int channel, int codec, int width, int height, int framerate, int bitrate, const char * strUrl);
	virtual bool				startCapture();
	void						SetDataCallBack(PInterDataCallBack callback);
    virtual void				addCallback(LiveVideoDataCB pCallback, void * pUserdata);
    virtual void				delCallback(LiveVideoDataCB pCallback, void * pUserdata);
    							
    virtual char *				getAuxSDPLine(int rtp_pt);
    virtual bool				captureThread();
								
	virtual void				stopCapture();

	bool						getCaptureStatus() { return m_bCapture; };
protected:
    CLiveVideo();
	CLiveVideo(CLiveVideo &obj) { *this = obj; };

    bool						isCallbackExist(LiveVideoDataCB pCallback, void *pUserdata);
    void						procData(uint8 * data, int size, int nDataType);

public:
    int							m_nDevIndex;
    int							m_nRefCnt;
								
    static void *				m_pInstMutex;
	static CLiveVideo *			m_pInstance[MAX_STREAM_NUMS];
	
protected:
	int							m_nWidth;
	int							m_nHeight;
	int							m_nFramerate;
    int							m_nBitrate;
    							
    void *						m_pMutex;
    bool						m_bInited;
    volatile bool				m_bCapture;
    pthread_t					m_hCapture;
								
    void *						m_pCallbackMutex;
	//LINKED_LIST *				m_pCallbackList;
	int							m_vCodec;

	int							m_id;					
	int							m_channel;
	PInterDataCallBack			m_pCallback;
public:						  
	RtspInterface *				m_rtsp;
	int							m_nIndex;
								
	char						*m_bufKey;
	uint32_t					m_nKey;
								
	char						m_bufSps[1024];
	uint32_t					m_nSps;
	volatile bool				m_bIKey;
	ObjectLists<RTP_BUFFER>*	m_lsObj;
};

#endif
