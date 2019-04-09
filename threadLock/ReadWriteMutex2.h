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
		std::lock_guard<std::mutex> lock(mutex_);
		while(++readCount==1)
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
