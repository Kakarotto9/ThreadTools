/*************************************************************************
	> File Name: ReadWriteMutex.h
	> Author: lihengfeng
	> Mail: 17704623602@163.com 
	> Created Time: Thu Nov 30 16:15:11 2017
 ************************************************************************/

#include<mutex>
#include<condition_variable>

using std::mutex;
using std::lock_guard;
class ReadWriteMutex{
public:
	ReadWriteMutex():readingCount(0){
	}
	//如果写线程占用资源，等待。
	//如果读线程占用，不影响。
	void readLock(){
		std::unique_lock<mutex> lock(mutex_);
		while(writingCount>0)
			cond.wait(lock);
		++readingCount;
	}
	//唤醒一个写操作
	void readUnLock(){
		std::lock_guard<mutex> lock(mutex_);
		--readingCount;
		if(readingCount==0)
			cond.notify_one();
	}
	//如果已经有读 或 写线程占用资源， 就等待。
	void writeLock(){
		std::unique_lock<mutex> lock(mutex_);
		while(readingCount>0|| writingCount>0)
			cond.wait(lock);  //
		++writingCount;
	}
	//唤醒所有读和写操作
	void writeUnLock(){
		std::lock_guard<mutex> lock(mutex_);
		//writingCount--;

		writingCount=0;//这里直接将writingCount=0，因为同时只有一个写线程获得资源，如果允许多个写线程同时访问资源，这里需要改为 writingCount--;
		cond.notify_all();
	}

private:
	std::mutex mutex_;
	std::condition_variable cond;  

	int readingCount=0; //正在占用资源的读线程的数量 
	int writingCount=0;   //正在占用资源的写线程数量
};

//下面readWriteMutex 只使用一个int变量- readingCount，代码简洁 但不是很直观，反而以上实现变量关系很明显 
/*
class ReadWriteMutex{
public:
	ReadWriteMutex():readingCount(0){
	}
	void readLock(){
		std::unique_lock<mutex> lock(mutex_);
		while(readingCount<0)
			cond.wait(lock);
		++readingCount;
	}
	void readUnLock(){
		std::lock_guard<mutex> lock(mutex_);
		--readingCount;
		if(readingCount==0)
			cond.notify_one();//唤醒一个写操作
	}
	void writeLock(){
		std::unique_lock<mutex> lock(mutex_);
		while(readingCount=0)
			cond.wait(lock);  //
		--readingCount;
	}
	void writeUnLock(){
		std::lock_guard<mutex> lock(mutex_);
		//readingCount++;

		readingCount=0;//这里直接将readCount=0，可以让read和write都能有机会停止等待，
		cond.notify_all();//唤醒所有读和写操作
	}

private:
	std::mutex mutex_;
	std::condition_variable cond;  
	//=0.说明无线程使用， 
	//>0.代表有readingCount个read线程同时使用，加了读锁 
	//<0.已经write线程正在运行，加了写锁
	int readingCount=0;  
};
*/
