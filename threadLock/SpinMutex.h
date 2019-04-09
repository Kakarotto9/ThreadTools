#pragma once
#include<atomic>

/*
   atomic<bool> 与CAS(compare and swap)的实现 
   1 .在指定memory order前需要明确自旋锁的责任:自旋锁除了要避免多线程重入，还要保证一个线程在持有自旋锁期间对内存的写操作要能够被另一个线程在获得自旋锁的时候可观测到(可见)。所以可以采用 relase-acquire的内存序，也就是在 store上使用relase,在compare上使用 acquire
   2 .自旋锁的所有权的充分必要条件是当且仅当compare_exchange_strong比较成功且成功改变flag的值为true，也就是说当比较失败时是没有必要执行acquire操作的。所以当compare失败时，使用消耗最小的 relaxed 内存序 

   class SpinMutex {
   std::atomic<bool> flag = ATOMIC_VAR_INIT(false);
   public:
   SpinMutex() = default;
   SpinMutex(const SpinMutex&) = delete;
   SpinMutex& operator= (const SpinMutex&) = delete;
   void lock() {
   bool expected = false;
   while(!flag.compare_exchange_strong(expected, true,std::memory_order_acquire, std::memory_order_relaxed))
   expected = false;
   }
   void unlock() {
   flag.store(false,std::memory_order_release);
   }
   };
*/

/*
atmoic<bool>与exchange 的实现版本

   class SpinMutex {
   std::atomic<bool> flag = ATOMIC_VAR_INIT(false);
   public:
   SpinMutex() = default;
   SpinMutex(const SpinMutex&) = delete;
   SpinMutex& operator= (const SpinMutex&) = delete;
   void lock() {
   while(flag.exchange(true, std::memory_order_acquire))
   ;

   }
   void unlock() {
   flag.store(false, std::memory_order_release);

   }

   };
*/

// atomic<T> 不能保证内部是无锁实现
// C++11提供了一个无锁的二值(bool)原子类型std::atomic_flag。使用std::atomic_flag就可以实现一个真正有用的自旋锁了。

class SpinMutex {
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
	public:
	SpinMutex() = default;
	SpinMutex(const SpinMutex&) = delete;
	SpinMutex& operator= (const SpinMutex&) = delete;
	void lock() {
		while(flag.test_and_set(std::memory_order_acquire))
			;
	}
	void unlock() {
		flag.clear(std::memory_order_release);

	}
};
