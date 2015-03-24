#ifndef _ISBUFFER_H
#define _ISBUFFER_H

template <class ItemType,class T > 
class ISBuffer{

public:
    ISBuffer(){};    

	int enqueue(ItemType * pit){(static_cast<T*>(this))->enqueue(pit);};
	ItemType * dequeue(){(static_cast<T*>(this))->dequeue();};

	void stop(){(static_cast<T*>(this))->stop();};
	
    ItemType * allocItem(){(static_cast<T*>(this))->allocItem();};
    void freeItem(ItemType * pit){(static_cast<T*>(this))->freeItem(pit);};
};
#endif
