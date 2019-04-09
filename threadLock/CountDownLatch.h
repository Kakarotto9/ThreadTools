#pragma once

#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<assert.h>

using namespace std;

//CountDownLatch这个类能够使一个线程等待其他线程完成各自的工作后再执行。例如，应用程序的主线程希望在负责启动框架服务的线程已经启动所有的框架服务之后再执行。
//CountDownLatch是通过一个计数器来实现的，计数器的初始值为线程的数量。每当一个线程完成了自己的任务后，计数器的值就会减1。当计数器值到达0时，它表示所有的线程已经完成了任务，然后在闭锁上等待的线程就可以恢复执行任务。

class CountDownLatch{
	public:
	explicit CountDownLatch(int n):count(n),mutex_(),condition_(){
		assert(count>0);
	}
	

	void countDown(){
		std::lock_guard<std::mutex> lock(mutex_);
		count--;
		if(0==count)
			condition_.notify_all();
	}
	void wait(){
		std::lock_guard<std::mutex> lock(mutex_);
		condition_.wait(mutex_,[this]{return count>0;});
	}

	private:
	int count;
	std::mutex mutex_;
	std::condition_variable_any condition_;
};
