#ifndef TREENODE_H
#define TREENODE_H

#include <vector>
#include <string>
#include <sqlite3/sqlite3.h>

namespace qiaoy {
		
	// Forward Declaration so that the TreeTraverseDelegate
	// class can declare a pointer to it
	class TreeNode;
	
	typedef std::vector<TreeNode *> TreeNodeVec_t;

	typedef struct _db_record {
		sqlite3_int64 id;
		sqlite3_int64 parentId;
		bool isStoredInDB;

		_db_record():
			id(0), parentId(0), isStoredInDB(false)
		{;}
	} _db_record_s;
	
	// Base class for traverse delegate, which defines a function
	// that will be called upon when the traverser stump upon a 
	// node. The node will be fed to the function as a parameter
	class TreeTraverseDelegate {
	protected:
		bool _isTerminated;
		
	public:
		TreeTraverseDelegate() { _isTerminated = false; }
		virtual ~TreeTraverseDelegate() {}
		
		inline void terminate() {_isTerminated = true;};
		inline bool isTerminated() {return _isTerminated;}
		
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
		_db_record_s dbRecord;
		
	public:
		friend class TreeTraverseDelegate;

		TreeNode():parent(NULL), dbRecord() {;}
		TreeNode(sqlite3 *database, sqlite3_int64 id);

		~TreeNode(){}

		virtual std::string description() {return "Generic Tree Node";}
		
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

		inline sqlite3_int64 getDBId() const {return dbRecord.id;}

		static void saveTree(sqlite3 *database, TreeNode *root);
		static std::vector<sqlite3_int64> loadRoots(sqlite3 *database);

		virtual void update(sqlite3 *database) {;}
		virtual void load(sqlite3 *database) {;}
		
	};
}

#endif
