#ifndef SEGMENTAL_MUTATION_H
#define SEGMENTAL_MUTATION_H

/**
 * @file SegmentalMutation.h
 * Interface description of data structure class SegmentalMutation
 * 
 * @author Yi Qiao
 */

#include "GenomicRange.h"

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
		public:
			GenomicRange range; /**< Genomic range over which the mutation occurred */
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
