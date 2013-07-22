/**
 * @file Unit tests for class GenomicLocation
 *
 * @see GenomicLocation
 * @author Yi Qiao
 */
#include "GenomicLocation.h"
#include "common.h"

SUITE(testGenomicLocation)
{
	TEST(ObjectCreation) 
	{
		SubcloneExplorer::GenomicLocation loc;
		CHECK( true /* object creation should always pass */);
	}
}

TEST_MAIN
