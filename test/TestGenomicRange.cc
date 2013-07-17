/**
 * @file Unit tests for class GenomicLocation
 *
 * @see GenomicLocation
 * @author Yi Qiao
 */

#include <iostream>
#include <UnitTest++/src/UnitTest++.h>
#include "GenomicRange.h"

using namespace SubcloneExplorer;

SUITE(testGenomicRange) {
	TEST(ObjectCreation) {
		GenomicRange loc;
		CHECK( true /* object creation should always pass */);
	}

	TEST(RangeOverlap) {

		GenomicRange range[8];

		range[0].chrom=1;
		range[0].position=300;
		range[0].length=400;

		range[1].chrom=1;
		range[1].position=200;
		range[1].length=600;

		range[2].chrom=1;
		range[2].position=400;
		range[2].length=200;

		range[3].chrom=1;
		range[3].position=100;
		range[3].length=400;

		range[4].chrom=1;
		range[4].position=500;
		range[4].length=400;

		range[5].chrom=1;
		range[5].position=0;
		range[5].length=50;

		range[6].chrom=1;
		range[6].position=950;
		range[6].length=50;

		range[7].chrom=2;
		range[7].position=300;
		range[7].length=500;

		for(int i=0; i<5; i++) {
			for(int j=0; j<5; j++) {
				CHECK(range[i].overlaps(range[j]));
			}
			for(int j=5; j<8; j++) {
				CHECK(not range[i].overlaps(range[j]));
			}
		}

		CHECK(range[1] < range[0]);
		CHECK(range[2] > range[0]);
		CHECK(range[3] < range[0]);
		CHECK(range[4] > range[0]);
		CHECK(range[5] < range[0]);
		CHECK(range[6] > range[0]);
		CHECK(range[7] > range[0]);
	}

	TEST(RangeCompare) {
		GenomicRange range;
		range.chrom=2;
		range.position=1000;
		range.length=1000;

		GenomicLocation loc1, loc2, loc3, loc4;
		loc1.chrom=1; loc1.position = 2000;
		loc2.chrom=2; loc2.position=500;
		loc3.chrom=2; loc3.position=1500;
		loc4.chrom=3; loc4.position = 500;

		CHECK( range > loc1 );
		CHECK(not ( range < loc1 ));
		CHECK(range > loc2);
		CHECK(not ( range < loc2 ));
		CHECK(range < loc3);
		CHECK(not ( range > loc3 ));
		CHECK(range < loc4);
		CHECK(not ( range > loc4 ));
	}
}

int main() {
	return UnitTest::RunAllTests();
}
