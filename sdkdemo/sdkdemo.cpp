// sdkdemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "VideoSdk.h"
#include <fstream>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/utils.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <thread>
std::string readFile(const char* configPath)
{
	std::ifstream ifs(configPath);
	if (!ifs)
		return std::string();

	ifs.seekg(0, std::ios::end);
	std::size_t len = ifs.tellg();
	char* buf = new char[len + 1];
	memset(buf, 0, len + 1);
	ifs.seekg(0, std::ios::beg);
	ifs.read(buf, len);

	std::string content(buf, len);
	delete[] buf;

	return content;
}

int main(int argc, char * argv[])
{
	bool bl = false;

	int logLevel = 4;
	if (argc > 2)
	{
		logLevel = atoi((const char*)argv[2]);
	}

	//1.  初始化sdk
	bl = VideoSdk_Init(2, 25, 0.75);
	//设置日志级别

	if (!bl)
	{
		printf(" Init error code=%d \n", VideoSdk_GetLastError());
		return -1;
	}

	//devicelist.json
	char jsonin[1024] = { 0 };
	char jsonres[1024] = { 0 };
	int nres = 0;
	std::string content = readFile("./devicelist.json");

	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(content.c_str());
	
	if (doc.HasParseError() || !doc.IsObject())
	{
		printf(" json error \n");
		return -2;
	}

	int nret = 0;
	int i = 0;

	//1
	nret = VideoSdk_SetStreamStatusQuest(content.c_str());
	printf(" VideoSdk_SetStreamStatusQuest return=%d \n", nret);

	i = 10;
	while (i--)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	nret = VideoSdk_GetStreamStatusRespond(content.c_str(), jsonres, nres);
	printf(" VideoSdk_GetStreamStatusRespond return=%d nres=%d json=%s\n", nret, nres, jsonres);

	i = 2;
	while (i--)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	//2
	nret = VideoSdk_SetForceKeyFrameQuest(content.c_str());
	printf(" VideoSdk_SetForceKeyFrameQuest return=%d \n", nret);

	i = 5;
	while (i--)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	nret = VideoSdk_GetForceKeyFrameRespond(content.c_str(), jsonres, nres);
	printf(" VideoSdk_GetForceKeyFrameRespond return=%d nres=%d json=%s\n", nret, nres, jsonres);



	//3
	 nret = VideoSdk_SetKeyFrameQuest(content.c_str());
	printf(" VideoSdk_SetKeyFrameQuest return=%d \n", nret);

	i = 20;
	while (i--)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	nret = VideoSdk_GetKeyFrameRespond(content.c_str(), jsonres, nres);
	printf(" VideoSdk_GetKeyFrameRespond return=%d nres=%d json=%s\n", nret, nres, jsonres);


	//4
	nret = VideoSdk_SetPollingQuest(content.c_str());
	printf(" VideoSdk_SetPollingQuest return=%d \n", nret);

	i = 20;
	while (i--)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	nret = VideoSdk_GetPollingRespond(content.c_str(), jsonres, nres);
	printf(" VideoSdk_GetPollingRespond return=%d nres=%d json=%s\n", nret, nres, jsonres);

	//5
	VideoSdk_UnInit();
	printf(" keypress any exit!\n");
	getchar();

    return 42;
}
