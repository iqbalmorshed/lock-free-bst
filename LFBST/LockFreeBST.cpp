/*
 * LockFreeBST.cpp
 *
 *  Created on: Oct 17, 2018
 *      Author: iqbal
 */

#include "LockFreeBST.h"


int LockFreeBST::locate(Node*& prev, Node*& curr, double k) {

	int stepCounter =0;
	while(true){
		stepCounter++;
		if(stepCounter>=10000)return -1;

		int dir = cmp(k, curr->k);

//		NodePtrInfo R = curr->child[dir];
//		Node* curr_backlink = curr->backLink;
//		printf("Locate: 1: prev->k = %d, curr->k = %d, curr->backlink->k %d, k = %lf, dir %d, child threaded?= %d\n",  prev->k, curr->k, curr_backlink->k, k, dir, R.thread);
		//if(prev->k == 492)
		if(dir == 2)
			return dir;

		else{
			NodePtrInfo R = curr->child[dir];
			if(R.mark == 1 && dir == 1){
				Node* newPrev = prev->backLink;
				//cleanMarked(prev, curr, dir); //cleanMarked function is not clear yet
				cleanMark(curr, dir);
				prev = newPrev;
				int pDir = cmp(k, prev->k);
				NodePtrInfo temp = prev->child[pDir];
				curr = temp.nodeRef;
				continue;
			}

			if(R.thread){
				double nextE = R.nodeRef->k;
				if(dir == 0 || k < nextE){
					//printf("Locate: 2: k = %d, prev->K = %d, curr->k = %d, dir %d\n", k, prev->k , curr->k, dir);
					//if((int)k==218)exit(0);
					return dir;
				}
				else{
					prev = curr;
					curr = R.nodeRef;
				}
			}
			else{
				prev = curr;
				curr = R.nodeRef;


			}

		}
	}

}

bool LockFreeBST::contains(int k) {

	Node* prev = &root[1];
	Node* curr = &root[0];
	int dir;
	dir = locate(prev, curr, k);
	if(dir == -1){
		return 0;
	}

	if(dir == 2)
		return true;
	else
		return false;
}

bool LockFreeBST::remove(int k) {

//	printf("\nremove requested for %d\n",k);
	Node* prev = &root[1];
	Node* curr = &root[0];
	bool result;
	double epsilon = 0.5;
	int dir;
	dir = locate(prev, curr, (double)k - epsilon);
	if(dir == -1){return 0;}

	NodePtrInfo next = curr->child[dir];

//	printf("remove:1: k: %d, prev->K %d, curr->k %d, dir %d, next->k %d,\n", k, prev->k, curr->k, dir, next.nodeRef->k);
	if(k != next.nodeRef->k){
		return false;
	}
	else{
//		printf("remove 2: key: %d, prev->K %d, curr->k %d, next->k %d,\n", k, prev->k, curr->k, next.nodeRef->k);
		result = tryFlag(curr, next.nodeRef, prev, true);
//		printf("remove 3: result %d\n",result);
//		printPointerInfo(curr, next.nodeRef, "inside remove");

		NodePtrInfo curr_child = curr->child[dir];
		if(curr_child.nodeRef == next.nodeRef){
			cleanFlag(curr, next.nodeRef, prev, true);
		}
	}

	return result;

}

bool LockFreeBST::add(int k) {
	Node* prev = &root[1];
	Node* curr = &root[0];

	Node* node = new Node;
	node->k = k;
	node->child[0] = NodePtrInfo(node, 0, 0, 1);

	while(true){
		//printf("add: while: 1: prev-k: %d, curr-k %d, k %d\n", prev->k, curr->k, k  );
		int dir;
		dir = locate(prev, curr, k);
		if(dir == -1){return 0;}

		if(dir == 2)
			return false;
		else{
			NodePtrInfo R = curr->child[dir];
			node->child[1] = NodePtrInfo(R.nodeRef, 0, 0, 1);
			node->backLink = curr;

			//printf("add: while:before CAS curr->k : %d\n", curr->k);
			//bool result = curr->child[dir].compare_exchange_weak(R, replacement);
			bool result = CAS(curr->child[dir], NodePtrInfo(R.nodeRef, 0, 0, 1), NodePtrInfo(node, 0, 0, 0));

			if(result){
//				if(k==204 || k==492)printf("added:==================================== %d\n\n", k);
				return true;
			}
			else{
				//This part is for helping
				NodePtrInfo newR = curr->child[dir];
				if(newR.nodeRef == R.nodeRef){
					Node* newCurr = prev;
					if(newR.mark)
						cleanMark(curr, dir);
					else if(newR.flag)
						cleanFlag(curr, R.nodeRef, prev, true);

					curr = newCurr;
					prev = newCurr->backLink;
				}
			}
		}
	}

}

