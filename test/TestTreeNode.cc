/**
 * @file Unit tests for TreeNode
 *
 * @see TreeNode
 * @author Yi Qiao
 */

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "../TreeNode.h"

using namespace SubcloneExplorer;

void testTreeNode() {

	TreeNode root;

	BOOST_CHECK(root.getParent() == NULL);
	BOOST_CHECK(root.getVecChildren().size() == 0);
	BOOST_CHECK(root.isRoot());
	BOOST_CHECK(root.isLeaf());
	
	TreeNode c1;
	root.addChild(&c1);
	
	BOOST_CHECK(c1.getParent() == &root);
	BOOST_CHECK(root.getVecChildren().size() == 1);
	BOOST_CHECK(root.getVecChildren()[0] == &c1);
	BOOST_CHECK(root.isRoot());
	BOOST_CHECK(!root.isLeaf());
	BOOST_CHECK(!c1.isRoot());
	BOOST_CHECK(c1.isLeaf());

	root.removeChild(&c1);

	BOOST_CHECK(c1.getParent() == NULL);
	BOOST_CHECK(root.getVecChildren().size() == 0);
}
