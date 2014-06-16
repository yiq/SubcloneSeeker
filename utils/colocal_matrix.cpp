#include <iostream>
#include <map>
#include <sqlite3/sqlite3.h>
#include <cassert>
#include <cstdlib>
#include "SegmentalMutation.h"
#include "EventCluster.h"
#include "SomaticEvent.h"
#include "Subclone.h"

using namespace std;
using namespace SubcloneSeeker;

static vector<int> preceedingStack;
static map< int, map<int, int> > occuranceMap;

void countOccurrance(int chr1, int chr2) {
	if(occuranceMap.find(chr1) == occuranceMap.end()) {
		occuranceMap[chr1][chr2] = 1;
	}
	else if (occuranceMap[chr1].find(chr2) == occuranceMap[chr1].end()) {
		occuranceMap[chr1][chr2] = 1;
	}
	else {
		occuranceMap[chr1][chr2]++;
	}
}

class CoexistanceTraverseDelegate : public TreeTraverseDelegate {

	public:

		// before processing any child nodes, push the events in the current
		// node onto the preceeding event stack
		virtual void preprocessNode(TreeNode * node) {
			Subclone *clone = dynamic_cast<Subclone *>(node);
			if(clone == NULL) {
				cerr<<"TreeNode is not Subclone, terminating"<<endl;
				terminate();
				return;
			}

			int numOfEvents = clone->vecEventCluster().size();

			for(size_t i=0; i<numOfEvents; i++) {
				EventCluster *ec = clone->vecEventCluster()[i];
				assert(ec->members().size() == 1);

				CNV *cnv = dynamic_cast<CNV *>(ec->members()[0]);
				preceedingStack.push_back(cnv->range.chrom);
			}
		}

		virtual void processNode(TreeNode *node) {
			Subclone *clone = dynamic_cast<Subclone *>(node);
			if(clone == NULL) {
				cerr<<"TreeNode is not Subclone, terminating"<<endl;
				terminate();
				return;
			}

			for(size_t i=0; i<clone->vecEventCluster().size(); i++) {
				EventCluster *ec = clone->vecEventCluster()[i];
				assert(ec->members().size() == 1);

				CNV *cnv = dynamic_cast<CNV *>(ec->members()[0]);
				vector<int>::const_iterator cit;
				for(cit = preceedingStack.begin(); cit != preceedingStack.end(); cit++) {
					countOccurrance(cnv->range.chrom, *cit);
				}
			}
		}

		virtual void postprocessNode(TreeNode * node) {
			Subclone *clone = dynamic_cast<Subclone *>(node);
			if(clone == NULL) {
				cerr<<"TreeNode is not Subclone, terminating"<<endl;
				terminate();
				return;
			}
			int numOfEvents = clone->vecEventCluster().size();

			for(int i=0; i<numOfEvents; i++) {
				preceedingStack.pop_back();
			}
		}
};

int main(int argc, char* argv[])
{
	if(argc<2) {
		cout<<"Usage: "<<argv[0]<<" <subclone-sqlite-db>"<<endl;
		exit(0);
	}

	// Open database connection
	sqlite3 *dbh;
	if(sqlite3_open_v2(argv[1], &dbh, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
		cerr<<"Unable to open database file "<<argv[1]<<endl;
		exit(1);
	}

	CoexistanceTraverseDelegate ctd;
	SubcloneLoadTreeTraverser loadTraverser(dbh);

	DBObjectID_vec rootIDs = SubcloneLoadTreeTraverser::rootNodes(dbh);

	cout<<rootIDs.size()<<endl;

	DBObjectID_vec::const_iterator it;
	for(it = rootIDs.begin(); it != rootIDs.end(); it++) {
		Subclone *root = new Subclone();
		assert(preceedingStack.size() == 0);
		root->unarchiveObjectFromDB(dbh, *it);
		TreeNode::PreOrderTraverse(root, loadTraverser);
		TreeNode::PreOrderTraverse(root, ctd);
	}

	map< int, map<int, int> >::const_iterator it1;
	map<int, int>::const_iterator it2;

	for(it1 = occuranceMap.begin(); it1 != occuranceMap.end(); it1++) {
		for(it2 = occuranceMap[it1->first].begin(); it2 != occuranceMap[it1->first].end(); it2++) {
			cout<<it1->first<<"\t"<<it2->first<<"\t"<<it2->second<<endl;
		}
	}

	sqlite3_close(dbh);
}
