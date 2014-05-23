/**
 * @file Unit tests for EventCluster
 *
 * @see EventCluster
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
#include <sqlite3/sqlite3.h>
#include <cstdio>

#include "EventCluster.h"
#include "SegmentalMutation.h"

#include "common.h"

SUITE(testEventCluster) 
{
	TEST(ObjectCreation) {
		SubcloneSeeker::EventCluster cluster;

		// default vault check
		CHECK_CLOSE(cluster.cellFraction(), 0, 1e-3);
		CHECK(cluster.members().size() == 0);

		SubcloneSeeker::CNV cnv;

		cnv.frequency=0.2;
		cnv.range.chrom=1;
		cnv.range.position = 1000000L;
		cnv.range.length = 1000L;

		cluster.addEvent(&cnv);

		CHECK_CLOSE(cluster.cellFraction(), 0.2, 1e-3);
		CHECK(cluster.members().size() == 1);


		SubcloneSeeker::CNV cnv2;

		cnv2.frequency=0.3;
		cnv2.range.chrom=1;
		cnv2.range.position = 1000000L;
		cnv2.range.length = 1000L;

		cluster.addEvent(&cnv2);

		CHECK_CLOSE(cluster.cellFraction(), 0.25, 1e-3);
		CHECK(cluster.members().size() == 2);
	}

	TEST(EventClusterComparator) {
		SubcloneSeeker::EventCluster cluster1, cluster2;
		SubcloneSeeker::CNV cnv1, cnv2;

		cnv1.frequency = 0.2;
		cnv1.range.length = 1000L;
		cnv2.frequency = 0.3;
		cnv2.range.length = 1000L;

		cluster1.addEvent(&cnv1);
		cluster2.addEvent(&cnv2);

		CHECK(cluster1 < cluster2);
		CHECK(cluster2 > cluster1);
	}

	TEST(EventClusterToDB) {
		SubcloneSeeker::EventCluster cluster;
		SubcloneSeeker::CNV cnv;

		cnv.frequency=0.2;
		cnv.range.chrom=1;
		cnv.range.position = 1000000L;
		cnv.range.length = 1000L;

		cluster.addEvent(&cnv);

		// write
		sqlite3 *database;
		sqlite3_open("test.sqlite", &database);

		cluster.createTableInDB(database);
		sqlite3_int64 id = cluster.archiveObjectToDB(database);

		CHECK(id != 0);

		sqlite3_close(database);
		database = 0;

		// read
		SubcloneSeeker::EventCluster cluster2;
		sqlite3_open("test.sqlite", &database);

		bool status = cluster2.unarchiveObjectFromDB(database, id);

		CHECK(status);
		CHECK_CLOSE(cluster2.cellFraction(), 0.2, 1e-3);
		CHECK(cluster2.members().size() == 0);

		sqlite3_close(database);

#ifndef KEEP_TEST_DB
		remove("test.sqlite");
#endif


	}
}

TEST_MAIN
