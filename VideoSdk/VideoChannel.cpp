#include "defs.h"
#include "VideoChannel.h"

#include <thread>

#define MAX_PS_BUFFER_SIZE 10485760

#ifndef H264_FRAME_SIZE_MAX
#define H264_FRAME_SIZE_MAX (1024*1024*10)
#endif
#define C_NORMALFRAMERATE 25

#include "VideoSdkImpl.h"

VideoChannel::VideoChannel()
{
    Port = 0;
	m_tmLastKeyFrame = (uint64_t)(-1);
	m_tmNowKeyFrame = (uint64_t)(-1);
	m_nWorkMode = -1;
	m_nMilliSecond = 0;
	m_tmWorkStart = 0;
	m_nKeyFrameCount = 0;
	m_nFrameCount = 0;
    RtpSSRC = 1;

	m_nInviteMode = 0;

    m_nLastSequenceNumber = 0;
    m_bIsCurrentFrameLostPacket = false;

	m_nReceiveRtpTime = 0;

    m_pRtpBuffer = (uint8_t*)malloc(MAX_PS_BUFFER_SIZE);
    m_nRtpBufferSize = 0;

    m_pCodecCtx = nullptr;
    m_pCodec = nullptr;

    m_pCodecCtx = nullptr;
    m_pFrameRGB = nullptr;
    m_pFrame = nullptr;

    m_out_buffer_rgb = nullptr;
    m_nBytes_rgb = 0;

    m_pH264buf = nullptr;
	m_nFrameRate = 0;

	m_rtsp = nullptr;

	m_bImmediate = true;
    m_nTerm = -1;
    m_startTime = "";
	m_blStreamClosed = true;
	m_pMain = VideoSdkImpl::GetInstance();
	m_blInitThread = false;
	m_rtsp = RtspInterface::Instance();
	m_lsObj = new ObjectLists<RTP_BUFFER>;

	m_pH264buf = (char *)malloc(H264_FRAME_SIZE_MAX);

	m_httpclient = new HttpClient();
}

VideoChannel::~VideoChannel()
{
	delete m_httpclient;

    if (m_pRtpBuffer != nullptr)
        free(m_pRtpBuffer);
    m_pRtpBuffer = nullptr;

    if (m_pH264buf != nullptr)
        free(m_pH264buf);	
	delete m_lsObj;
}

void VideoChannel::threadStart()
{
	if (m_blInitThread)
	{
		threadStop();
	}

    m_bIsLastKeyFrameLostPacket = false; //上一个I帧是否丢包了，是的话，接下来的帧都有可能花屏 接下来所有的帧， 都不传入检测
    m_bIsKeyFrameGetted = false; //用来记录I帧是否获取到了 否则丢弃得到的h264帧

    OpenH264Decoder();
	m_blRunning = false;
	m_thread = std::thread(&VideoChannel::process, this);
}

