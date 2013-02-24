#include <boost/test/included/unit_test.hpp>

#include "TestSomaticEvent.h"

void testGenomicLocation();
void testGenomicRange();

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[]) {

	// add test cases with the following syntax
	// framework::master_test_suite().add( BOOST_TEST_CASE( &function_name ));
	
	framework::master_test_suite().add( BOOST_TEST_CASE( &testGenomicLocation ));
	framework::master_test_suite().add( BOOST_TEST_CASE( &testGenomicRange ));
	framework::master_test_suite().add( BOOST_TEST_CASE( &testCNV ));
	framework::master_test_suite().add( BOOST_TEST_CASE( &testLOH ));
	framework::master_test_suite().add( BOOST_TEST_CASE( &testSNP ));

	return 0;
}
