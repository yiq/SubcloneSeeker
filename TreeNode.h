#ifndef TREENODE_H
#define TREENODE_H

#include <vector>

namespace qiaoy {
		
	// Forward Declaration so that the TreeTraverseDelegate
	// class can declare a pointer to it
	class TreeNode;
	
	typedef std::vector<TreeNode *> TreeNodeVec_t;
	
	// Base class for traverse delegate, which defines a function
	// that will be called upon when the traverser stump upon a 
	// node. The node will be fed to the function as a parameter
	class TreeTraverseDelegate {
	public:
		virtual void processNode(TreeNode * node) = 0;	
	};
	
	// The base class for tree nodes. Implements the basic tree
	// operations such as add children and remove children.
	// also it defines some convenient functions to test whether
	// any given node is a leaf or a root node.
	class TreeNode {
	protected:
		TreeNodeVec_t children;
		TreeNode * parent;
		
	public:
		TreeNode():parent(NULL) {;}
		~TreeNode(){}
		
		// returns a reference to the children vector
		inline TreeNodeVec_t& vecChildren() {return children;}
		
		// returns the node's parent.
		inline TreeNode * getParent() {return parent;}
		
		// append a given node to this node's children list
		void addChild(TreeNode *child);
		
		// remove a child of this node
		void removeChild(TreeNode *child);
		
		static void PreOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate);
		static void PostOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate);
		
		inline bool isLeaf() const {return children.size() == 0;}
		inline bool isRoot() const {return parent == NULL;}
	};
}

#endif
