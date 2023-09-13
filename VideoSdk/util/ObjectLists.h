/*****************************************************************************
* projectName: tracking sdk													 *
* file  log4j.h																 *
* brief 跟踪SDK 帧缓存队列处理													 *
* date: 2019/11/11 13:00													 *
* copyright(c) 2019 mhchang													 *
*****************************************************************************/
#pragma once

#include "pubutil.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <iostream>

#define C_MAX_OBJ_COUNT 512

using namespace std;

class rwLock
{
public:
	void getReadLock()
	{
		std::unique_lock<std::mutex> lock(m);
		while (writerUsed == true)
		{
			cv.wait(lock);
		}
		readerCount++;
	}
	void getWriteLock()
	{
		std::unique_lock<std::mutex> lock(m);
		while (readerCount != 0 || writerUsed == true)
		{
			cv.wait(lock);
		}
		writerUsed = true;
	}
	void unlockReader()
	{
		std::unique_lock<std::mutex> lock(m);
		readerCount--;
		if (readerCount == 0)
		{
			cv.notify_all();
		}
	}
	void unlockWriter()
	{
		std::unique_lock<std::mutex> lock(m);
		writerUsed = false;
		cv.notify_all();
	}

private:
	int readerCount = 0;
	bool writerUsed = false;
	std::mutex m;
	std::condition_variable cv;
};

template<typename T>
/**
* @brief 帧缓存队列处理类
*	当sdk被调用setframe时 外部提供的帧数据等信息压入缓存 
*	跟踪处理线程使用popObj获取信息并处理 队列为先进先出对列
*/
class ObjectLists
{
public:
	ObjectLists() 
	{
		m_nCount = C_MAX_OBJ_COUNT;
		m_pRwInstance = new rwLock;
	}
	~ObjectLists() 
	{
		delete m_pRwInstance;
	};

	/**
	* @brief 设置队列最大值 
	* @param nCount 当设置值大于0且小于等于C_MAXCAMERACOUNT(40) 设置成功 否则为缺省值C_MAXCAMERACOUNT
	* @return 
	* @throws 无
	*/
	int setMaxCount(int nCount)
	{
		if (nCount > 0 && nCount <= C_MAX_OBJ_COUNT)
			m_nCount = nCount;
		return m_nCount;
	}

	/**
	* @brief 帧数据压入队列
	* @param obj 帧数据
	* @return 无
	* @throws 无
	*/
	void pushObj(T obj)
	{
		m_pRwInstance->getWriteLock();
		m_lsObjs.push_back(obj);

		//printf("pushObj m_lsObjs.size=%zd \r\n", m_lsObjs.size());
		if (m_lsObjs.size() > m_nCount)
		{
			obj = m_lsObjs[0];
			m_lsObjs.erase(m_lsObjs.begin());
		}

		m_pRwInstance->unlockWriter();
	}

	/**
	* @brief 帧数据弹出队列 先进先出
	* @param 无
	* @return 帧数据
	*/
	bool popObj(T &obj)
	{
		m_pRwInstance->getWriteLock();
		//printf("popObj m_lsObjs.size=%zd \r\n", m_lsObjs.size());
		if (m_lsObjs.size() > 0)
		{
			obj = m_lsObjs[0];
			m_lsObjs.erase(m_lsObjs.begin());

			m_pRwInstance->unlockWriter();
			return true;
		}
		m_pRwInstance->unlockWriter();
		return false;
	}

	/**
	* @brief 帧数据弹出队列 从头开始 获取多个帧数据 可指定数量
	* @param nCount 当参数大于队列总数时返回的列表为队列实际数量 如果传入的参数为负值获取不传 表示弹出全部数据
	* @return 帧列表 
	*/
	std::vector<T> popObjs(int nCount = -1)
	{
		std::vector<T> objs;
		
		m_pRwInstance->getWriteLock();
		int ic = 0;
		if (nCount < 0)
			ic = m_lsObjs.size();
		else
			ic = __min(m_lsObjs.size(), nCount);
		for (int i = 0; i < ic; i++)
		{
			T obj = m_lsObjs[0];
			m_lsObjs.erase(m_lsObjs.begin());
			objs.push_back(obj);
		}
		m_pRwInstance->unlockWriter();

		//std::cout << " popObjs" << std::endl;
		return objs;
	}

	int size() { return m_nCount; };
protected:
	//数据队列
	std::vector<T> m_lsObjs;
	//队列读写锁/
	rwLock* m_pRwInstance;
	//队列最大数量
	volatile int m_nCount;
};
