/**
 * @file Unit tests for TreeNode
 *
 * @see TreeNode
 * @author Yi Qiao
 */

#include <iostream>
#include "TreeNode.h"

#include "common.h"

using namespace SubcloneExplorer;

SUITE(testTreeNode) {
	TEST(ObjectCreation) {

		TreeNode root;

		CHECK(root.getParent() == NULL);
		CHECK(root.getVecChildren().size() == 0);
		CHECK(root.isRoot());
		CHECK(root.isLeaf());

		TreeNode c1;
		root.addChild(&c1);

		CHECK(c1.getParent() == &root);
		CHECK(root.getVecChildren().size() == 1);
		CHECK(root.getVecChildren()[0] == &c1);
		CHECK(root.isRoot());
		CHECK(!root.isLeaf());
		CHECK(!c1.isRoot());
		CHECK(c1.isLeaf());

		root.removeChild(&c1);

		CHECK(c1.getParent() == NULL);
		CHECK(root.getVecChildren().size() == 0);
	}
}

TEST_MAIN
