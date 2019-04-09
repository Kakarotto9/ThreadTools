#include<cstdio>
#include<memory>
#include<mutex>
#include<exception>

class threadsafe_lookup_talbe{
	private:
	vecctor<bucket> buckets;
	std::hash<key> hasher;
	bucket get_bucket(Key key){
		int n=hasher(key)%18;
		return bucket[n];
	}
	public:
	Value get_value(Key );
	void add_or_update(Key,Value );
	void remove(Value );
}
