/**
 * @file Unit tests for clustering algorithm
 *
 * @author Yi Qiao
 */

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "../SegmentalMutation.h"
#include "../EventCluster.h"
#include <sqlite3/sqlite3.h>

void testClustering() {

	SubcloneExplorer::CNV cnv[4];

	cnv[0].frequency=0.21;
	cnv[0].range.chrom = 1;
	cnv[0].range.position = 1000000L;
	cnv[0].range.length = 1000L;

	cnv[1].frequency=0.42;
	cnv[1].range.chrom = 2;
	cnv[1].range.position = 1200000L;
	cnv[1].range.length = 2000L;

	cnv[2].frequency=0.19;
	cnv[2].range.chrom = 3;
	cnv[2].range.position = 3000000L;
	cnv[2].range.length = 3000L;

	cnv[3].frequency=0.45;
	cnv[3].range.chrom = 4;
	cnv[3].range.position = 4000000L;
	cnv[3].range.length = 41000L;

	sqlite3 *database;
	int rc;
	sqlite3_open("testClustering.sqlite", &database);

	for(size_t i=0; i<4; i++)
		cnv[i].archiveObjectToDB(database);

	sqlite3_close(database);

	system("./cluster testClustering.sqlite");

	sqlite3_open("testClustering.sqlite", &database);

	SubcloneExplorer::EventCluster dummyCluster;

	std::vector<sqlite3_int64> allClusterID = dummyCluster.vecAllObjectsID(database);
	BOOST_CHECK(allClusterID.size() == 2);
}