void VideoChannel::threadStop()
{
	m_blRunning = false;
	if (!m_blStreamClosed)
	{
		m_blStreamClosed = true;
		m_rtsp->closeStream(m_nChannelId);
		m_nChannelId = 0;
		CloseH264Decoder();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	if (m_thread.joinable())
	{
		{
			std::unique_lock<std::mutex> lock(this->m_mutex);
			m_thread.join();
		}
	}
	m_blInitThread = false;
}

void VideoChannel::dealwithData()
{
	
}

void VideoChannel::process()
{
	m_blInitThread = true;
	m_blRunning = true;
	printf(" - - - run \n");
	while (m_blRunning)
	{
		std::unique_lock<std::mutex> lock(this->m_mutex);
		//printf(" - 111 run \n");
		uint64_t tm = MHC_STLNOW_MILLISEC;

		switch (m_nWorkMode)
		{
		case 1:
		{
			//关键帧
			if (m_tmWorkStart > 0 && m_nMilliSecond > 0)
			{
				if (tm - m_tmWorkStart > m_nMilliSecond + 900)
				{
					//m_tmWorkStart = 0;
					printf("///////////stop 1 %s [tm - m_tmWorkStart=%d][%llu], [m_nMilliSecond=%d] %d, %d\n", this->DeviceID.c_str(), tm - m_tmWorkStart, m_tmWorkStart, m_nMilliSecond, m_nFrameCount, m_nFrameRate);
					m_blRunning = false;
					//return;// threadStop();
				}
			}
			break;
		}
		case 2:
		{
			if (m_tmWorkStart > 0 && m_nMilliSecond > 0)
			{
				if (tm - m_tmWorkStart > m_nMilliSecond + 900)
				{
					printf("///////////stop 2 %s  [tm - m_tmWorkStart=%d][%llu], [m_nMilliSecond=%d] %d, %d \n", this->DeviceID.c_str(), tm - m_tmWorkStart, m_tmWorkStart,
						m_nMilliSecond, m_nFrameCount, m_nFrameRate);

					//m_tmWorkStart = 0;
					m_blRunning = false; //threadStop();
					//return;
				}
			}
			break;
		}
		case 3:
		{
			if (m_tmWorkStart > 0 && m_nMilliSecond > 0)
			{
				if (tm - m_tmWorkStart > m_nMilliSecond + 900)
				{
					//m_tmWorkStart = 0;
					printf("///////////stop 3  [tm - m_tmWorkStart=%d][%llu], [m_nMilliSecond=%d] %d, %d \n", this->DeviceID.c_str(), tm - m_tmWorkStart, m_tmWorkStart,
						m_nMilliSecond, m_nFrameCount, m_nFrameRate);

					m_blRunning = false;
					//return;//threadStop();
				}
			}
			break;
		}
		default:
			break;
		}

		///printf("m_lsObj size %d \n", m_lsObj->size());
		RTP_BUFFER obj;
		while (m_lsObj->popObj(obj))
		{
			if (obj.uLen == -1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			//procData(obj.buf, obj.uLen);

			//
			if (obj.uFrameType == 1)
			{
				m_nKeyFrameCount++;
				m_nFrameCount++;

				m_nFrameRate = m_nFrameCount / ((tm - m_tmWorkStart) / 1000.0f);
			}
			//sps pps
			if (obj.uFrameType == 11)
			{
				//
			}
			if (obj.uFrameType == 3)
			{
				// data vps
			}
			if (obj.uFrameType == 2)
			{
				m_nFrameCount++;
				m_nFrameRate = m_nFrameCount / ((tm - m_tmWorkStart) / 1000.0f);
			}

			//printf("delete obj buf %p %d, %d %llu %llu \n", obj.buf, m_nFrameCount, m_nFrameRate, tm - m_tmWorkStart, m_tmWorkStart);
			delete[] obj.buf;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	m_rtsp->closeStream(m_nChannelId);
	CloseH264Decoder();
	m_blStreamClosed = true;
}

void VideoChannel::dealwithDataNode(const RtpDataNode &node)
{
    unsigned char * ptr = node.buffer;
    int psLen = node.size;

    if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01 && ptr[3] == 0xBA && psLen > 0)
    {
        int h264length = 0;

        if (h264length > 0)
        {
#ifdef DEBUG            
            if (NULL == m_fl)
            {
                time_t tmn;
                time(&tmn);
                struct tm *t1;
                t1 = localtime(&tmn);
                char sztmn[256] = { 0 };
                snprintf((char*)sztmn, 26, "%04d%02d%02d-%02d%02d%02d.h264",
                    t1->tm_year + 1900, t1->tm_mon + 1, t1->tm_mday,
                    t1->tm_hour, t1->tm_min, t1->tm_sec);                
                
                m_fl = fopen(sztmn, "ab+");
            }
            int ir = fwrite(m_pH264buf, 1, h264length, m_fl);
            if (ir != h264length)
                fprintf(stderr, "%d fwrite length=%d ir=%d\n", __LINE__, h264length, ir);    
#endif            
            bool isKeyFrame = false;

//          ///还没获取到关键帧 则先判断是否是关键帧 否则丢弃
//          if (!isKeyFrameGetted)
            {
                int pos = 0;
                while(1)
                {
                    if ((h264length - pos) < 4)
                    {
                        break;
                    }

                    unsigned char *buf = (unsigned char *)(m_pH264buf + pos);

                    int index = 0;

                    if (buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x01)
                    {
                        index = 3;
                    }
                    else if (buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x00 && buf[3] == 0x01)
                    {
                        index = 4;
                    }
                    else
                    {
                        pos++;
                        continue;
                    }

                    int nal_unit_type = (buf[index]) & 0x1f;  // 5 bit

                    if (nal_unit_type == 7)
                    {
                        m_bIsLastKeyFrameLostPacket = node.isLostPacket;
                        isKeyFrame = true;
						m_tmLastKeyFrame = MHC_STLNOW_MILLISEC; // std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                        m_bIsKeyFrameGetted = true;
                        break;
                    }

                    pos++;
                }
            }

            fprintf(stderr, "%d dealwithDataNode length=%d m_bIsKeyFrameGetted=%d\n", __LINE__, h264length, m_bIsKeyFrameGetted);

            //if (m_bIsKeyFrameGetted)
            {
                bool isLostPacket = node.isLostPacket;

                if (node.isLostPacket)
                {
                    fprintf(stderr, "lost packet! isKeyFrame=%d\n",isKeyFrame);
                }

                if (m_bIsLastKeyFrameLostPacket) //I帧丢了，接下来的帧都不检测，直到遇到下一个I帧
                {
                    isLostPacket = true;
                }

                DecodeH264Buffer((uint8_t*)m_pH264buf, h264length, isLostPacket);
            }
        }
    }

    free(node.buffer);

	//mhc  工作模式
	uint64_t tm = MHC_STLNOW_MILLISEC;
	//检测完成 结束
    printf("%d tm %llu m_tmWorkStart %llu  m_nMilliSecond %llu,  [tm - m_tmWorkStart = %llu] \n ", __LINE__, tm, m_tmWorkStart, m_nMilliSecond, tm - m_tmWorkStart);

	switch (m_nWorkMode)
	{
	case 1:
	{
		//关键帧
		if (m_tmWorkStart > 0 && m_nMilliSecond > 0)
		{
			if (tm - m_tmWorkStart > m_nMilliSecond + 900)
			{
               // m_tmWorkStart = 0;
                printf("///////////stop 1 %s", this->DeviceID.c_str());
                
				m_blRunning = false;
#ifdef DEBUG
                if (NULL != m_fl)
                {
                    fclose(m_fl);
                    m_fl = NULL;
                } 
#endif                
            }	
		}
		break;
	}
	case 2:
	{
		if (m_tmWorkStart > 0 && m_nMilliSecond > 0)
		{
			if (tm - m_tmWorkStart > m_nMilliSecond + 900)
			{
                printf("///////////stop 2 %s", this->DeviceID.c_str());

                //m_tmWorkStart = 0;
                m_blRunning = false;
            }	
		}
		break;
	}
    case 3:
    {
		if (m_tmWorkStart > 0 && m_nMilliSecond > 0)
		{
			if (tm - m_tmWorkStart > m_nMilliSecond + 900)
			{
                //m_tmWorkStart = 0;
                printf("///////////stop 3 %s", this->DeviceID.c_str());
				m_blRunning = false;
            }	
		}     
        break;   
    }
	default:
		break; // this->stop(true);
	}
}

///塞入rtp数据等待处理
void VideoChannel::InputRtpBuffer(uint8_t *buffer, int size, uint32_t sequenceNumber, bool isLastPacket)
{
    if ((sequenceNumber - m_nLastSequenceNumber ) != 1)
    {
        if (m_nLastSequenceNumber != 0 && sequenceNumber != 0)
        {
            m_bIsCurrentFrameLostPacket = true;

            fprintf(stderr,"sequenceNumber=%d %d\n", sequenceNumber, m_nLastSequenceNumber);
        }
    }

    m_nLastSequenceNumber = sequenceNumber;

    m_nReceiveRtpTime = MHC_STLNOW_MILLISEC;

    unsigned char *ptr = buffer;

    ///这是一个ps头 则处理上一包
    if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01 && ptr[3] == 0xBA)
    {
        if (m_nRtpBufferSize > 0)
        {
            RtpDataNode node;
            node.isLostPacket = m_bIsCurrentFrameLostPacket;
            node.size = m_nRtpBufferSize;
            node.buffer = (uint8_t *)malloc(m_nRtpBufferSize + 10);
            memcpy(node.buffer, m_pRtpBuffer, m_nRtpBufferSize);

            int listSize = 0;

            m_nRtpBufferSize = 0;
            m_bIsCurrentFrameLostPacket = false;

            if (listSize > 100 && listSize % 100 == 0)
            {
                fprintf(stderr, "decode too slow! mRtpBufferList=%d RtpSSRC=%d\n", listSize, RtpSSRC);
            }
        }
    }

    if ((m_nRtpBufferSize + size) > MAX_PS_BUFFER_SIZE)
    {
        fprintf(stderr, "rtp packet out of range! mRtpBufferSize=%d size=%d isLastPacket=%d\n", m_nRtpBufferSize, size, isLastPacket);
        m_nRtpBufferSize = 0;
        return;
    }

    memcpy(m_pRtpBuffer + m_nRtpBufferSize, buffer, size);
    m_nRtpBufferSize += size;
}

bool VideoChannel::OpenH264Decoder()
{
    return true;
    bool isOpenHardDecoderucceed = false;

    m_pCodecCtx = avcodec_alloc_context3(NULL);

    ///查找硬件解码器 h264_cuvid
    m_pCodec = avcodec_find_decoder_by_name("h264");

    if (m_pCodec == NULL)
    {
        fprintf(stderr,"h264Codec not found.\n");
    }
    else
    {
        ///打开解码器
        if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0)
        {
            avcodec_close(m_pCodecCtx);
            fprintf(stderr,"Could not open codec h264_cuvid\n");
        }
        else
        {
            isOpenHardDecoderucceed = true;
        }
    }

    if (!isOpenHardDecoderucceed)
    {
        fprintf(stderr,"\n\nOpen h264 failed! use AV_CODEC_ID_H264\n\n");

        m_pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);

        if (m_pCodec == NULL)
        {
    //      qDebug("Codec not found.\n");
            return false;
        }

        m_pCodecCtx->thread_count = 8;

        ///打开解码器
        if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0) 
        {
            printf("Could not open codec.\n");
            avcodec_close(m_pCodecCtx);
            return false;
        }
    }
    else
    {
        fprintf(stderr,"\n\nOpen h264 succeed !\n\n");
    }

    m_pFrame = av_frame_alloc();

    av_init_packet(&m_packet);

	printf("open codec h264 succeed!\n");
    return true;
}

