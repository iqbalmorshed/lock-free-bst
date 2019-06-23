/*
 * CBSTmain.cpp
 *
 *  Created on: Oct 17, 2018
 *      Author: iqbal
 */

#include "LockFreeBST.h"
#include <time.h>
#include <vector>
#include <thread>
#include <future>
#include <stdlib.h>
#include <iostream>
#include <functional>
#include <chrono>

//shared lock free bst
LockFreeBST *lfbst;

void sequentialOperations(LockFreeBST& lfbst){
	lfbst.add(6);
	lfbst.add(9);
	lfbst.add(10);
	lfbst.add(7);
	lfbst.add(8);
	lfbst.add(3);
	lfbst.add(5);
	lfbst.add(4);
	lfbst.add(2);
	lfbst.add(1);

	lfbst.remove(1);
	std::cout<<"contains 1:"<<lfbst.contains(1)<<'\n';
	std::cout<<"contains 6:"<<lfbst.contains(6)<<'\n';
	lfbst.add(1);
	std::cout<<"contains 1:"<<lfbst.contains(1)<<'\n';
	lfbst.remove(6);
	std::cout<<"contains 1:"<<lfbst.contains(6)<<'\n';
	lfbst.remove(8);
	std::cout<<"contains 1:"<<lfbst.contains(8)<<'\n';

	//	std::cout<<"contains 10:"<<lfbst.contains(10)<<'\n';
	//	std::cout<<"contains 20:"<<lfbst.contains(20)<<'\n';
	//	std::cout<<"contains 30:"<<lfbst.contains(30)<<'\n';
	//	std::cout<<"contains 32:"<<lfbst.contains(32)<<'\n';
	//	std::cout<<"contains 29:"<<lfbst.contains(29)<<'\n';
	//	std::cout<<"contains 25:"<<lfbst.contains(25)<<'\n';
	//	std::cout<<"contains 15:"<<lfbst.contains(15)<<'\n';
	//	std::cout<<"contains 5:"<<lfbst.contains(5)<<'\n';
	//	lfbst.remove(30);
	//	std::cout<<"contains 30:"<<lfbst.contains(30)<<'\n';
	//	std::cout<<"contains 25:"<<lfbst.contains(25)<<'\n';
	//	std::cout<<"contains 32:"<<lfbst.contains(32)<<'\n';
	//	lfbst.remove(32);
	//	std::cout<<"\ncontains 32:"<<lfbst.contains(32)<<'\n';
	//	lfbst.remove(5);
	//	std::cout<<"\ncontains 5:"<<lfbst.contains(5)<<'\n';
	//	lfbst.remove(20);
	//	std::cout<<"\ncontains 20:"<<lfbst.contains(20)<<'\n';
	//	std::cout<<"\ncontains 26:"<<lfbst.contains(26)<<'\n';
	//	lfbst.add(15);
	//	std::cout<<"\ncontains 15:"<<lfbst.contains(15)<<'\n';


}

void lfbstOperations(double ratioAdd, double ratioRemove, double ratioContains){

	//puts("This thread started..");
	int numOps = 1;
	int addLimit = numOps*ratioAdd;
	int removeLimit = numOps*ratioRemove;
	int containsLimit = numOps*ratioContains;

	int addCounter, removeCounter, containsCounter;
	addCounter = removeCounter = containsCounter = 0;

	for( int i =0 ; i< numOps; i++ ){
		int operationType = rand() %3;
		int randValue = rand() % 1000000;
		if(operationType == 0 && addCounter < addLimit){
			//printf("add called: %d\n", randValue);
			lfbst->add(randValue);
			addCounter++;
			//printf("add returned: %d\n", randValue);
//			puts("add called");
//			break;
		}
		else if(operationType == 1 && removeCounter < removeLimit){
			//printf("remove called: %d\n", randValue);
			lfbst->remove(randValue);
			removeCounter++;
			//printf("remove returned: %d\n", randValue);
//			puts("remove called");
//			break;

		}
		else if (containsCounter < containsLimit){
			//printf("contains called: %d\n", randValue);
			lfbst->contains(randValue);
			containsCounter++;
			//printf("contains returned: %d\n", randValue);
//			puts("contains called");
//			break;

		}
		//printf("executed %d\n",i);
	}
	//puts("This thread finished");


//	for(int i=0; i< addLimit; i++ ){
//		int randValue = rand() % 1000;
//		lfbst->add(randValue);
//	}
//	for(int i=0; i< removeLimit; i++ ){
//		int randValue = rand() % 1000;
//		lfbst->remove(randValue);
//	}
//	for(int i=0; i< containsLimit; i++ ){
//		int randValue = rand() % 1000;
//		lfbst->contains(randValue);
//	}
}

void TestScenario(double addRatio, double removeRatio, double containsRatio){


	for(int numThreads=1; numThreads<=8; numThreads*=2){
		lfbst = new LockFreeBST();
		std::cout<<"time required for number of thread: "<< numThreads <<"\n";
	    typedef std::chrono::high_resolution_clock Clock;
	    typedef std::chrono::milliseconds milliseconds;
	    Clock::time_point t0 = Clock::now();

		std::vector<std::thread > threadCollection;

		for(int threadCount =0; threadCount< numThreads; threadCount++){
			threadCollection.push_back(std::thread(lfbstOperations, addRatio, removeRatio, containsRatio));
		}

		for(auto &thread : threadCollection)
			thread.join();

	    Clock::time_point t1 = Clock::now();
	    milliseconds ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
	    std::cout<< ms.count() << " ms\n";
	    delete lfbst;
	}

}
void concurrentOperations(){

	TestScenario(.33, .33, .34);
	TestScenario(.50, .25, .25);
	TestScenario(.25, .25, .50);

}


int main(){



//	sequentialOperations(lfbst);
	concurrentOperations();
	return 0;
}

