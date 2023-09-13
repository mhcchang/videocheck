#pragma once

#include <vector>
using namespace std;

#include "RtspStream.h"
#include "pubutil.h"

class RtspInterface
{
public:
	RtspInterface(void);
	~RtspInterface(void);
	static RtspInterface* Instance();
	static RtspInterface *m_pInstance;
	int Init();
	void UnInit();

	int openStream(char const* progName, char const* rtspURL, int debugLevel);
	int getStreamStatus(int nStream);
	int closeStream(int nStream);
public:
	RtspSession* m_rtsp[C_MAX_RTSP_CLIENT];
};

int IPNC_OpenStream(char const* progName, char const* rtspURL, int debugLevel);
int IPNC_GetStreamStatus(int nStream);
int IPNC_CloseStream(int nStream);
