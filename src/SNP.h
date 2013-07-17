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
		protected:
			// Implements Archivable
			virtual std::string getTableName();
			virtual std::string createObjectStatementStr();
			virtual std::string updateObjectStatementStr();
			virtual std::string selectObjectColumnListStr();
			virtual int bindObjectToStatement(sqlite3_stmt *);
			virtual void updateObjectFromStatement(sqlite3_stmt *);


		public:
			GenomicLocation location; /**< At which location did the SNP occurred */
	};
}

#endif
