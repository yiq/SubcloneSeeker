#ifndef SEGMENTAL_MUTATION_H
#define SEGMENTAL_MUTATION_H

/**
 * @file SegmentalMutation.h
 * Interface description of data structure class SegmentalMutation
 * 
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

#include "GenomicRange.h"
#include "SomaticEvent.h"

namespace SubcloneExplorer {

	/**
	 * @brief Abstract data structure class that represents any segmental somatic mutation
	 *
	 * Mutations described by this class are segmental mutations, which means that they
	 * happen over a range of continuous genomic location. Such range is described by
	 * GenomicRange
	 *
	 * @see GenomicRange
	 */
	class SegmentalMutation : public SomaticEvent{
		protected:
			// Implements Archivable
			virtual std::string createObjectStatementStr();
			virtual std::string updateObjectStatementStr();
			virtual std::string selectObjectColumnListStr();
			virtual int bindObjectToStatement(sqlite3_stmt *);
			virtual void updateObjectFromStatement(sqlite3_stmt *);

		public:
			GenomicRange range; /**< Genomic range over which the mutation occurred */

			/**
			 * minimal constructor to reset all member variables 
			 */
			SegmentalMutation() : SomaticEvent(), range() {}


	};

	/**
	 * @brief Copy Number Variation
	 *
	 * A copy number variation is a segment of amplified or deleted genomic region.
	 *
	 * @see SegmentalMutation
	 */
	class CNV : public SegmentalMutation {
		protected:
			// Overwrite Archivable tableName
			virtual std::string getTableName();

		public:
			// Override isEqualTo
			virtual bool isEqualTo(SomaticEvent * anotherEvent, unsigned long resolution=10000L);
	};

	/**
	 * @brief Loss of Heterozygosity
	 *
	 * A loss of heterozygosity event is a segment of genome within which both chromosome
	 * exhibit the same DNA sequence
	 *
	 * @see SegmentalMutation
	 */
	class LOH : public SegmentalMutation {
		protected:
			// Overwrite Archivable tableName
			virtual std::string getTableName();
	
	};
}

#endif