void VideoChannel::CloseH264Decoder()
{
    return;
    if (m_pCodecCtx == NULL) 
	{
        avcodec_close(m_pCodecCtx);
    }

    av_frame_free(&m_pFrame);
    av_frame_free(&m_pFrameRGB);

    av_free(m_out_buffer_rgb);

    m_pCodecCtx = NULL;
    m_pCodec = NULL;

    m_out_buffer_rgb = NULL;

    m_pFrameRGB = NULL;
    m_pFrame = NULL;

    printf("closeH264Decoder succeed!\n");
}

void VideoChannel::DecodeH264Buffer(uint8_t *buffer, int size, bool isLostPacket)
{
    m_packet.data = buffer;
    m_packet.size = size;

    static int ic = 0;
    //fprintf(stderr, "decodeH264Buffer %d!\n", ic++);
    if (avcodec_send_packet(m_pCodecCtx, &m_packet) != 0)
    {
       fprintf(stderr, "input AVPacket to decoder failed!\n");
       return;
    }

    while (0 == avcodec_receive_frame(m_pCodecCtx, m_pFrame))
    {
        ///判断解码完毕的帧是否是关键帧
        bool isKeyFrame = false;

        if(m_pFrame->key_frame)
        {
            isKeyFrame = true;
			m_nKeyFrameCount++;
			m_tmLastKeyFrame = m_tmNowKeyFrame;
			m_tmNowKeyFrame = MHC_STLNOW_MILLISEC;

            ;//fprintf(stderr, "-----avcodec_receive_frame key_frame %d!\n", m_nKeyFrameCount);
        }
        else
            ;// fprintf(stderr, "---++avcodec_receive_frame  %d!\n", m_nFrameCount);
        
#if 0
        if (pFrameRGB == NULL)
        {
            pFrameRGB = av_frame_alloc();

            ///将解码后的YUV数据转换成RGB32
            img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                    pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                    AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

            //numBytes_rgb = avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);
			numBytes_rgb = av_image_get_buffer_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1);
			//avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);
            out_buffer_rgb = (uint8_t *) av_malloc(numBytes_rgb * sizeof(uint8_t));

            //avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,
             //       pCodecCtx->width, pCodecCtx->height);
			 av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer_rgb, AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1);
        }


        if (isLostPacket)
        {
            fprintf(stderr,"current frame lost packet! do not dealwith it %s! \n", DeviceID.c_str());
        }
        else
        {
            sws_scale(img_convert_ctx,
                    (uint8_t const * const *) pFrame->data,
                    pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                    pFrameRGB->linesize);

            dealWithRgb32Frame(out_buffer_rgb, numBytes_rgb, pCodecCtx->width, pCodecCtx->height, isLostPacket, isKeyFrame);
        }
	#endif // 	
		//工作模式
		switch (m_nWorkMode)
		{
		case 1:
		{
			//关键帧
            //mhc test
			//if (!isLostPacket && isKeyFrame)
            if (isKeyFrame)
			{
                //m_nKeyFrameCount++;
                fprintf(stderr, "m_nWorkMode %d KeyFrame  %d!\n", m_nWorkMode, m_nKeyFrameCount);
            }	
			break;
		}
		case 2:
		{
            //mhc test
			//if (!isLostPacket)
				m_nFrameCount++;

             fprintf(stderr, "m_nWorkMode %d Frame %d!\n", m_nWorkMode, m_nFrameCount);
			break;
		}
        case 3:
        {
            m_nFrameCount++;
            break;
        }
		default:
		{
			//if (m_pCallByeHandle)
			//	m_pCallByeHandle(this);
			//this->stop(true);
			break;
		}
		}

        if (!IsCheckIng())
        {

        }
    }
}

