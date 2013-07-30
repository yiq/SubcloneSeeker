/**
 * @file Unit tests for SomaticEvent subclasses
 *
 * @see SomaticEvent
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

#include <sqlite3/sqlite3.h>
#include <cstdio>
#include <iostream>

#include "SomaticEvent.h"
#include "SegmentalMutation.h"
#include "SNP.h"

#include "common.h"

SUITE(TestSomaticEvent) {
	TEST(CNV) {
		SubcloneExplorer::CNV cnv;

		CHECK_CLOSE( cnv.frequency, 0, 1e-3);
		CHECK(cnv.range.chrom==0);
		CHECK(cnv.range.position==0);
		CHECK(cnv.range.length==0);

		cnv.frequency=0.2;
		cnv.range.chrom=1;
		cnv.range.position = 1000000L;
		cnv.range.length = 1000L;

		CHECK_CLOSE(cnv.frequency, 0.2, 1e-3);
		CHECK(cnv.range.chrom==1);
		CHECK(cnv.range.position==1000000L);
		CHECK(cnv.range.length==1000L);

		SubcloneExplorer::CNV *cnv2 = new SubcloneExplorer::CNV();
		cnv2->frequency = 0.4;
		cnv2->range.chrom=1;
		cnv2->range.position = 1000123L;
		cnv2->range.length = 1012L;

		CHECK(cnv.isEqualTo(cnv2));

		SubcloneExplorer::LOH *loh = new SubcloneExplorer::LOH();
		loh->frequency = 0.4;
		loh->range.chrom=1;
		loh->range.position = 1000123L;
		loh->range.length = 1012L;

		CHECK(not cnv.isEqualTo(loh));


	}

	TEST_FIXTURE(DBFixture, CNVToDB) {
		SubcloneExplorer::CNV cnv;

		cnv.frequency=0.2;
		cnv.range.chrom=1;
		cnv.range.position = 1000000L;
		cnv.range.length = 1000L;

		CHECK_CLOSE(cnv.frequency, 0.2, 1e-3);
		CHECK(cnv.range.chrom==1);
		CHECK(cnv.range.position==1000000L);
		CHECK(cnv.range.length==1000L);

		// write
		cnv.createTableInDB(database);
		sqlite3_int64 id = cnv.archiveObjectToDB(database);

		CHECK(id != 0);

		// read
		SubcloneExplorer::CNV cnv2;
		bool status = cnv2.unarchiveObjectFromDB(database, id);

		CHECK(status);
		CHECK_CLOSE(cnv2.frequency, 0.2, 1e-3);
		CHECK(cnv2.range.chrom==1);
		CHECK(cnv2.range.position==1000000L);
		CHECK(cnv2.range.length==1000L);
	}

	TEST(LOH) {
		SubcloneExplorer::LOH loh;
		CHECK_CLOSE( loh.frequency, 0, 1e-3);
		CHECK(loh.range.chrom==0);
		CHECK(loh.range.position==0);
		CHECK(loh.range.length==0);

		loh.frequency=0.2;
		loh.range.chrom=1;
		loh.range.position = 1000000L;
		loh.range.length = 1000L;

		CHECK_CLOSE(loh.frequency, 0.2, 1e-3);
		CHECK(loh.range.chrom==1);
		CHECK(loh.range.position==1000000L);
		CHECK(loh.range.length==1000L);
	}

	TEST_FIXTURE(DBFixture, LOHToDB) {
		SubcloneExplorer::LOH loh;

		loh.frequency=0.2;
		loh.range.chrom=1;
		loh.range.position = 1000000L;
		loh.range.length = 1000L;

		CHECK_CLOSE(loh.frequency, 0.2, 1e-3);
		CHECK(loh.range.chrom==1);
		CHECK(loh.range.position==1000000L);
		CHECK(loh.range.length==1000L);

		// write
		loh.createTableInDB(database);
		sqlite3_int64 id = loh.archiveObjectToDB(database);

		CHECK(id != 0);

		// read
		SubcloneExplorer::LOH loh2;
		bool status = loh2.unarchiveObjectFromDB(database, id);

		CHECK(status);
		CHECK_CLOSE(loh2.frequency, 0.2, 1e-3);
		CHECK(loh2.range.chrom==1);
		CHECK(loh2.range.position==1000000L);
		CHECK(loh2.range.length==1000L);
	}

	TEST(SNP) {
		SubcloneExplorer::SNP snp;

		CHECK_CLOSE(snp.frequency, 0, 1e-3);
		CHECK(snp.location.chrom==0);
		CHECK(snp.location.position==0);


		snp.frequency=0.2;
		snp.location.chrom=1;
		snp.location.position = 1000000L;


		CHECK_CLOSE(snp.frequency, 0.2, 1e-3);
		CHECK(snp.location.chrom==1);
		CHECK(snp.location.position==1000000L);

	}

	TEST_FIXTURE(DBFixture, SNPToDB) {
		SubcloneExplorer::SNP snp;

		snp.frequency=0.2;
		snp.location.chrom=1;
		snp.location.position = 1000000L;

		CHECK_CLOSE(snp.frequency, 0.2, 1e-3);
		CHECK(snp.location.chrom==1);
		CHECK(snp.location.position==1000000L);

		// write
		snp.createTableInDB(database);
		sqlite3_int64 id = snp.archiveObjectToDB(database);

		CHECK(id != 0);

		// read
		SubcloneExplorer::SNP snp2;
		bool status = snp2.unarchiveObjectFromDB(database, id);

		CHECK(status);
		CHECK_CLOSE(snp2.frequency, 0.2, 1e-3);
		CHECK(snp2.location.chrom==1);
		CHECK(snp2.location.position==1000000L);

	}
}

int main() {
	return UnitTest::RunAllTests();
}
