
#include "isbuffer.h"

template <class ItemType, unsigned N > 
class SBuffer:public ISBuffer<ItemType,SBuffer<ItemType,N> >{
public:
    SBuffer();

	int enqueue(ItemType * pit);
	ItemType * dequeue();
	
    ItemType * allocItem();
    void freeItem(ItemType * pit);

    void stop();
};

template <class ItemType, unsigned int N >
SBuffer<ItemType,N>::SBuffer(){
}

template <class ItemType, unsigned int N >
int SBuffer<ItemType,N>::enqueue(ItemType * pit){
    return 0;
}

template <class ItemType, unsigned int N >
ItemType * SBuffer<ItemType,N>::dequeue(){
    return nullptr;
}

template <class ItemType, unsigned int N >
ItemType * SBuffer<ItemType,N>::allocItem(){
    return nullptr;
}

template <class ItemType, unsigned int N >
void SBuffer<ItemType,N>::freeItem(ItemType * pit){
}

template <class ItemType, unsigned int N >
void SBuffer<ItemType,N>::stop(){
}

