#include "TreeNode.h"
#include <vector>
#include <algorithm>

using namespace SubcloneExplorer;

void TreeNode::addChild(TreeNode *child)
{
	if(child==NULL) return;
	
	TreeNodeVec_t::iterator child_it = std::find(children.begin(), children.end(), child);
	if(child_it != children.end()) return;
	
	children.push_back(child);
	child->parent = this;
}

void TreeNode::removeChild(TreeNode *child)
{
	TreeNodeVec_t::iterator child_it = std::find(children.begin(), children.end(), child);
	if(child_it != children.end()) {
		// child found in the node's children list, removing it from the vector
		children.erase(child_it);
		child->parent = NULL;
	}
}

void TreeNode::PreOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate)
{
	// check if premature-termination has happened
	if(traverseDelegate.isTerminated())
		return;
	
	// Check if the root node exists at all
	if (root == NULL) return;
	
	// Do the bidding of the traverse delegate
	traverseDelegate.processNode(root);
	
	// recursively traverse the children nodes
	for(TreeNodeVec_t::iterator it = root->children.begin(); it != root->children.end(); it++) {
		TreeNode::PreOrderTraverse(*it, traverseDelegate);
		// check if premature-termination has happened
		if(traverseDelegate.isTerminated())
			return;
	}
}

void TreeNode::PostOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate)
{
	// check if premature-termination has happened
	if(traverseDelegate.isTerminated())
		return;
	
	// Check if the root node exists at all
	if (root == NULL) return;
	
	// recursively traverse the children nodes
	for(TreeNodeVec_t::iterator it = root->children.begin(); it != root->children.end(); it++) {
		TreeNode::PostOrderTraverse(*it, traverseDelegate);
		// check if premature-termination has happened
		if(traverseDelegate.isTerminated())
			return;
	}
	
	// Do the bidding of the traverse delegate
	traverseDelegate.processNode(root);
}
