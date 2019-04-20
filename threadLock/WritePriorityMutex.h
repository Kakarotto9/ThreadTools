//这个版本是为了更容易理解而写，其实可以将readRunningCount和 writeRuningCoun合成一个变量

#pragma once

#include<iostream>
#include<mutex>
#include<condition_variable>


class WritePriorityMutex{
public:
	WritePriorityMutex():readRunningCount(0),writeRuningCount(0),writeWaitCount(0){
	}
	WritePriorityMutex(const WritePriorityMutex& rhs)=delete;
	WritePriorityMutex& operator=(const WritePriorityMutex& rhs)=delete;

	WritePriorityMutex(WritePriorityMutex&& rhs)=delete;
	WritePriorityMutex& operator=(WritePriorityMutex&& rhs)=delete;

	//当有写线程占用资源时，等待，如果都是读，那么同时运行
	void readLock(){
		std::unique_lock<std::mutex> ul(mutex_);
		
		//需要注意writeWait>0时不一定会阻塞，只有当readRuning=0且writeWait>0时才阻塞，所以需要对readRuning进行判断
		while(writeRuningCount>0||(readRunningCount==0&&writeWaitCount>0)){
			cond.wait(ul);
			if(writeRuningCount<=0) //当writeRuning>0时是说明写线程正在占用时，此时读线程是被伪唤醒，不需要唤醒其他线程
				cond.notify_one();
		}
		readRunningCount++;
	}
	//当readRunningCount=0,唤醒等待的写线程
	void readUnlock(){
		std::lock_guard<std::mutex> lock(mutex_);
		--readRunningCount;
		if(readRunningCount==0)
			cond.notify_one();
		//如果允许多个写线程同时进行，上句改为下句
		//	cond.notify_all();
	}
	//当有读线程占用资源时，写等待
	//如果设置写线程间互斥，那么如果有其他写线程占用资源，该写线程等待
	void writeLock(){
		std::unique_lock<std::mutex> ul(mutex_);
		++writeWaitCount;
		while(readRunningCount>0||writeRuningCount>0)
			cond.wait(ul);
		++writeRuningCount;
		--writeWaitCount;
	}
	//当没有写线程等待时，执行notify_all()，如果有写线程等待，为了避免大量读线程的频繁唤醒又沉睡。使用notify_one();
	void writeUnlock(){
		std::lock_guard<std::mutex> lock(mutex_);
		writeRuningCount=0;
		if(writeWaitCount>0)
			cond.notify_one();
		else
			cond.notify_all();
		// writeRuningCount--;
	}
private:
	std::mutex mutex_;
	std::condition_variable cond;

	int readRunningCount;  // <0：写线程正在运行 =0 无线程运行  >0 正在运行的读线程的数量
	int writeRuningCount;  // =0 代表没有写线程运行 >0代表有 
	int writeWaitCount;  //等待写的线程的数量  the count of thread to wait for write

};
