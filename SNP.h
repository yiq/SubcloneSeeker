#ifndef SNP_H
#define SNP_H

/**
 * @file SNP.h
 * Interface description of data structure class SNP
 * 
 * @author Yi Qiao
 */

#include "GenomicLocation.h"

namespace SubcloneExplorer {

	/**
	 * @brief Single Nucleotide Polymorphism
	 *
	 * A SNP is a point mutation at a specific location on the genome
	 * that the DNA nucleotide is different from a more common alternative
	 */
	class SNP : SomaticEvent {
		public:
			GenomicLocation location;
	};
}

#endif
