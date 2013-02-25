/**
 * @file Unit tests for class GenomicLocation
 *
 * @see GenomicLocation
 * @author Yi Qiao
 */

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "../GenomicRange.h"

using namespace SubcloneExplorer;

void testGenomicRange() {
	GenomicRange loc;
	BOOST_CHECK( true /* object creation should always pass */);
}

void testGenomicRangeOverlaps() {

	GenomicRange range[8];

	range[0].start.chrom=1;
	range[0].start.position=300;
	range[0].length=400;

	range[1].start.chrom=1;
	range[1].start.position=200;
	range[1].length=600;

	range[2].start.chrom=1;
	range[2].start.position=400;
	range[2].length=200;

	range[3].start.chrom=1;
	range[3].start.position=100;
	range[3].length=400;

	range[4].start.chrom=1;
	range[4].start.position=500;
	range[4].length=400;

	range[5].start.chrom=1;
	range[5].start.position=0;
	range[5].length=50;

	range[6].start.chrom=1;
	range[6].start.position=950;
	range[6].length=50;

	range[7].start.chrom=2;
	range[7].start.position=300;
	range[7].length=500;

	for(int i=0; i<5; i++) {
		for(int j=0; j<5; j++) {
			BOOST_CHECK(range[i].overlaps(range[j]));
		}
		for(int j=5; j<8; j++) {
			BOOST_CHECK(not range[i].overlaps(range[j]));
		}
	}

	BOOST_CHECK(range[1] < range[0]);
	BOOST_CHECK(range[2] > range[0]);
	BOOST_CHECK(range[3] < range[0]);
	BOOST_CHECK(range[4] > range[0]);
	BOOST_CHECK(range[5] < range[0]);
	BOOST_CHECK(range[6] > range[0]);
	BOOST_CHECK(range[7] > range[0]);
}
