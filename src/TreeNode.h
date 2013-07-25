#ifndef TREENODE_H
#define TREENODE_H

/**
 * @file TreeNode.h
 * Interface description of helper class TreeNode
 *
 * @author Yi Qiao
 */

#include <vector>

namespace SubcloneExplorer{
		
	// Forward Declaration so that the TreeTraverseDelegate
	// class can declare a pointer to it
	class TreeNode;

	/**
	 * @brief Vector of TreeNode pointers
	 *
	 * This data type is usually used for children node list
	 */
	typedef std::vector<TreeNode *> TreeNodeVec_t;

	/**
	 * @brief Delegate class handling tree traversing
	 *
	 * This delegate class defines a abstract method processNode, which will be
	 * called with the TreeNode the tree traversing algorithm is currently on.
	 *
	 * It also handles flow control, such as early termination
	 */
	class TreeTraverseDelegate {
	protected:
		bool _isTerminated; /**< whether the traversing has been terminated */
		
	public:
		/**
		 * Constructor
		 */
		TreeTraverseDelegate() { _isTerminated = false; }

		/**
		 * Destructor
		 */
		virtual ~TreeTraverseDelegate() {}
		
		/**
		 * terminate the traversing
		 */
		inline void terminate() {_isTerminated = true;};

		/**
		 * check whether the traversing has been terminated
		 */
		inline bool isTerminated() {return _isTerminated;}

		/**
		 * Hook to call before children nodes are recursively processed
		 *
		 * @param node the node whose children nodes are to be recursively processed
		 */
		virtual void preprocessNode(TreeNode * node) {;}
		
		/**
		 * Process the given node during a tree traverse
		 *
		 * @param node the Node which needs to be processed
		 */
		virtual void processNode(TreeNode * node) = 0;

		/**
		 * Hook to call after children nodes are recursively processed
		 *
		 * @param node the node whose children nodes are recursively processed
		 */
		virtual void postprocessNode(TreeNode * node) {;}
	};

	/**
	 * @brief Base class for any object that can seve as a tree node
	 *
	 * This class implements the basic tree operations, such as adding and
	 * removing children nodes, checking whether self is a root or a leaf, etc.
	 * It also implements, as class method, two basic traversing algorithm:
	 * . PreOrderTraverse
	 * . PostOrderTraverse
	 * Since the tree is not required to be binary, InOrder traverse makes
	 * little sense.
	 */
	class TreeNode {
	protected:
		TreeNodeVec_t children; /**< children node list */
		TreeNode * parent;		/**< parent node */

	public:

		// Allowing tree traverser to access protected members
		friend class TreeTraverseDelegate;

		/**
		 * minimal constructor, reset all member variables
		 */
		TreeNode():parent(NULL) {;}

		/**
		 * declaring destructor to be virtual
		 */
		virtual ~TreeNode(){}

		/**
		 * returns a reference to the children vector
		 *
		 * @return A std::vector of children node pointers
		 */
		inline TreeNodeVec_t& getVecChildren() {return children;}
		
		/** 
		 * returns the node's parent.
		 *
		 * @return parent node's address, or NULL if called on a root node
		 */
		inline TreeNode * getParent() {return parent;}
		
		/**
		 * append a given node to this node's children list
		 *
		 * @param child A pointer to the node which which will be added as a child
		 */
		void addChild(TreeNode *child);
		
		/** 
		 * remove a child of this node
		 *
		 * @param child A pointer to the children node to be removed
		 */
		void removeChild(TreeNode *child);
		
		/**
		 * Pre-Order traverse algorithm
		 *
		 * This method process all the children nodes first (recursively), and
		 * then process the root. The actual action performed on the nodes are
		 * defined by the TreeTraverseDelegate object
		 *
		 * @param root The root node of the (sub)tree the traversing takes place
		 * @param traverseDelegate An TreeTraverseDelegate object which defines the action performed on nodes
		 */
		static void PreOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate);

		/**
		 * Post-Order traverse algorithm
		 *
		 * This method process the root node first, then all the children 
		 * nodes (recursively), The actual action performed on the nodes are
		 * defined by the TreeTraverseDelegate object
		 *
		 * @param root The root node of the (sub)tree the traversing takes place
		 * @param traverseDelegate An TreeTraverseDelegate object which defines the action performed on nodes
		 */
		static void PostOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate);
	
		/**
		 * whether the node is a leaf node
		 *
		 * @return whether the node is a leaf node
		 */
		inline bool isLeaf() const {return children.size() == 0;}

		/**
		 * whether the node is a root node
		 *
		 * @return whether the node is a root node
		 */
		inline bool isRoot() const {return parent == NULL;}
	};
}

#endif
