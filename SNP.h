#ifndef SNP_H
#define SNP_H

/**
 * @file SNP.h
 * Interface description of data structure class SNP
 * 
 * @author Yi Qiao
 */

#include "GenomicLocation.h"
#include "SomaticEvent.h"

namespace SubcloneExplorer {

	/**
	 * @brief Single Nucleotide Polymorphism
	 *
	 * A SNP is a point mutation at a specific location on the genome
	 * that the DNA nucleotide is different from a more common alternative
	 */
	class SNP : public SomaticEvent {
		public:
			virtual sqlite3_int64 archiveObjectToDB(sqlite3 *database);
			virtual bool unarchiveObjectFromDB(sqlite3 *database, sqlite3_int64 id);

		public:
			GenomicLocation location; /**< At which location did the SNP occurred */
	};
}

#endif
