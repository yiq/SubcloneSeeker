#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <assert.h>

#include "SubcloneExplore.h"
#include "MutationTreeNode.h"

using namespace qiaoy;

void TreeEnumeration(MutationTreeNode * root, std::vector<Symbol_t>& mutationVec, size_t symIdx);
void TreeAssessment(MutationTreeNode * root, std::vector<Symbol_t>& mutationVec);

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
	
	// At this point, ins should be a good in-stream.
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

// First of all, a tree traverser that will print out the tree.
// This will be used when the program decides to output a tree
	
// Tree Print Traverser. This one will just print the symbol id
// of the node that is given to it.
class TreePrintTraverser : public TreeTraverseDelegate {
public:
	virtual void processNode(TreeNode * node) {
		std::cout<<((MutationTreeNode *)node)->getSymbolId()<<",";
	}
};


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
	// Tree Enum Traverser. It will check if the last symbol has been
	// treated or not. If yes, the tree is complete and it will call
	// TreeAssessment to assess the viability of the tree; If no, it 
	// will add the current untreated symbol as a children to the node 
	// that is given to it, and call TreeEnumeration again to go into 
	// one more level of the recursion.
	class TreeEnumTraverser : public TreeTraverseDelegate {
	protected:
		// Some state variables that the TreeEnumeration
		// function needs to expose to the traverser
		std::vector<Symbol_t>& _mutationVec;
		size_t _symIdx;
		MutationTreeNode *_floatNode;
		MutationTreeNode *_root;
		
	public:
		
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
				
				#ifdef DEBUG
				TreePrintTraverser TreePrintTraverserObj;
				std::cout<<"Pre-Order: ";
				TreeNode::PreOrderTraverse(_root, TreePrintTraverserObj);
				std::cout<<"\t";
				
				std::cout<<"Post-Order: ";
				TreeNode::PostOrderTraverse(_root, TreePrintTraverserObj);
				std::cout<<std::endl;
				#endif
				
				TreeAssessment(_root, _mutationVec);
				
				#ifdef DEBUG
				std::cerr<<"press enter to next tree..."<<std::endl;
				
				#ifdef STEP
				getchar();
				#endif
				#endif
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

void TreeAssessment(MutationTreeNode * root, std::vector<Symbol_t>& mutationVec)
{
	class FracAsnTraverser : public TreeTraverseDelegate {
	protected:
		std::vector<Symbol_t>& _mutationVec;
		
	public:
		FracAsnTraverser(std::vector<Symbol_t>& mutationVec): _mutationVec(mutationVec) {;}
		virtual void processNode(TreeNode * node) {
			if(node->isLeaf()) {
				// direct assign
				((MutationTreeNode *)node)->setNodeFraction(_mutationVec[((MutationTreeNode *)node)->getSymbolId()-1].fraction);
				((MutationTreeNode *)node)->setTreeFraction(_mutationVec[((MutationTreeNode *)node)->getSymbolId()-1].fraction);
				
				#ifdef DEBUG
				std::cerr<<"Setting fraction "<<((MutationTreeNode *)node)->getNodeFraction() << " for L-symbol "<<((MutationTreeNode *)node)->getSymbolId()<<std::endl;
				#endif
				
			}
			else {
				// intermediate node. assign it's mutation fraction - subtree_fraction
				// actually, if it's root, assign 1
				if(((MutationTreeNode *)node)->getSymbolId() == 0)
					((MutationTreeNode *)node)->setTreeFraction(1);
				else
					((MutationTreeNode *)node)->setTreeFraction(_mutationVec[((MutationTreeNode *)node)->getSymbolId()-1].fraction);
				
				assert(((MutationTreeNode *)node)->getTreeFraction() >= 0 && ((MutationTreeNode *)node)->getTreeFraction() <= 1);
				
				#ifdef DEBUG
				std::cerr<<"1. Setting fraction "<<((MutationTreeNode *)node)->getTreeFraction() << " for I-symbol "<<((MutationTreeNode *)node)->getSymbolId()<<std::endl;
				#endif
				
				double childrenFraction = 0;
				for(size_t i=0; i<node->vecChildren().size(); i++) 
					childrenFraction += ((MutationTreeNode *)node->vecChildren()[i])->getTreeFraction();
					
				#ifdef DEBUG
				std::cerr<<"2. Children Frac: "<<childrenFraction<<std::endl;
				#endif
				
				double nodeFraction = ((MutationTreeNode *)node)->getTreeFraction() - childrenFraction;
				if(nodeFraction < 1e-3 && nodeFraction > -1e-3)
					nodeFraction = 0;
				
				// check tree viability
				if(nodeFraction < 0) // root allowed to be depleted
				{
					#ifdef DEBUG
					std::cerr<<"Terminated"<<std::endl;
					#endif
					terminate();
				}
				else {
					((MutationTreeNode *)node)->setNodeFraction(nodeFraction);
					#ifdef DEBUG
					std::cerr<<"3. Setting N-fraction "<<((MutationTreeNode *)node)->getNodeFraction() << " for I-symbol "<<((MutationTreeNode *)node)->getSymbolId()<<std::endl;
					#endif
					assert(((MutationTreeNode *)node)->getNodeFraction() >= 0 && ((MutationTreeNode *)node)->getNodeFraction() <= 1);
					
				}
			}
		}
	};
	
	// Fraction Reset Traverser. This will go through the nodes and
	// reset the fraction to uninitialized state so that the same nodes
	// can be used for another structure's evaluation
	class NodeResetTraverser : public TreeTraverseDelegate {
	public:
		virtual void processNode(TreeNode * node) {
			((MutationTreeNode *)node)->setNodeFraction(-1);
			((MutationTreeNode *)node)->setTreeFraction(-1);
		}
	};
	
	class NodeFracTraverser : public TreeTraverseDelegate {
	public:
		virtual void processNode(TreeNode * node) {
			std::cout<<((MutationTreeNode *)node)->getSymbolId()<<":"<<((MutationTreeNode *)node)->getNodeFraction()<<std::endl;
		}
	};
	
	// check if the root is sane
	if(root == NULL)
		return;

	// calcuate tree fractions
	FracAsnTraverser fracTraverser(mutationVec);
	TreeNode::PostOrderTraverse(root, fracTraverser);
	
	// if the tree is viable, output it
	if(root->getNodeFraction() >= 0) {
		TreePrintTraverser printTraverser;
		NodeFracTraverser fracTraverser;
		std::cout<<"Viable Tree! Pre-Orer: ";
		TreeNode::PreOrderTraverse(root, printTraverser);
		std::cout<<"\tPost-Order: ";
		TreeNode::PostOrderTraverse(root, printTraverser);
		std::cout<<"\troot N-Frac: "<< ((MutationTreeNode *)root)->getNodeFraction()<<std::endl;
		
		#ifdef DEBUG_FRAC
		TreeNode::PreOrderTraverse(root, fracTraverser);
		#endif
	}

	// reset node and tree fractions
	NodeResetTraverser nrTraverser;
	TreeNode::PreOrderTraverse(root, nrTraverser);
}
