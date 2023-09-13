
#pragma once
#include <string>
#include <functional>
#include "mongoose.h"

// 此处必须用function类，typedef再后面函数指针赋值无效
//std::string, std::string 1=url 2=body
using ReqCallback = std::function<void(std::string, std::string)>;

class HttpClient_St
{
public:
	HttpClient() {}
	~HttpClient() {}

	static void SyncSendReq(const std::string url, const std::string extra_headers,
		const std::string post_data, std::string &resp);

	static void SendReq(const std::string url, const std::string extra_headers,
		const std::string post_data, ReqCallback req_callback);
	static void OnHttpEvent(mg_connection *connection, int ev, void *http_req, void *fn_data);
	//static void OnHttpEvent(mg_connection *connection, int event_type, void *event_data);
	static int s_exit_flag;
	static ReqCallback s_req_callback;

	static std::string s_post_data;
	static std::string s_url;

	static std::string s_resp;
};
//	static void HandleHttpEvent(mg_connection *connection, int ev, void *http_req, void *fn_data);
