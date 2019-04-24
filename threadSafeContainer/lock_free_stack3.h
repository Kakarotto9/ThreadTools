#pragma once

#include<memory>
#include<atomic>
using std::shared_ptr;

//难点在于理解node.internal_count的意义和注释6.7所在的语句
//当internal_count>0,表示有多少线程正在使用该节点
//当internal_count<0时，表示有多少原本使用该节点的线程不再使用该节点了
//然后结合 注释6.7语句就能保证节点释放的安全性了

template<typename T>
class lock_free_stack
{
	private:
		struct node;  
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
			counted_node_ptr new_node;
			do{
				new_node=old;
				++new_node.counted_node_ptr;
			}while(head.compare_exchange_weak(old,new_node,
						std::memory_order_acquire,std::memory_order_relaxed	));  //4  和注释6 相互作用
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
			new_node.ptr->next=head.load(std::memory_order_relaxed);
			while(!head.compare_exchange_weak(new_node.ptr->next,new_node,std::memory_order_release,std::memory_order_relaxed));
		}
		shared_ptr<T> pop(){
			counted_node_ptr old_node=head.load(std::memory_order_relaxed);
			for(;;){  //因为注释7所在语句执行时，会导致没有数据抛出，所以需要循环到数据抛出为止
				increase_head_count(old_node);
				node* ptr=old_node.ptr;
				if(old_node){
					if(head.compare_exchange_weak(old_node,ptr->next,std::memory_order_relaxed)){ // 让最后一个指向该节点的线程 进行更新(也就统计了总共有多少线程指向该节点)
						shared_ptr<T> res;
						res.swap(ptr->data);  //5
						int count=old_node.external_count-2;//统计除了自己之外，还有多少线程曾使用该节点
						if(ptr.internal_count.fetch_add(count,
								std::memory_order_release)==-count){ // 6 与注释7结合使用 ,通过release-acquire,保证注释8中的ptr->data获取到 res.swap()之后的值
							delete ptr;
						}
						return res;

					}
					else if(ptr->internal_count.fetch_sub(1,std::memory_order_relaxed)==1){ //7 internal_count>0时，表示有多少线程调用节点， <0时表示 有多少原来使用该节点的线程不再使用
						ptr->internal_count.load(std::memory_order_acquire);  //8 是保证ptr->data能够获取到最新的值，避免delete将注释5的res中指向的地址删除掉
						delete ptr;
					}
				}
				else {
					return shared_ptr<T> ();
				}
			}
		}

};