int LockFreeBST::cmp(double x, double y) {
	if(x==y)
		return 2;
	if(x > y)
		return 1;
	else
		return 0;
}



bool LockFreeBST::tryFlag(Node*& prev, Node*& curr, Node*& back, bool isThread) {

	std::atomic<NodePtrInfo> atomicNodePointerInfo;
	while(true){

		int pDir = cmp(curr->k, prev->k) & 1; //curr-> and prev->K will be same when they are pointing to the same node. This is only possible by the threaded left-link.
		bool t = isThread;

		NodePtrInfo test = NodePtrInfo(curr, 0, 0, t);
		NodePtrInfo replace = NodePtrInfo(curr, 1, 0, t);
		//compareNodePointerInfo(prev->child[pDir], test, "inside try Flag 1");
		bool result = CAS(prev->child[pDir],test, replace);

		if(result){
//			printf("tryFlag: Flagged successfully: prev->k %d, curr->k %d\n", prev->k, curr->k);
			return true;
		}
		else{

			NodePtrInfo temp = prev->child[pDir];
			if(temp.nodeRef == curr){
				if(temp.flag)
					return false;
				else if(temp.mark)
					cleanMark(prev, pDir);
				prev = back;	//back is provided as third parameter. It helps to step back and restart.
				int newPDir = cmp(curr->k, prev->k);
				NodePtrInfo temp = prev->child[newPDir];
				Node* newCurr = temp.nodeRef;
				locate(prev, newCurr, curr->k);
				if(newCurr != curr)
					return false;
				prev = prev->backLink;
			}
		}

	}
}

void LockFreeBST::cleanFlag(Node*& prev, Node*& curr, Node*& back, bool isThread) {

	std::atomic<NodePtrInfo> atomicNodePointerInfo;
	if(isThread){
		while(true){
			NodePtrInfo next = curr->child[1];
			if(next.mark)
				break;
			else if(next.flag){

				if(back == next.nodeRef)
					back = back->backLink;

				Node* backNode = curr->backLink;
				cleanFlag(curr, next.nodeRef, backNode, next.thread);

				if(back == next.nodeRef){
					int pDir = cmp(prev->k, backNode->k);
					//prev = back->child[pDir];
					Node* back_temp = back;
					NodePtrInfo back_child = back_temp->child[pDir];
					prev = back_child.nodeRef;
				}
			}
			else{
				if(curr->preLink != prev) //step 2: set the prelink.
					curr->preLink = prev;

				//step: 3: mark the outgoing right link
				bool result = CAS(curr->child[1], NodePtrInfo(next.nodeRef, 0, 0, next.thread), NodePtrInfo(next.nodeRef, 0, 1, next.thread));

				NodePtrInfo temp = curr->child[1];
				if(result){

//					puts("CleanFlag 1: successfully marked right link");
//					printf("curr->k %d, temp.nodeRef->k %d, temp.mark %d\n", curr->k, temp.nodeRef->k, temp.mark);
//					exit(0);
					break;
				}


			}
		}
		cleanMark(curr, 1);
	}
	else{

		NodePtrInfo right = curr->child[1];
//		printf("inside cleanFlag2: prev->k %d, curr->k %d, right.nodeRef->k %d, right.flag %d, right.mark %d, right.thread %d\n",
//				prev->k, curr->k, right.nodeRef->k, right.flag, right.mark, right.thread );
		//exit(0);
		if(right.mark){
			NodePtrInfo left = curr->child[0];
			Node* preNode = curr->preLink;

			if(left.nodeRef != preNode){ //this is cat 3 node
//				puts("Clean flag 3: Entered to step 6");
				//exit(0);
				tryMark(curr, 0);
				cleanMark(curr, 0);
			}
			else{
				int pDir = cmp(curr->k, prev->k);
				if(left.nodeRef == curr){ //cat 1 node

					CAS(prev->child[pDir], NodePtrInfo(curr, 1, 0, 0), NodePtrInfo(right.nodeRef, 0, 0, right.thread)); //what is f.

					if(!right.thread){
						right.nodeRef->backLink.compare_exchange_weak(curr,prev);
					}
//					printf("removed %d\n",curr->k);
				}
				else{


					bool result = CAS(preNode->child[1], NodePtrInfo(curr, 1, 0, 1), NodePtrInfo(right.nodeRef, 0, 0, right.thread));
//					printf("swap success1: %d\n", result);

					if(!right.thread){
						result = right.nodeRef->backLink.compare_exchange_strong(curr, prev);
//						printf("swap success2: %d\n", result);
					}


					result = CAS(prev->child[pDir], NodePtrInfo(curr, 1, 0, 0), NodePtrInfo(preNode, 0, 0, right.thread));
//					printf("swap success3: %d\n", result);

					result = preNode->backLink.compare_exchange_strong(curr, prev);
//					printf("swap success4: %d\n", result);
//					std::cout<<"removed: "<<curr->k<<'\n';
				}
			}
		}
		else if(right.thread && right.flag){
			Node* delNode = right.nodeRef;
			Node* parent = delNode->backLink;
			while(true){

				int pDir = cmp(delNode->k, parent->k); //changed from paper
				NodePtrInfo temp = parent->child[pDir];
				if(temp.mark)
					cleanMark(parent, pDir);
				else if(temp.flag)
					break;
				else if( CAS(parent->child[pDir],NodePtrInfo(delNode, 0, 0, 0), NodePtrInfo(delNode, 1, 0, 0) )){
//					puts("cleanFlag 4: step 5 done. Parent link of the node to be deleted flagged successfully");
					break;
				}

			}

			Node* backNode = parent->backLink;

//			printf("parent->k %d, delNode->k %d, backNode->k %d\n", parent->k, delNode->k, backNode->k);
			//exit(0);
			cleanFlag(parent, delNode, backNode, false); //changed to false. I think, "true" was a mistake.
		}
	}
}

