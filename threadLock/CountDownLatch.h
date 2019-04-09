/*************************************************************************
	> File Name: CountDownLatch.h
	> Author: ma6174
	> Mail: 17704623602@163.com 
	> Created Time: Sat Sep  2 00:03:33 2017
************************************************************************/

#pragma once

#include<iostream>

#include<thread>
#include<mutex>
#include<condition_variable>

using namespace std;

class CountDownLatch{
	public:
	explicit CountDownLatch(int n):count(n),mutex_(),condition_(){

	}
	

	void countDown(){
		std::lock_guard<std::mutex> lock(mutex_);
		count--;
		if(0==count)
			condition_.notify_all();
	}

	void wait(){
		std::lock_guard<std::mutex> lock(mutex_);
		while(count>0)
		condition_.wait(mutex_);
	}


	private:
	int count;
	std::mutex mutex_;
	std::condition_variable_any condition_;
};
