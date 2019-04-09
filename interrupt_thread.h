#pragma once
#include"interrupt_flag.h"
#include<thread>
#include<mutex>

//thread_local修饰
local_thread interrupt_flag this_local_flag;

interrupt_point(){
	if(this_local_flag.is_set())
		throw thread_interrupted();
}

void interrupt_wait(std::conditon_variable& cv,std::unique_lock<std::mutex> &lk){
	interrupt_point();
	this_local_flag.set_condition(cv);
	this_local_flag::clear_flag cf;
	interrupt_point();
	cv.wait_for(lk,time(1));
	interrupt_point();
}




class interrupt_thread{
	public:
		template<typename FunctionType>
		void interrupt_thread(FunctionType f){
			td=std::thread(
				[&f]{
					flag=&this_local_flag;
					f();		
				}
			);
		}
		void join(){
			td.join();
		}
		bool joinable(){
			return td.joinable();
		}
		void deatch(){
			td.deatch();
		}
		void interrupt(){
			if(flag)
				flag->set();
		}
	private:
		std::thread td;
		interrupt_flag* flag;
};



