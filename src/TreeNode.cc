/**
 * @file TreeNode.cc
 * The implementation of the class TreeNode
 *
 * @author Yi Qiao
 */

/*
The MIT License (MIT)

Copyright (c) 2013 Yi Qiao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

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

	// Preprocess Hook
	traverseDelegate.preprocessNode(root);
	
	// recursively traverse the children nodes
	for(TreeNodeVec_t::iterator it = root->children.begin(); it != root->children.end(); it++) {
		TreeNode::PreOrderTraverse(*it, traverseDelegate);
		// check if premature-termination has happened
		if(traverseDelegate.isTerminated())
			return;
	}

	// Postprocess Hook
	traverseDelegate.postprocessNode(root);
}

void TreeNode::PostOrderTraverse(TreeNode * root, TreeTraverseDelegate &traverseDelegate)
{
	// check if premature-termination has happened
	if(traverseDelegate.isTerminated())
		return;
	
	// Check if the root node exists at all
	if (root == NULL) return;
	
	// Preprocess Hook
	traverseDelegate.preprocessNode(root);
	
	// recursively traverse the children nodes
	for(TreeNodeVec_t::iterator it = root->children.begin(); it != root->children.end(); it++) {
		TreeNode::PostOrderTraverse(*it, traverseDelegate);
		// check if premature-termination has happened
		if(traverseDelegate.isTerminated())
			return;
	}

	// Postprocess Hook
	traverseDelegate.postprocessNode(root);

	// Do the bidding of the traverse delegate
	traverseDelegate.processNode(root);
}
