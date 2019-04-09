/*************************************************************************
  > File Name: cengciMutex.h
  > Author: ma6174
  > Mail: 17704623602@163.com 
  > Created Time: Mon Nov 27 19:43:16 2017
 ************************************************************************/
#include<unistd.h>

#include<iostream>
#include<vector>
#include<mutex>
#include<stdexcept>
#include<climits>


//levelMutex实现方法，要能使用lock_guard对levelMutex进行管理必须实现lock/unlock/try_lock方法；
//为了实现层次间的比较进而决定能不能加锁:
//(1)需要记录准备加锁mutex的层号、
//(2)该线程中当前已经加锁的mutex的层号、
//(3)在解锁时恢复原先现场就要记录先前mutex的层号，
//综上levelMutex定义如下：

class LevelMutex{
	public:
		explicit LevelMutex(int num):level(num),preLevel(0){  //静态变量不能在构造函数中初始化，所以不能在这初始化curlevel;
		}
		//满足规则时加锁并且更新内部变量
		void lock(){
			checkLevel();
			mutex_.lock();
			preLevel=curLevel;
			curLevel=level;
		}
		//恢复到调用lock之前的现场，解锁
		void unlock(){
			curLevel=preLevel;
			mutex_.unlock();
		}
		bool try_lock(){
			checkLevel();
			if(!mutex_.try_lock())
				return false;
			preLevel=curLevel;
			curLevel=level;
			return true;
		}

	private:
		void checkLevel(){//检查层次是否符合规范
			if (level>=curLevel){	
				throw std::logic_error("mutex level violated!");
			}
		}
		const int level;   //准备加锁的层次数
		int preLevel;   //解锁恢复现场时记录的层次数
		static thread_local int curLevel;   //代表正在锁住的层次数,这里使用了thread Local
		std::mutex mutex_;
};
//使用UNSIGND_MAX初始化表明刚开始任何层的mutex都可以加锁成功
thread_local int LevelMutex::curLevel(INT_MAX);
