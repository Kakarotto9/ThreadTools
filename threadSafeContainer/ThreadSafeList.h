#pragma once

#include<stdio.h>
#include<memory>
#include<mutex>

using std::unique_ptr;
using std::shared_ptr;
using std::unique_lock;
using std::lock_guard;
using std::mutex;

template<typename T>
class LinkList{
	struct node{
		mutex mutex_;
		shared_ptr<T> data;
		unique_ptr<node> next;
		node():data(),next(),mutex_(){
		}
		node(const T& t):data(std::make_shared<T>(t)),next(),mutex_(){
		}
	};

	node head;

	public:
	void push(const T& t){
		unique_ptr<node> new_node(new node(t));
		lock_guard<mutex> head_lock(head.mutex_);
		new_node->next=std::move(head.next);
		head.next=std::move(new_node);
	}
	template<typename FunctionType>
		void remove_if(FunctionType f){
			node* current_ptr(&head);
			unique_lock<mutex> current_lock(current_ptr->mutex_);
			while(node* next_ptr=current_ptr->next.get()){
				unique_lock<mutex> next_lock(next_ptr->mutex_);
				if(f(*next_ptr->data)){
					unique_ptr<node> old_node=std::move(current_ptr->next);
					current_ptr->next=std::move(old_node->next);
					next_lock.unlock(); //old_node离开if语句后就会析构，从而让old_node.mutex也被释放了,所以为了避免此时next_lock释放空Mutex,需要主动将next_lock释放
				}
				else{
					current_lock.unlock();
					current_lock=std::move(next_lock); 
					current_ptr=next_ptr;
				} 
			}
		}
	template<typename FunctionType>
		void for_each(FunctionType f){
			node* current_ptr(&head);
			unique_lock<mutex> current_lock(current_ptr->mutex_);
			while(node* next_ptr=current_ptr->next.get()){
				unique_lock<mutex> next_lock(next_ptr->mutex_); //必须在current_lock.unlock的前边
				current_lock.unlock();//当next_lock枷锁后，就至少保证next_ptr节点不被删除,所以释放current_lock就可以
				f(*next_ptr->data);
				current_ptr=next_ptr;
				current_lock=std::move(next_lock);
			}
		}
	template<typename FunctionType>
		shared_ptr<T> find_first_if(FunctionType f){
			node* current_ptr(&head);
			unique_lock<mutex> current_lock(current_ptr->mutex_);
			while(node* next_ptr=current_ptr->next.get()){
				unique_lock<mutex> next_lock(next_ptr->mutex_); 
				current_lock.unlock();
				if(f(*next_ptr->data)){
					return next_ptr->data;
				}
				current_ptr=next_ptr;
				current_lock=std::move(next_lock);
			}
			return shared_ptr<T> ();
		}
};
