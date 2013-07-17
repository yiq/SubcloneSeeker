/**
 * @file Unit tests for class GenomicLocation
 *
 * @see GenomicLocation
 * @author Yi Qiao
 */

#include <UnitTest++/src/UnitTest++.h>
#include "GenomicLocation.h"

SUITE(testGenomicLocation)
{
	TEST(ObjectCreation) 
	{
		SubcloneExplorer::GenomicLocation loc;
		CHECK( true /* object creation should always pass */);
	}
}


int main() {
	return UnitTest::RunAllTests();
}
