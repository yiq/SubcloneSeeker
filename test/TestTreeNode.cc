/**
 * @file Unit tests for TreeNode
 *
 * @see TreeNode
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

#include <iostream>
#include "TreeNode.h"

#include "common.h"

using namespace SubcloneSeeker;

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
