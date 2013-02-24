/**
 * @file Unit tests for SomaticEvent subclasses
 *
 * @see SomaticEvent
 * @author Yi Qiao
 */

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "../SomaticEvent.h"
#include "../SegmentalMutation.h"
#include "../SNP.h"

void testCNV() {
	SubcloneExplorer::CNV cnv;
	BOOST_CHECK( true /* object creation should always pass */);
}

void testLOH() {
	SubcloneExplorer::LOH loh;
	BOOST_CHECK( true /* object creation should always pass */);
}

void testSNP() {
	SubcloneExplorer::SNP snp;
	BOOST_CHECK( true /* object creation should always pass */);
}
