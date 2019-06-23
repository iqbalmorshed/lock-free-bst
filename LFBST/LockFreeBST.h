/*
 * LockFreeBST.h
 *
 *  Created on: Oct 17, 2018
 *      Author: iqbal
 */

#ifndef LOCKFREEBST_H_
#define LOCKFREEBST_H_


#include <iostream>       // std::cout
#include <atomic>         // std::atomic
#include <thread>         // std::thread

#define INF 1000000

//class NodePtrInfo;
class Node;


class NodePtrInfo{
public:
	Node* nodeRef;
	bool flag;
	bool mark;
	bool thread;
	NodePtrInfo() noexcept {}
	NodePtrInfo(Node* nodeRef, bool flag, bool mark, bool thread) noexcept	:
							nodeRef(nodeRef),
							flag(flag),
							mark(mark),
							thread(thread){}
};

class Node{
public:
	Node() noexcept {}
	int k;
	std::atomic<NodePtrInfo> child[2];
	std::atomic<Node*> backLink;
	Node* preLink;
};


class LockFreeBST {

public:
	LockFreeBST(){

		root[0].k = -INF;
		root[0].child[0] = NodePtrInfo(&root[0], 0, 0, 1 );
		root[0].child[1] = NodePtrInfo(&root[1], 0, 0, 1);
		root[0].backLink = &root[1];
		root[0].preLink = nullptr;

		root[1].k = INF;
		root[1].child[0] = NodePtrInfo(&root[0], 0, 0, 0 );
		root[1].child[1] = NodePtrInfo(nullptr, 0, 0, 1);
		root[1].backLink = nullptr;
		root[1].preLink = nullptr;


	}
	bool contains(int k);
	bool remove(int k);
	bool add(int k);

private:
	int locate(Node*& prev, Node*& curr, double k);
	bool tryFlag(Node*& prev, Node*& curr, Node*& back, bool isThread);
	void tryMark(Node* curr, int dir);
	void cleanFlag(Node*& prev, Node*& curr, Node*& back, bool isThread);
	void cleanMark(Node*& curr, int markDir);
	int cmp(double x, double y);

	void printPointerInfo(Node*& prev, Node*& curr,const std::string& str);
	void compareNodePointerInfo(NodePtrInfo nodePtr1, NodePtrInfo nodePtr2, const std::string& str);
	bool CAS(std::atomic<NodePtrInfo>& oldValue, NodePtrInfo newValue, NodePtrInfo replacement );


	Node root[2];
	const int TIMES=4;


};

#endif /* LOCKFREEBST_H_ */
