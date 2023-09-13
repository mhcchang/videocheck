/*****************************************************************************
* projectName: tracking sdk													 *
* file  log4j.h																 *
* brief ����SDK ֡������д���													 *
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
* @brief ֡������д�����
*	��sdk������setframeʱ �ⲿ�ṩ��֡���ݵ���Ϣѹ�뻺�� 
*	���ٴ����߳�ʹ��popObj��ȡ��Ϣ������ ����Ϊ�Ƚ��ȳ�����
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
	* @brief ���ö������ֵ 
	* @param nCount ������ֵ����0��С�ڵ���C_MAXCAMERACOUNT(40) ���óɹ� ����ΪȱʡֵC_MAXCAMERACOUNT
	* @return 
	* @throws ��
	*/
	int setMaxCount(int nCount)
	{
		if (nCount > 0 && nCount <= C_MAX_OBJ_COUNT)
			m_nCount = nCount;
		return m_nCount;
	}

	/**
	* @brief ֡����ѹ�����
	* @param obj ֡����
	* @return ��
	* @throws ��
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
	* @brief ֡���ݵ������� �Ƚ��ȳ�
	* @param ��
	* @return ֡����
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
	* @brief ֡���ݵ������� ��ͷ��ʼ ��ȡ���֡���� ��ָ������
	* @param nCount ���������ڶ�������ʱ���ص��б�Ϊ����ʵ������ �������Ĳ���Ϊ��ֵ��ȡ���� ��ʾ����ȫ������
	* @return ֡�б� 
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
	//���ݶ���
	std::vector<T> m_lsObjs;
	//���ж�д��/
	rwLock* m_pRwInstance;
	//�����������
	volatile int m_nCount;
};
