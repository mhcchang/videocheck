
#pragma once
#include <string>
#include <functional>
#include "mongoose.h"

/*
typedef void (*voidCCallback)();
template<typename T>
voidCCallback makeCCallback(void (T::*method)(),T* r){
  Callback<void()>::func = std::bind(method, r);
  void (*c_function_pointer)() = static_cast<decltype(c_function_pointer)>(Callback<void()>::callback);
  return c_function_pointer;
}

voidCCallback callback = makeCCallback(&Foo::print, this);
plainOldCFunction(callback);
*/

#include <type_traits>

template<typename T>
struct ActualType {
	typedef T type;
};
template<typename T>
struct ActualType<T*> {
	typedef typename ActualType<T>::type type;
};

template<typename T, unsigned int n, typename CallerType>
struct Callback;

template<typename Ret, typename ... Params, unsigned int n, typename CallerType>
struct Callback<Ret(Params...), n, CallerType> {
	typedef Ret(*ret_cb)(Params...);
	template<typename ... Args>
	static Ret callback(Args ... args) {
		func(args...);
	}

	static ret_cb getCallback(std::function<Ret(Params...)> fn) {
		func = fn;
		return static_cast<ret_cb>(Callback<Ret(Params...), n, CallerType>::callback);
	}

	static std::function<Ret(Params...)> func;

};

template<typename Ret, typename ... Params, unsigned int n, typename CallerType>
std::function<Ret(Params...)> Callback<Ret(Params...), n, CallerType>::func;

#define GETCB(ptrtype, callertype) Callback<ActualType<ptrtype>::type,__COUNTER__,callertype>::getCallback

// 此处必须用function类，typedef再后面函数指针赋值无效
//std::string, std::string 1=url 2=body
using ReqCallback = std::function<void(std::string, std::string)>;

class HttpClient
{
public:
	HttpClient();
	~HttpClient();

	void SyncSendReq(const std::string url, const std::string extra_headers,
		const std::string post_data, std::string &resp);

	void SendReq(const std::string url, const std::string extra_headers,
		const std::string post_data, ReqCallback req_callback);
	void OnHttpEvent(mg_connection *connection, int ev, void *http_req, void *fn_data);
	//static void OnHttpEvent(mg_connection *connection, int event_type, void *event_data);
	int s_exit_flag;
	ReqCallback s_req_callback;
	mg_event_handler_t fn;
	std::string s_post_data;
	std::string s_url;

	std::string s_resp;
};
//	static void HandleHttpEvent(mg_connection *connection, int ev, void *http_req, void *fn_data);
