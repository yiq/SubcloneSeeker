#ifndef SNP_H
#define SNP_H

/**
 * @file SNP.h
 * Interface description of data structure class SNP
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

#include "GenomicLocation.h"
#include "SomaticEvent.h"

namespace SubcloneSeeker {

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
