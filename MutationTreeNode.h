#ifndef MUTATION_TREE_NODE_H
#define MUTATION_TREE_NODE_H

#include "TreeNode.h"

namespace qiaoy {
	
	typedef struct _Symbol {
	public:
		int symId;
		double fraction;
	} Symbol_t;
	
	// The Tree Node subclass tailored for storing mutation
	// symbol id and node fraction. 
	class MutationTreeNode : public TreeNode {
	protected:
		int symId;
		double nodeFraction;
		double treeFraction;
	
	public:
		
		MutationTreeNode(): symId(0), nodeFraction(-1), treeFraction(-1) {}
		
		inline void setSymbolId(int symId) {this->symId = symId;}
		inline int getSymbolId() const {return symId;}
		
		inline void setNodeFraction(double f) {nodeFraction = f;}
		inline double getNodeFraction() const {return nodeFraction;}
		
		inline void setTreeFraction(double f) {treeFraction = f;}
		inline double getTreeFraction() const {return treeFraction;}
	};
}


#endif