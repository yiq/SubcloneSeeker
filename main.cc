#include <iostream>
#include <assert.h>
#include "TreeNode.h"

using namespace qiaoy;

void test_tree();
void test_traverse();

// simple tree node with int as it's satllite data
class SimpleTreeNode : public TreeNode {
protected:
	int data;
	
public:
	void setData(int data) {this->data = data;}
	int getData() {return data;}
};


int main(int argc, char *argv[])
{
	test_tree();
	test_traverse();
	return 0;
}



#pragma mark --Text Functions--
void test_tree() {
	SimpleTreeNode *root = new SimpleTreeNode();
	assert(root->getParent() == NULL);
	assert(root->getData() == 0);
	std::cout<<"Test passed"<<std::endl;
}

class PrintDataTraverser : public TreeTraverseDelegate {
public:
	virtual void processNode(TreeNode * node) {
		
		std::cout<<((SimpleTreeNode *)node)->getData()<<",";
	}
} pdTraverserObj;

void test_traverse() {
	// 1
	SimpleTreeNode * root = new SimpleTreeNode;
	root->setData(1);
	
	// 2
	SimpleTreeNode * node2 = new SimpleTreeNode;
	node2->setData(2);
	root->addChild(node2);
	
	// 3
	SimpleTreeNode * node3 = new SimpleTreeNode;
	node3->setData(3);
	node2->addChild(node3);
	
	// 4
	SimpleTreeNode * node4 = new SimpleTreeNode;
	node4->setData(4);
	node2->addChild(node4);
	
	// 5
	SimpleTreeNode * node5 = new SimpleTreeNode;
	node5->setData(5);
	root->addChild(node5);
	
	// 6
	SimpleTreeNode * node6 = new SimpleTreeNode;
	node6->setData(6);
	node5->addChild(node6);
	
	// 7
	SimpleTreeNode * node7 = new SimpleTreeNode;
	node7->setData(7);
	node5->addChild(node7);
	
	//Traverser
	std::cout<<"pre-order traverse: ";
	TreeNode::PreOrderTraverse(root, pdTraverserObj);
	std::cout<<std::endl;
	
	std::cout<<"post-order traverse: ";
	TreeNode::PostOrderTraverse(root, pdTraverserObj);
	std::cout<<std::endl;
	
	// removing the right tree
	std::cout<<"removing the right tree rooted with node 5"<<std::endl;
	root->removeChild(node5);
	
	std::cout<<"pre-order traverse: ";
	TreeNode::PreOrderTraverse(root, pdTraverserObj);
	std::cout<<std::endl;
	
	std::cout<<"post-order traverse: ";
	TreeNode::PostOrderTraverse(root, pdTraverserObj);
	std::cout<<std::endl;
	
	
	
}