//难点在于理解node.internal_count的意义和注释6.7所在的语句
//当internal_count>0,表示有多少线程正在使用该节点
//当internal_count<0时，表示有多少原本使用该节点的线程不再使用该节点了
//然后结合 注释6.7语句就能保证节点释放的安全性了
//
#pragma once

#include<memory>
#include<atomic>

using std::shared_ptr;

template<typename T>
class lock_free_stack
{
	private:
		struct node;  //前置声明
		struct counted_node_ptr 
		{
			int external_count;  //不需要设置为atomic
			node* ptr;
		};
		struct node
		{
			std::shared_ptr<T> data;
			std::atomic<int> internal_count; // 2
			counted_node_ptr next; 
			node(T const& data_):
				data(std::make_shared<T>(data_)),
				internal_count(0)
			{}
		};
		void increase_head_count(counted_node_ptr & old){
			// counted_node_ptr new;我本来想用 new来简洁表达，但忘了new是 关键字了
			counted_node_ptr new_node;
			do{
				new_node=old;
				++new_node.counted_node_ptr;
			}while(head.compare_exchange_weak(old,new_node));  //4  和注释6 相互作用
			old.external_count=new_node.external_count;
		}

		std::atomic<counted_node_ptr> head; 

	public:
		~lock_free_stack()//需要主动释放全部
		{
			while(pop());  
		}
		void push(T const& data)
		{
			counted_node_ptr new_node;
			new_node.ptr=new node(data);
			new_node.external_count=1;
			new_node.ptr->next=head.load();
			while(!head.compare_exchange_weak(new_node.ptr->next,new_node));
		}
		shared_ptr<T> pop(){
			counted_node_ptr old_node=head.load();
			for(;;){  //因为注释7所在语句执行时，会导致没有数据抛出，所以需要循环到数据抛出为止
				increase_head_count(old_node);
				node* ptr=old_node.ptr;
				if(old_node){
					if(head.compare_exchange_weak(old_node,ptr->next)){ // 让最后一个指向该节点的线程 进行更新(也就统计了总共有多少线程指向该节点)
						shared_ptr<T> res;
						res.swap(ptr->data);
						int count=old_node.external_count-2;//统计除了自己之外，还有多少线程曾使用该节点
						if(ptr.internal_count.fetch_add(count)==-count){ // 6 与注释7结合使用
							delete ptr;
						}
						return res;
					}
					else if(ptr->internal_count.fetch_sub(1)==1){ //7 internal_count>0时，表示有多少线程调用节点， <0时表示 有多少原来使用该节点的线程不再使用
						delete ptr;
					}
				}
				else {
					return shared_ptr<T> ();
				}
			}
		}

};