void LockFreeBST::tryMark(Node* curr, int dir) {

	while(true){
		Node* back = curr->backLink;
		NodePtrInfo next = curr->child[dir];
		if(next.mark)
			break;
		else if(next.flag){
			if(!next.thread){
				cleanFlag(curr, next.nodeRef, back, false);
				continue;
			}
			else if( next.thread && dir){
				cleanFlag(curr, next.nodeRef, back, true);
				continue;
			}
		}

		bool result = CAS(curr->child[dir], NodePtrInfo(next.nodeRef, 0, 0, next.thread), NodePtrInfo(next.nodeRef, 0, 1, next.thread));
		if(result)
			break;

	}

}
void LockFreeBST::cleanMark(Node*& curr, int markDir) {

	std::atomic<NodePtrInfo> atomicNodePointerInfo;
	NodePtrInfo left = curr->child[0];
	NodePtrInfo right = curr->child[1];
//	puts("inside clean Mark:");
//	printf("curr %d, marDir %d, curr->child[0]->k %d, curr->child[1]->k: %d\n", curr->k, markDir, left.nodeRef->k, right.nodeRef->k);

	if(markDir){

		//int pDir = markDir;
		Node* delNode = curr; //Not sure
		while(true){

			Node* preNode = delNode->preLink;
//			std::cout<<"preNode->k :"<<preNode->k<<'\n';

			Node* parent = delNode->backLink;
			int pDir = cmp(delNode->k, parent->k);
			NodePtrInfo parent_child =  parent->child[pDir];

			//step 4 for category 1 and 2. Acutally step 5: flag the incoming parent link
			if(preNode == left.nodeRef){ //category 1 or 2 node.
//				puts("clearn mark: inside cat 1 and 2");

				Node* parent = delNode->backLink;
				Node* back = parent->backLink;
//				printf("delNode->k %d, parent->k %d, back-> %d\n", delNode->k, parent->k, back->k);
				tryFlag(parent, curr, back, false); // why threaded? So, I changed it to non-threaded

//				printf("Clean Mark 4: parent_child.nodeRef %d, curr %d, parent_child->k %d, curr->k %d\n", parent_child.nodeRef, curr, parent_child.nodeRef->k, curr->k);
				if(parent_child.nodeRef == curr){ // If the link still persists
					cleanFlag(parent, curr, back, false);
					break;
				}
			}
			else{
				//category 3 node.
				//This step 4 for category 3 node. Step 4: flag the incoming parent link of the predecessor.
				Node* preParent = preNode->backLink;
				NodePtrInfo temp = preParent->child[1]; // child[1] because predecessor of cat3 node is always the right child of it's parent.
				Node* backNode = preParent->backLink;

//				printf("clean Mark 2: preNode->k %d, preParent->k %d, backNode->k %d\n", preNode->k , preParent->k, backNode->k );
//				printf("clean Mark 3: temp.noderef->k: %d, temp.flag: %d, temp.mark: %d, temp.thread: %d\n", temp.nodeRef->k, temp.flag, temp.mark, temp.thread);


				if(temp.mark)
					cleanMark(preParent, 1);
				else if(temp.flag){
					cleanFlag(preParent, preNode, backNode, false); //changed isThreaded parameter to false
					break;
				}
				else if(CAS(preParent->child[pDir], NodePtrInfo (preNode, 0, 0, 0), NodePtrInfo(preNode, 1, 0, 0))){ //
//					printf("incoming parentlink pred successfully flagged\n");

					cleanFlag(preParent, preNode, backNode, false); //changed isThreaded parameter to false.
					break;
				}
			}
		}
	}
	else{
		if(right.mark){

			Node* preNode = curr->preLink;
			tryMark(preNode, 0);
			cleanMark(preNode, 0);
		}
		else if(right.thread && right.flag){

			Node* delNode = right.nodeRef;
			Node* delNodePa = delNode->backLink;
			Node* preParent = curr->backLink;
			int pDir = cmp(delNode->k, delNodePa->k);
			NodePtrInfo delNodeL = delNode->child[0];
			NodePtrInfo delNodeR = delNode->child[1];

			int res1 = -1, res2 =-1, res3=-1, res4=-1, res5=-1, res6 =-1, res7=-1, res8=-1;
			//res1 = CAS(preParent->child[1], NodePtrInfo(curr, left.flag, 0, 0), NodePtrInfo(left.nodeRef, left.flag, 0, left.thread));
			res1 = CAS(preParent->child[1], NodePtrInfo(curr, 1, 0, 0), NodePtrInfo(left.nodeRef, left.flag, 0, left.thread));

			if(!left.thread){
				res2 = left.nodeRef->backLink.compare_exchange_weak(curr,preParent);

			}

			res3 = CAS(curr->child[0], NodePtrInfo(left.nodeRef, 0, 1, left.thread), NodePtrInfo(delNodeL.nodeRef, 0, 0, 0) );

			res4 = delNodeL.nodeRef->backLink.compare_exchange_strong(delNode, curr);

			res5= CAS(curr->child[1], NodePtrInfo(right.nodeRef, 1, 0, 1), NodePtrInfo(delNodeR.nodeRef, 0, 0, delNodeR.thread) );

			if(!delNodeR.thread){
				res6 = delNodeR.nodeRef->backLink.compare_exchange_strong(delNode,curr);

			}

			res7 = CAS(delNodePa->child[pDir], NodePtrInfo(delNode, 1, 0, 0), NodePtrInfo(curr, 0, 0, 0) );

			res8 = curr->backLink.compare_exchange_strong(preParent, delNodePa);

//			printf("removed: %d\n", delNode->k);
//			std::cout<<res1<<" "<<res2<<" "<<res3<<" "<<res4<<" "<<res5<<" "<<res6<<" "<<res7<<" "<<res8<<'\n';
			//exit(0);

		}
	}
}

