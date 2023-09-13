/*****************************************************************************
* projectName: tracking sdk													 *
* file  rwLock.h															 *
* brief ¶ÁÐ´Ëø 																 *
* date: 2019/11/11 13:00													 *
* copyright(c) 2019 mhc													 *
*****************************************************************************/
#pragma once

#include <mutex>
#include <shared_mutex>
#include <condition_variable>
/**
* @brief ¶ÁÐ´ËøÀà
*/
class rwLock
{
public:
	void getReadLock()
	{
		std::unique_lock<std::mutex> lock(m);
		while (writerUsed == true)
		{
			cv.wait(lock);
			//printf("%p getReadLock \n", this);
		}
		readerCount++;
	}

	void getWriteLock()
	{
		std::unique_lock<std::mutex> lock(m);
		while (readerCount != 0 || writerUsed == true)
		{
			cv.wait(lock);
			//printf("%p getWriteLock \n", this);
		}
		writerUsed = true;
	}

	void unlockReader()
	{
		std::unique_lock<std::mutex> lock(m);
		readerCount--;
		if (readerCount <= 0)
		{
			readerCount = 0;
			cv.notify_all();
			//printf("%p unlockReader \n", this);
		}
	}

	void unlockWriter()
	{
		std::unique_lock<std::mutex> lock(m);
		writerUsed = false;
		cv.notify_all();
		//printf("%p unlockWriter \n", this);
	}

private:
	int readerCount = 0;
	volatile bool writerUsed = false;
	std::mutex m;
	std::condition_variable cv;
};
