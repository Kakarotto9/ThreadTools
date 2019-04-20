#pragma once

#include<iostream>
#include<mutex>

/*
 * 利用两个互斥量实现读写锁
 * 其中mutex_ 保护readCount数据，
 * writeMutex_ 起互斥读写的作用
*/

class ReadWriteMutex{
public:
	ReadWriteMutex():readCount(0){
	}
	ReadWriteMutex(const ReadWriteMutex& rhs)=delete;
	ReadWriteMutex& operator=(const ReadWriteMutex& rhs)=delete;

	void readLock(){
		std::lock_guard<std::mutex> lock(mutex_); //当第一个read线程在writeLock被阻塞时，mutex_没有被释放，就会阻塞其他的read线程，保证只有一个read线程涉及writeMutex
		if(++readCount==1)
			writeMutex_.lock();
	}
	void readUnlock(){
		std::lock_guard<std::mutex> lock(mutex_);
		if(--readCount==0)
			writeMutex_.unlock();
	}
	void writeLock(){
		writeMutex_.lock();
	}
	void wirteUnlock(){
		writeMutex_.unlock();
	}
private:
	std::mutex mutex_;
	std::mutex writeMutex_;
	int readCount;
};
