#pragma once

#include<memory>
#include<atomic>

//使用shared_ptr来记录的节点引用次数 
//使用该种lock_free_stack的前提是平台支持 std::atomic_is_lock_free(&some_shared_ptr)的 实现返回true
//atomic_is_lock_free返回true,说明shared_ptr的原子操作是无锁的

template<typename T>
class lock_free_stack
{
	private:
		struct node
		{
			std::shared_ptr<T> data;
			std::shared_ptr<node> next;
			node(T const& data_):
				data(std::make_shared<T>(data_))
			{}

		};
		std::shared_ptr<node> head;
	public:
		void push(T const& data)
		{
			std::shared_ptr<node> const new_node=std::make_shared<node>(data);
			new_node->next=head.load();
			while(!std::atomic_compare_exchange_weak(&head,
						&new_node->next,new_node));

		}
		std::shared_ptr<T> pop()
		{
			std::shared_ptr<node> old_head=std::atomic_load(&head);
			while(old_head && !std::atomic_compare_exchange_weak(&head,
						&old_head,old_head->next));
			return old_head ? old_head->data : std::shared_ptr<T>();

		}

};
