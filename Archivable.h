#ifndef ARCHIVABLE_H
#define ARCHIVABLE_H

#include <string>
#include <sqlite3.h>

/**
 * @file Archivable.h
 * Interface description of the abstract class Archivable
 *
 * @author Yi Qiao
 */

namespace SubcloneExplorer{

	/**
	 * @brief Abstract class that defines the interface to handle archiving objects into sqlite3 database
	 * 
	 * This abstract class defines the required behaviors when handling object archiving to and from a
	 * sqlite3 database, which will be used by the project to store computation results. As of now four
	 * methods are required for any class that wishes to support archiving:
	 *   1. return a string describing the name of the table
	 *   2. create the table in a given database
	 *   3. store an object into the table
	 *   4. retrieve an object from the table with an identifier
	 *
	 * The unarchiving procedure uses an integer id to determine which database record is to be used for
	 * unarchiving. This would require that an SERIAL column exists in the table.
	 */
	class Archivable {
		protected:
			/**
			 * returns the name of the table in which all object of a specific class are stored
			 * @return Table name
			 */
			virtual std::string getTableName() = 0;

			/**
			 * Create the storage table in the database
			 * @param database An open sqlite3 database connection handle
			 * @return Whether the operation is successful or not
			 */
			virtual bool createTableInDB(sqlite3 *database) = 0;

			/**
			 * Archive the object into the database
			 * @param database An open sqlite3 database connection handle
			 * @return The id of the newly inserted object, if successful; or -1 if error occurred
			 */
			virtual sqlite3_int64 archiveObjectToDB(sqlite3 *database) = 0;

			/**
			 * Unarchive an object from the database
			 * @param database An open sqlite3 database connection handle
			 * @param id The identifier with which the correct record is to be found
			 * @return Whether the operation is successful or not
			 */
			virtual bool unarchiveObjectFromDB(sqlite3 *database, sqlite3_int64 id) = 0;
	};
}

#endif