void LockFreeBST::printPointerInfo(Node*& prev, Node*& curr,const std::string& str) {
	int dir = cmp(curr->k, prev->k) & 1;
	NodePtrInfo ptrInfo = prev->child[dir];
	printf(" message: %s, pointer info: prev->k %d, curr->k %d, dir %d, flag bit: %d, mark bit: %d, thread bit: %d \n",str.c_str(), prev->k, curr->k, dir, ptrInfo.flag, ptrInfo.mark, ptrInfo.thread);
}

void LockFreeBST::compareNodePointerInfo(NodePtrInfo nodePtr1,
		NodePtrInfo nodePtr2, const std::string& str) {
	printf("comparing two Node Pointer:\n");
	printf("%s : NodePtr1: nodeRef %d, flag %d, mark %d, thread %d ### NodePtr2: nodeRef %d, flag %d, mark %d, thread %d\n", str.c_str(),
			nodePtr1.nodeRef, nodePtr1.flag, nodePtr1.mark, nodePtr1.thread,
			nodePtr2.nodeRef, nodePtr2.flag, nodePtr2.mark, nodePtr2.thread);

}

bool LockFreeBST::CAS(std::atomic<NodePtrInfo>& oldValue, NodePtrInfo newValue,
		NodePtrInfo replacement) {

	bool result = 0;
	for(int i=0; i<TIMES && !result; i++){
		result = oldValue.compare_exchange_strong(newValue, replacement);
	}
	return result;

}
