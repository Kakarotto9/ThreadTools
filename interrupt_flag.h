#pragma once

#include<atomic>
#include<mutex>
#include<conditon_variable>

class interrupt_flag{
	std::atomic<bool> done;
	std::conditon_variable cond;
	std::mutex cond_mutex;

	public:
	void set(){
		done.store(true);
		std::lock_guard<std::mutex> cond_lock(cond_mutex);
		if(cond)
			cond.notify_all();
	}
	bool is_set(){
		return done;
	}
	void set_condition(){
		std::lock_guard<std::mutex> cond_lock(cond_mutex);

	}
	void clear_condtion(){
		std::lock_guard<std::mutex> cond_lock(cond_mutex);
		cond=0;
	}

	struct clear_flag{
		~clear_flag(){
			this_local_flag.clear_condtion();
		}
	}
};
