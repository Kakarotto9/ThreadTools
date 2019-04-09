#include<iostream>
#include<thread>
#include<chrono>
#include"threadsafe_queue.h"


threadsafe_queue<int> queue_;

void fun(){
	std::cout<<"fun"<<std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	queue_.push(100);
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	queue_.push(200);
}
void fun2(){
	std::cout<<"fun2"<<std::endl;
	shared_ptr<int> sptr=queue_.wait_pop();
	std::cout<<"*sptr="<<*sptr<<std::endl;
}
int main(){
	std::thread t1(fun);
	std::thread t2(fun2);
	queue_.push(9);
	int n=0;
	queue_.try_pop(n);
	std::cout<<"n="<<n<<std::endl;
	bool f=queue_.try_pop(n);
	std::cout<<"f="<<f<<std::endl;
	queue_.wait_pop(n);
	std::cout<<"n2="<<n<<std::endl;
	t1.join();
	t2.join();
}
