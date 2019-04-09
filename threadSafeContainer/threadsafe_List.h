class {
	struct node{
		T data;
		unique_ptr<node> next;
		mutex mutex_;
	}

	void push(T t){
		unique_ptr<node> new_node(new node(t));
		lock_guard<mutex> head_lock(head.mutex_);
		new_node->next=std::move(head.next);
		head.next=std::move(new_node);
	}
	template<typename FunctionType>
	void remove_if(FunctionType f){
		node* current_ptr(&head);
		unique_lock<mutex> current_lock(current_ptr->mutex_);
		while(unique_ptr<node> next_node=std::move(current_ptr->next)){
			unique_lock<mutex> next_lock(next_node->mutex_);
			if(f(next_node)){
				unique_ptr<node> old_node=std::move(next_node);
				current_node->next=std::move(old_node->next);
			}
			else{
				current_ptr=next_node.get();
				current_lock=std::move(next_lock);
			} 
		}
	}
	private: 
	node head;
}
