#include "TreeNode.h"
#include <vector>
#include <algorithm>

using namespace qiaoy;

void TreeNode::addChild(TreeNode *child)
{
	if(child==NULL) return;
	
	for(size_t i=0; i<children.size(); i++) {
		if(child==children[i]) return;
	}
	
	children.push_back(child);
}

void TreeNode::removeChild(TreeNode *child)
{
	TreeNodeVec_t::iterator child_it = std::find(children.begin(), children.end(), child);
	if(child_it != children.end()) {
		// child found in the node's children list, removing it from the vector
		children.erase(child_it);
	}
}

void TreeNode::PreOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate)
{
	if (root == NULL) return;
	traverseDelegate.processNode(root);
	
	for(TreeNodeVec_t::iterator it = root->children.begin(); it != root->children.end(); it++) {
		TreeNode::PreOrderTraverse(*it, traverseDelegate);
	}
}

void TreeNode::PostOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate)
{
	if (root == NULL) return;
	
	for(TreeNodeVec_t::iterator it = root->children.begin(); it != root->children.end(); it++) {
		TreeNode::PostOrderTraverse(*it, traverseDelegate);
	}
	traverseDelegate.processNode(root);
}