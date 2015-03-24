#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "isbuffer.h"

template <class ItemType, unsigned N > 
class SBuffer:public ISBuffer<ItemType, SBuffer<ItemType,N>>{
public:
	SBuffer();
	int enqueue(ItemType * pit);
	ItemType * dequeue();
	ItemType * allocItem();
	void freeItem(ItemType * pit);
	void stop();
private:
	std::queue< std::unique_ptr<ItemType> > queue, item_pool;
	bool operating;
	std::condition_variable cv_alloc, cv_queue;
	std::mutex busy, alloc;
};

template <class ItemType, unsigned int N >
SBuffer<ItemType,N>::SBuffer(){
	for (unsigned i=0; i<N; ++i) 
		item_pool.push(std::move(std::unique_ptr<ItemType>(new ItemType)));
	operating = true;	
}

template <class ItemType, unsigned int N >
int SBuffer<ItemType,N>::enqueue(ItemType * pit){
	if (operating) {
		std::unique_lock<std::mutex> tmp(busy);
		queue.push(std::move(std::unique_ptr<ItemType>(pit)));
		cv_queue.notify_all();
		return 1;
	} else
		return 0;
}

template <class ItemType, unsigned int N >
ItemType * SBuffer<ItemType,N>::dequeue(){
	std::unique_lock<std::mutex> tmp(busy);
	cv_queue.wait(tmp, [&]{ return !queue.empty() || !operating; });
	if (queue.empty())
		return nullptr;			
	ItemType * pit = queue.front().release();
	queue.pop();
	return pit;
}

template <class ItemType, unsigned int N >
ItemType * SBuffer<ItemType,N>::allocItem(){
	std::unique_lock<std::mutex> tmp(alloc);
	cv_alloc.wait(tmp, [&]{ return !item_pool.empty() || !operating; });
	if (!operating)
		return nullptr;
	ItemType * pit = item_pool.front().release();
	item_pool.pop();
	return pit;
}

template <class ItemType, unsigned int N >
void SBuffer<ItemType,N>::freeItem(ItemType * pit){
	std::unique_lock<std::mutex> tmp(alloc);
	item_pool.push(std::move(std::unique_ptr<ItemType>(pit)));
	cv_alloc.notify_all();
}

template <class ItemType, unsigned int N >
void SBuffer<ItemType,N>::stop(){
	operating = false;
	cv_alloc.notify_all();
	cv_queue.notify_all();
}
