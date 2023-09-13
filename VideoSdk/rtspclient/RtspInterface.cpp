
#include "RtspInterface.h"
#include "sys_inc.h"

RtspInterface * RtspInterface::m_pInstance = nullptr;

RtspInterface::RtspInterface(void)
{
	Init();
}

RtspInterface::~RtspInterface(void)
{
	UnInit();
}

RtspInterface* RtspInterface::Instance()
{
	if (nullptr == m_pInstance)
	{
		m_pInstance = new RtspInterface;

	}
	return m_pInstance;
}

int RtspInterface::Init()
{
	for (int i = 0; i < C_MAX_RTSP_CLIENT; i++)
	{
		m_rtsp[i] = NULL;
	}
	return 0;
}

void RtspInterface::UnInit()
{
	for (int i = 0; i < C_MAX_RTSP_CLIENT; i++)
	{
		if (m_rtsp[i] != NULL)
		{
			m_rtsp[i]->stopRTSPClient();
			delete m_rtsp[i];
			m_rtsp[i] = NULL;
		}
	}
}

int RtspInterface::openStream(char const* progName, char const* rtspURL, int debugLevel)
{
	int nIndex = -1;
	for (int i = 0; i < C_MAX_RTSP_CLIENT; i++)
	{
		if (m_rtsp[i] == NULL)
		{
			nIndex = i;
			break;
		}
	}
	if (nIndex < 0)
		return -1; //busy

	RtspSession* pRtsp = new RtspSession;
	pRtsp->m_nID = nIndex;
	m_rtsp[nIndex] = pRtsp;

	if (pRtsp->startRTSPClient(progName, rtspURL, debugLevel))
	{
		delete pRtsp;
		pRtsp = NULL;
		return -2;
	}

	return nIndex;
}

int RtspInterface::getStreamStatus(int nStream)
{
	RtspSession* pRtsp = m_rtsp[nStream];
	if (pRtsp)
	{
		return pRtsp->CheckStatus();
	}
	return -1;
}

int RtspInterface::closeStream(int nStream)
{
	RtspSession* pRtsp = m_rtsp[nStream];
	if (pRtsp)
	{
		pRtsp->stopRTSPClient();
		while (1)
		{
			if (pRtsp->CheckStatus() == 2)
				//if (!pRtsp->CheckRunning())
			{
				delete pRtsp;
				pRtsp = NULL;
				m_rtsp[nStream] = NULL;
				break;
			}
            usleep(3000);
		}
	}
	return 0;
}

//****************************************
//c interface
int IPNC_OpenStream(char const* progName, char const* rtspURL, int debugLevel)
{
	return RtspInterface::Instance()->openStream(progName, rtspURL, debugLevel);
}

int IPNC_GetStreamStatus(int nStream)
{
	return RtspInterface::Instance()->getStreamStatus(nStream);
}

int IPNC_CloseStream(int nStream)
{
	return RtspInterface::Instance()->closeStream(nStream);
}
