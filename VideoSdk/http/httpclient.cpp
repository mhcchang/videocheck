
#include "httpclient.h"

HttpClient::HttpClient()
{
	s_exit_flag = 0;
	s_req_callback = nullptr;
	s_post_data = "";
	s_url = "";
	s_resp = "";

	fn = GETCB(mg_event_handler_t, HttpClient)(std::bind(&HttpClient::OnHttpEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

HttpClient::~HttpClient()
{

}
// 客户端的网络请求响应
void HttpClient::OnHttpEvent(mg_connection *connection, int ev, void *http_req, void *fn_data)//(mg_connection *connection, int ev, int event_type, void *event_data)
{
	mg_http_message *hm = (mg_http_message *)http_req;//(struct mg_http_message *)event_data;
	int connect_status;

	switch (ev)
	{
	case MG_EV_CONNECT:
	{
		connect_status = *(int *)hm;
		if (connect_status != 0)
		{
			//printf("Error connecting to server, error code: %d\n", connect_status);
			s_exit_flag = 1;
		}

		struct mg_str host = mg_url_host(s_url.c_str());

		// Send request
		int content_length = s_post_data.length();
		mg_printf(connection,
			"%s %s HTTP/1.0\r\n"
			"Host: %.*s\r\n"
			"Content-Type: octet-stream\r\n"
			"Content-Length: %d\r\n"
			"\r\n",
			s_post_data.c_str() ? "POST" : "GET", mg_url_uri(s_url.c_str()), (int)host.len,
			host.ptr, content_length);
		mg_send(connection, s_post_data.c_str(), content_length);

		break;
	}
	case MG_EV_POLL:
		if (mg_millis() > *(uint64_t *)connection->label &&
			(connection->is_connecting || connection->is_resolving)) {
			mg_error(connection, "Connect timeout");
		}
		break;
	case MG_EV_OPEN:
		*(uint64_t *)connection->label = mg_millis() + 1000;
		break;
	case MG_EV_HTTP_MSG:
	{
		//printf("Got reply:\n%.*s\n", (int)hm->body.len, hm->body.p);
		s_resp = std::string(hm->body.ptr, hm->body.len);
		connection->is_closing = 1;
		s_exit_flag = 1; // 每次收到请求后关闭本次连接，重置标记

		// 回调处理
		s_req_callback(s_url, s_resp);
	}
	break;
	case MG_EV_CLOSE:
		if (s_exit_flag == 0)
		{
			//printf("Server closed connection\n");
			s_exit_flag = 1;
		};
		break;
	default:
		break;
	}
}

// 发送一次请求，并回调处理，然后关闭本次连接
void HttpClient::SendReq(const std::string url, const std::string extra_headers,
	const std::string post_data, ReqCallback req_callback)
{
	// 给回调函数赋值
	s_req_callback = req_callback;
	mg_mgr mgr;
	mg_mgr_init(&mgr);

	//auto connection = mg_http_connect(&mgr, url.c_str(), extra_headers.c_str(), post_data.c_str());
	mg_connection* connection = mg_http_connect(&mgr, url.c_str(), fn, (void*)&mgr);
	//mg_set_protocol_http_websocket(connection);
	s_url = url.c_str();
	while (s_exit_flag == 0)
		mg_mgr_poll(&mgr, 50);

	mg_mgr_free(&mgr);
}

void HttpClient::SyncSendReq(const std::string url, const std::string extra_headers,
	const std::string post_data, std::string &resp)
{
	mg_mgr mgr;
	mg_mgr_init(&mgr);

	mg_connection* connection = mg_http_connect(&mgr, url.c_str(), fn, (void*)&mgr);

	while (s_exit_flag == 0)
		mg_mgr_poll(&mgr, 50);

	mg_mgr_free(&mgr);

	resp = std::move(s_resp);
}
