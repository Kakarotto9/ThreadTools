#pragma once

#include<cstdio>
#include<memory>
#include<mutex>
#include<condition_variable>
#include<exception>

using std::mutex;
using std::shared_ptr;
using std::unique_ptr;
using std::unique_lock;
using std::lock_guard;
using std::make_shared;

/*
 * 1 pop和push操作的不是同一个节点，提高并发度
 * 2 创建虚拟节点
 * 3 node 中 使用 shared_ptr<T> 保存 值,一方面减少了pop中的内存分配，另一方面将内存不够导致的异常放入push中，
 */ 

template <typename T>
class threadsafe_queue{
	private:
		struct node{
			shared_ptr<T> data;
			unique_ptr<node> next;
		};
		std::mutex head_mutex;  
		std::mutex tail_mutex;
		std::condition_variable cond;
		unique_ptr<node> head; //使用 unique_ptr,不需要在析构函数主动释放
		node* tail;

		node* get_tail(){// 配合pop使用的,可以把该函数放到if（）语句内，从而达到缩小临界区的目的
			std::lock_guard<mutex> tail_lock(tail_mutex);
			return tail;
		}
		unique_ptr<node> pop_head(){
			unique_ptr<node> old_node=std::move(head);
			head=std::move(old_node->next);
			return std::move(old_node);
		}
		unique_lock<mutex> wait_for_data(){
			std::unique_lock<mutex> head_lock(head_mutex);
			cond.wait(head_lock,[this]{return head.get()!=get_tail();}); //注意，这里wait等待的是head_mutex，而不是tail_mutex
			return std::move(head_lock);
		}
		unique_ptr<node> try_pop_head(){
			/* 如果将get_tai()放置到前边，会出现数据竞争，极端情况下会导致获取的old_tail所指对象被pop中移除掉
			node* old_tail=get_tail()
			std::lock_guard<mutex> head_lock(head_mutex);
			 */
			std::lock_guard<mutex> head_lock(head_mutex);
			if(head.get()==get_tail())
				return unique_ptr<node> ();
			return pop_head();
		}
		unique_ptr<node> try_pop_head(T& value){
			std::lock_guard<mutex> head_lock(head_mutex);
			if(head.get()==get_tail())
			{
				return unique_ptr<node> ();
			}
			value=std::move(*head->data);  // 需要放到pop_head()前面，这是为了异常安全考虑
			return pop_head();
		}
		unique_ptr<node> wait_pop_head(){
			std::unique_lock<mutex> head_lock (wait_for_data());
			return pop_head();
		}
		void wait_pop_head(T& value){
			std::unique_lock<mutex> head_lock(wait_for_data());
			value=std::move(*head->data); //pop head()放后面
			pop_head();
		}

	public:
		threadsafe_queue():head(new node),tail(head.get()){};
		threadsafe_queue(const threadsafe_queue& other)=delete;
		threadsafe_queue& operator=(const threadsafe_queue& other)=delete;

		void push(T n){
			shared_ptr<T> new_data(make_shared<T>(std::move(n)));
			unique_ptr<node> p(new node); 
			{
				std::lock_guard<mutex> tail_lock(tail_mutex);
				node* new_tail=p.get();
				tail->data=new_data;
				tail->next=std::move(p);  // tail->next接管p后会释放原来指向的元素，但原本指向一个空Node，所以无碍
				//tail=get();
				tail=new_tail;
			}
			cond.notify_one();
		}
		void wait_pop(T& value){
			wait_pop_head(value);
		}
		shared_ptr<T> wait_pop(){
			unique_ptr<node> old_node=wait_pop_head();
			return old_node?old_node->data:shared_ptr<T>();
		}
		bool try_pop(T& value){
			unique_ptr<node> old_node=try_pop_head(value);
			return old_node? true:false;
		}
		shared_ptr<T> try_pop(){
			unique_ptr<node> old_node=try_pop_head();
			return old_node?old_node->data:shared_ptr<T>();
		}

		bool empty() const{
			std::lock_guard<mutex> head_lock(head_mutex);
			return head.get()==get_tail();
		}
};
