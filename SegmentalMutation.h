#ifndef SEGMENTAL_MUTATION_H
#define SEGMENTAL_MUTATION_H

/**
 * @file SegmentalMutation.h
 * Interface description of data structure class SegmentalMutation
 * 
 * @author Yi Qiao
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
			virtual void bindObjectToStatement(sqlite3_stmt *);
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
	class CNV : public SegmentalMutation {};

	/**
	 * @brief Loss of Heterozygosity
	 *
	 * A loss of heterozygosity event is a segment of genome within which both chromosome
	 * exhibit the same DNA sequence
	 *
	 * @see SegmentalMutation
	 */
	class LOH : public SegmentalMutation {};
}

#endif
