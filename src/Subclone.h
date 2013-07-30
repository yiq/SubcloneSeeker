#ifndef SUBCLONE_H
#define SUBCLONE_H

/**
 * @file Subclone.h
 * Interface description of the data structure class Subclone
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
#include "Archivable.h"
#include <vector>

namespace SubcloneExplorer {

	// forward declaration of EventCluster, so that pointers can be made
	class EventCluster;
	class SubcloneSaveTreeTraverser;
	
	/**
	 * @brief The class that represents a subclone in a subclonal structure tree
	 *
	 * This class implements both the TreeNode and Archivable interfaces, so that it
	 * can be a node in a tree structure, and at the same time be archived into a 
	 * sqlite3 database. This is the fundamental building block of subclonal
	 * deconvoution solutions.
	 */
	class Subclone : public TreeNode, public Archivable {
		protected:
			double _fraction; /**< The percentage of this subclone */
			double _treeFraction; /**< The total fraction taken by the subtree rooted by this object */
			std::vector<EventCluster *> _eventClusters; /**< the event clusters this subclone contains */

			sqlite3_int64 parentId;	/**< The database id of the parent node, 0 represents a root */

		protected:
			// Implements Archivable
			virtual std::string getTableName();
			virtual std::string createTableStatementStr();
			virtual std::string createObjectStatementStr();
			virtual std::string updateObjectStatementStr();
			virtual std::string selectObjectColumnListStr();

			virtual int bindObjectToStatement(sqlite3_stmt* statement);
			virtual void updateObjectFromStatement(sqlite3_stmt* statement);

		public:

			/**
			 * Minimal constructor to reset all member variables
			 */
			Subclone() : TreeNode(), Archivable(), _fraction(0), _treeFraction(0), parentId(0) {;}

			/** set parent id
			 * @param pid The new parent id
			 */
			inline void setParentId(sqlite3_int64 pid) {parentId = pid;}

			/**
			 * return the fraction
			 * @return percentage of this subclone
			 */
			inline double fraction() const {return _fraction;}

			/**
			 * update the fraction
			 * @param fraction The new fraction value
			 */
			inline void setFraction(double fraction) { /*if (fraction >=0 && fraction <=1)*/ _fraction = fraction; }

			/**
			 * return the tree fraction
			 * @return percentage of the subtree rooted by this subclone
			 */
			inline double treeFraction() const {return _treeFraction;}

			/**
			 * update the tree fraction
			 * @param fraction The new tree fraction value
			 */
			inline void setTreeFraction(double fraction) { /*if (fraction >=0 && fraction <=1)*/ _treeFraction = fraction; }

			/**
			 * retreve a vector of all EventClusters this subclone has
			 *
			 * @return member EventCluster vector
			 */
			inline std::vector<EventCluster *> &vecEventCluster() {return _eventClusters;}

			/**
			 * Add a given EventCluster into the subclone
			 *
			 * @param cluster The EventCluster to be added to the subclone
			 */
			void addEventCluster(EventCluster *cluster);
	};

	/**
	 * @brief A tree traverser that saves an entire tree structure from a database
	 *
	 * The traverser takes one argument at construction, which is the database pointer to which the 
	 * entire subclone structure will be saved. When performing the actual load, a pre-order traverse
	 * should be performed on the root node of the tree being archived. The traverser will archive the
	 * root node, then traverse its children node.
	 */
	class SubcloneSaveTreeTraverser : public TreeTraverseDelegate {
		protected:
			sqlite3* _database; /**< To which database will the tree be saved */

		public:
			/**
			 * Constructor of the SubcloneSaveTreeTraverser class 
			 *
			 * @param database To which database will the tree be saved
			 */
			SubcloneSaveTreeTraverser(sqlite3 *database): _database(database) {;}

			virtual void processNode(TreeNode *node);
			virtual void preprocessNode(TreeNode *node);
	};

	/**
	 * @brief A tree traverser that loads an entire tree structure from a database
	 *
	 * The traverser takes one argument at construction, which is the database pointer from which the 
	 * entire subclone structure will be loaded. When performing the actual load, a pre-order traverse
	 * should be performed on a node that is initialized through unarchiving (which would have its id 
	 * field populated). The traverser will find all the children nodes in the database, unarchive them, 
	 * add them as children to the node currently being processed, and continue the traverse.
	 */
	class SubcloneLoadTreeTraverser : public TreeTraverseDelegate {
		protected:
			sqlite3* _database; /**< From which database will be tree be loaded */

		public:
			/**
			 * Constructor of the SubcloneLoadTreeTraverser class 
			 *
			 * @param database From which database will the tree be load
			 */
			SubcloneLoadTreeTraverser(sqlite3 *database): _database(database) {;}
			virtual void processNode(TreeNode *node);

			/**
			 * @brief Query the database for a set of nodes that appears to be root
			 *
			 * @param database The database to which the query is sent
			 * @return A vector of IDs representing nodes in the database that appears to be root nodes.
			 */
			static std::vector<sqlite3_int64> rootNodes(sqlite3 *database);

			/**
			 * @brief Query the database for a set of nodes that appears to be the children of a given node ID
			 *
			 * @param database The database to which the query is sent
			 * @param parentId The ID representing a node in the database
			 * @return a vector of IDs representing nodes in the database that appears to be the direct children of the given parent ID
			 */
			static std::vector<sqlite3_int64> nodesOfParentID(sqlite3 *database, sqlite3_int64 parentId);
	};


	/**
	 * A vector of Subclone pointers
	 */
	typedef std::vector<Subclone *> SubclonePtr_vec;
}

#endif