void VideoChannel::DealWithRgb32Frame(const uint8_t *rgb32Buffer, int bufferSize, int width, int height, bool isLostPacket, bool isKeyFrame)
{
}

void VideoChannel::SetWorkMode(int nWorkMode, int nSecond) 
{
	m_nWorkMode = nWorkMode; 
	m_nMilliSecond = nSecond * 1000;

	m_nKeyFrameCount = 0;
	m_nFrameCount = 0;

	if (RtspUrl != "" && !m_blRunning)
	{
		m_tmWorkStart = MHC_STLNOW_MILLISEC;
		m_blStreamClosed = true;
		m_nChannelId = m_rtsp->openStream("VideoCheck", RtspUrl.c_str(), 1);
		m_rtsp->m_rtsp[m_nChannelId]->m_lsObj = m_lsObj;
		threadStart();
	}
}

bool VideoChannel::IsCheckIng()
{
	uint64_t tm = MHC_STLNOW_MILLISEC;
	if (tm - m_tmWorkStart > m_nMilliSecond)
		return false;
	else
		return true;
}

bool VideoChannel::GetWorkResult(int &nKeyFrameCount, int &nFrameRate)
{
	nKeyFrameCount = m_nKeyFrameCount;
	nFrameRate = m_nFrameRate;
	switch (m_nWorkMode)
	{
	default:
	case 0:
		return false;

	case 1:
	{
		if (nKeyFrameCount <= 0)
			return false;
		//强制关键帧
		if (nKeyFrameCount > 0 && m_nMilliSecond <= 1000)
			return true;

		//持续关键帧数
		if (nKeyFrameCount > 0 && m_nMilliSecond > 1000)
		{
			//只有关键帧数大于等于 设置的检测数确定为真
			if (nKeyFrameCount >= m_nMilliSecond / 1000.0f)
				return true;
		}
		return false;
	}
	case 2:
	{
		//帧数 判断视频流是否正常
		if (m_nFrameCount >= m_nMilliSecond / 1000.0f * C_NORMALFRAMERATE * 0.9)
			return true;
		else
			return false;
	}
    case 3:
    {
		if (nKeyFrameCount <= 0)
			return false;
		//强制关键帧
		if (nKeyFrameCount > 0 && m_nMilliSecond <= 1000)
			return true;

		//持续关键帧数
		if (nKeyFrameCount > 0 && m_nMilliSecond > 1000)
		{
			//只有关键帧数大于等于 设置的检测数确定为真
			if (nKeyFrameCount >= m_nMilliSecond / 1000.0f)
				return true;
		}

		//帧数 判断视频流是否正常
		if (m_nFrameCount >= m_nMilliSecond / 1000.0f * C_NORMALFRAMERATE * 0.9)
			return true;
		else
			return false;        
    }
	}
}
