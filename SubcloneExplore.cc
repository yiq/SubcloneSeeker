#include <iostream>
#include <fstream>
#include <vector>

#include "SubcloneExplore.h"
#include "MutationTreeNode.h"

using namespace qiaoy;

void TreeEnumeration(MutationTreeNode * root, std::vector<Symbol_t>& mutationVec, size_t symIdx);

void SubcloneExplorerMain(int argc, char* argv[])
{
	std::istream * ins;
	bool isInputFile = false;
	argc--; argv++;
	if(argc == 0) {
		ins = &std::cin;
		#ifdef DEBUG
		std::cerr<<"DEBUG: reading data from stdin"<<std::endl;
		#endif
	}
	else {
		ins = new std::ifstream(*argv, std::ifstream::in);
		if(!ins->good()) {
			std::cout<<"Unable to open file "<<*argv<<std::endl;
			exit(0);
		}
		isInputFile = true;
		#ifdef DEBUG
		std::cerr<<"DEBUG: reading data from file "<<*argv<<std::endl;
		#endif
	}
	
	// At this point, ins should be a good in stream.
	std::vector<Symbol_t> symbolVec;
	while(!ins->eof()) {
		Symbol_t newSymbol;
		newSymbol.symId = -1; newSymbol.fraction = -1;
		*ins >> newSymbol.symId >> newSymbol.fraction;
		if(newSymbol.symId < 0 || newSymbol.fraction < 0) break; // partial result
		symbolVec.push_back(newSymbol);
		#ifdef DEBUG
		std::cerr<<"DEBUG: new item read: "<<newSymbol.symId<<","<<newSymbol.fraction<<std::endl;
		#endif
	}
	if(isInputFile) ((std::ifstream *)ins)->close();
	
	#ifdef DEBUG
	std::vector<Symbol_t>::iterator symIt;
	for(symIt = symbolVec.begin(); symIt != symbolVec.end(); symIt++)
		std::cout<<"DEBUG: "<<symIt->symId<<","<<symIt->fraction<<std::endl;
	#endif
	
	// Mutation list read. Start to enumerate trees
	// 1. Create a node contains no mutation (symId = 0).
	// this node will act as the root of the trees
	MutationTreeNode *root = new MutationTreeNode;
	root->setSymbolId(0);
	
	TreeEnumeration(root, symbolVec, 0);	
	
}


// This function will recursively construct all possible tree structures
// using the given mutation list, starting with the mutation identified by symIdx
//
// The idea behind this is quite simple. If a node is not the last symbol on the 
// mutation list, it will try to add this particular node to every other existing
// node's children list. After one addition, the traverser calls the TreeEnumeration
// again but with a incremented symIdx, so that the next symbol can be treated in the
// same way. When the last symbol has been reached, the tree is printed out, in both
// Pre-Order and Post-Order, by the TreePrintTraverser class, so that each tree can be 
// uniquely identified.
void TreeEnumeration(MutationTreeNode * root, std::vector<Symbol_t>& mutationVec, size_t symIdx)
{
	// Tree Print Traverser. This one will just print the symbol id
	// of the node that is given to it.
	class TreePrintTraverser : public TreeTraverseDelegate {
	public:
		virtual void processNode(TreeNode * node) {
			std::cout<<((MutationTreeNode *)node)->getSymbolId()<<",";
		}
	};
	
	// Tree Enum Traverser. It will check if the last symbol has been
	// treated or not. If yes, the tree is complete and it will call
	// pre- and post- order traverse with the TreePrintTraverser
	// to output it; If no, it will add the current untreated symbol 
	// as a children to the node that is given to it, and call
	// TreeEnumeration again to go into one more level of the recursion.
	class TreeEnumTraverser : public TreeTraverseDelegate {
	public:
		// Some state variables that the TreeEnumeration
		// function needs to expose to the traverser
		std::vector<Symbol_t>& _mutationVec;
		size_t _symIdx;
		MutationTreeNode *_floatNode;
		MutationTreeNode *_root;
		
		TreeEnumTraverser(std::vector<Symbol_t>& mutationVec,
						  size_t symIdx,
						  MutationTreeNode *floatNode,
						  MutationTreeNode *root):
						_mutationVec(mutationVec), _symIdx(symIdx), 
						_floatNode(floatNode), _root(root) {;}
								
		
		virtual void processNode(TreeNode * node) {
			// Add the floating node as the chilren of the current node
			node->addChild(_floatNode);
			
			if(_symIdx == _mutationVec.size()) {
				// if the tree is complete, assess it somehow
				TreePrintTraverser TreePrintTraverserObj;
				std::cout<<"Pre-Order: ";
				TreeNode::PreOrderTraverse(_root, TreePrintTraverserObj);
				std::cout<<"\t";
				
				std::cout<<"Post-Order: ";
				TreeNode::PostOrderTraverse(_root, TreePrintTraverserObj);
				std::cout<<std::endl;
				
			}
			else {
				// Move on to the next symbol
				TreeEnumeration(_root, _mutationVec, _symIdx);
			}
			
			// Remove the child
			node->removeChild(_floatNode);
		}
	};

	// Create the new tree node
	MutationTreeNode *newNode = new MutationTreeNode;
	newNode->setSymbolId(mutationVec[symIdx].symId);

	// Configure the tree traverser
	TreeEnumTraverser TreeEnumTraverserObj(mutationVec, ++symIdx, newNode, root);
	
	// Traverse the tree
	TreeNode::PreOrderTraverse(root, TreeEnumTraverserObj);
	
	delete newNode;
}