/**
 * @file Unit tests for EventCluster
 *
 * @see EventCluster
 * @author Yi Qiao
 */

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <sqlite3/sqlite3.h>
#include <cstdio>

#include "../EventCluster.h"
#include "../SegmentalMutation.h"

void testEventCluster() {
	SubcloneExplorer::EventCluster cluster;

	// default vault check
	BOOST_CHECK_CLOSE(cluster.cellFraction(), 0, 1e-3);
	BOOST_CHECK(cluster.members().size() == 0);

	SubcloneExplorer::CNV cnv;

	cnv.frequency=0.2;
	cnv.range.start.chrom=1;
	cnv.range.start.position = 1000000L;
	cnv.range.length = 1000L;

	cluster.addEvent(&cnv);

	BOOST_CHECK_CLOSE(cluster.cellFraction(), 0.2, 1e-3);
	BOOST_CHECK(cluster.members().size() == 1);


	SubcloneExplorer::CNV cnv2;

	cnv2.frequency=0.3;
	cnv2.range.start.chrom=1;
	cnv2.range.start.position = 1000000L;
	cnv2.range.length = 1000L;

	cluster.addEvent(&cnv2);

	BOOST_CHECK_CLOSE(cluster.cellFraction(), 0.25, 1e-3);
	BOOST_CHECK(cluster.members().size() == 2);
}

void testEventClusterToDB() {
	SubcloneExplorer::EventCluster cluster;
	SubcloneExplorer::CNV cnv;

	cnv.frequency=0.2;
	cnv.range.start.chrom=1;
	cnv.range.start.position = 1000000L;
	cnv.range.length = 1000L;

	cluster.addEvent(&cnv);

	// write
	sqlite3 *database;
	sqlite3_open("test.sqlite", &database);

	cluster.createTableInDB(database);
	sqlite3_int64 id = cluster.archiveObjectToDB(database);

	BOOST_CHECK(id != 0);

	sqlite3_close(database);
	database = 0;

	// read
	SubcloneExplorer::EventCluster cluster2;
	sqlite3_open("test.sqlite", &database);

	bool status = cluster2.unarchiveObjectFromDB(database, id);

	BOOST_CHECK(status);
	BOOST_CHECK_CLOSE(cluster2.cellFraction(), 0.2, 1e-3);
	BOOST_CHECK(cluster2.members().size() == 0);

	sqlite3_close(database);

#ifndef KEEP_TEST_DB
	remove("test.sqlite");
#endif


}
