
//上一个版本，存在线程间内存泄露问题，原因当某个线程删除a节点后，可能其他线程正在使用这个节点，导致错误
//


#pragma once

#include<atomic>
#include<memory>

using std::shared_ptr;

tempalte<typename T>
class lock_free_stack{
	struct node{
		shared_ptr<T> data;
		shared_ptr<node> next;
		node(T& value):data(make_shared<T>(value)){

		}
	};
	
	public:
	void push(T& n){
		shared_ptr<node> new(make_shared<node>(n));
		new->next=head.load();
		while(std::atomic_compare_exchange_weak(&head,&new->next,new);
	}
	shared_ptr<T> pop(){
		shared_ptr<node> old=head.load();
		while(old&&  //这里用old，而不用head.load(),是为了符合pop调用的即时性
				head.exchange_and_weak(old,old->next));
		return old?old-data:shared_ptr<T>() ;
	}
};
