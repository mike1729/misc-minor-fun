#include <thread>
#include <iostream>
#include <atomic>
#include <chrono>

#include "sbuffer.h"
#include "isbuffer.h"

#define ROUNDS (1<<18)
#define BSIZE (1<<10)

#define PRODUCERS 2
#define PRODUCE_DELAY 0
#define CONSUMERS 2
#define CONSUME_DELAY 0


struct Item {
	unsigned long data;
	unsigned long producer;
};

SBuffer<Item,BSIZE>  S1;
ISBuffer<Item,SBuffer<Item,BSIZE> > & S= S1;

int produce(Item & it, unsigned long who){
    std::chrono::milliseconds  dura( PRODUCE_DELAY );
    std::this_thread::sleep_for( dura );

    it.producer= who;
    it.data = who;
};

int consume(Item & it, unsigned long who){
    std::chrono::milliseconds  dura( PRODUCE_DELAY );
    std::this_thread::sleep_for( dura );
    //std::cout << who << " is consuming:"<<it.data<<" from:"<<it.producer << std::endl;
    return 0;
};

void writer(unsigned long nb){
	for (int i=0; i<  ROUNDS; i++){
		Item * pit= S.allocItem();
		if (pit==nullptr) break;
		produce(*pit,nb);
		S.enqueue(pit);
	}
    
	std::cout<<"writer "<< nb <<" done."<<std::endl;
}

unsigned long rd[CONSUMERS+1];

void reader(unsigned long nb){
	unsigned long counter=0;
	
	while (true){
		Item * pit = S.dequeue();
		if (pit==nullptr) break;
		consume(*pit,nb);
		S.freeItem(pit);
		counter++;
	}
    std::cout<<"reader "<<nb<<" done, count:"<<counter<<std::endl;
    rd[nb-1]=counter;
}

int main(){

    std::thread producers[PRODUCERS];
    for (int i=0; i< PRODUCERS; i++){
        producers[i]=std::thread(writer,i+1);
    }
    std::thread consumers[CONSUMERS];
    for (int i=0; i< CONSUMERS; i++){
        consumers[i]=std::thread(reader,i+1);
    }

    for (int i=0; i< PRODUCERS; i++){
        producers[i].join();
    }
    S.stop();
    unsigned long sum=0;
    for (int i=0; i< CONSUMERS; i++){
        consumers[i].join();
        sum+=rd[i];
    }

    std::cout<< "sum:"<<sum<<std::endl;
}
