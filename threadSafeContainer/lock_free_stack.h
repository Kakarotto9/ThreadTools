#pragma once

#include<atomic>
#include<memory>

using std::shared_ptr;

tempalte<typename T>
class lock_free_stack{
	struct node{
		shared_ptr<T> data;
		/*
		   unique_ptr<node> next;
		   */
		node* next;
		node(T& value):data(make_shared<T>(value)){

		}
	}

	void push(T& n){
		node* new=new node(n);
		new->next=head.load();
		/*
		   mutex;
		   while(true){
		   if(head.load()==new->next){
		   head=new;
		   break;
		   }
		   else{
		   new->next=head.load();
		   }
		   }*/
		while(head.exchange_and_weak(new->next,new));
	}
	shared_ptr<T> pop(){
		node* old=head.load();
		/*
		   mutex;
		   while(true){
		   if(head==old){
		   head=old->next;
		   break;
		   }
		   else 
		   old=head.load();
		   }*/
		while(old&&  //这里用old，而不用head.load(),是为了符合pop调用的即时性
				head.exchange_and_weak(old,old->next));
		//shared_ptr<T> res(old->data); 这样写是错的，当old是null时，调用null指针错误
		shared_ptr<T> res;
		if(old){
			res=old->data;
			delete old;
		}
		return res;
	}
};
