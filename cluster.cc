/*
 * @file cluster.cc
 * The main source file for the utility 'cluster', which reads
 * SomaticEvent objects from a given database, cluster them, and
 * save EventClusters into the database.
 *
 * @author Yi Qiao
 */

#include "SomaticEvent.h"
#include "SegmentalMutation.h"
#include "SNP.h"
#include "EventCluster.h"
#include <sqlite3/sqlite3.h>
#include <iostream>
#include <vector>
#include <cmath>

using namespace SubcloneExplorer;

typedef std::vector<SomaticEvent *> EventList_t;
typedef std::vector<EventCluster *> ClusterList_t;
typedef std::vector<sqlite3_int64> DBObjRecordList_t;


ClusterList_t clustering_running(const EventList_t &events, double threshold);

int main(int argc, char* argv[]) {
	if(argc<2) {
		std::cout<<"usage: "<<argv[0]<<" <event database>"<<std::endl;
		return(0);
	}

	sqlite3* src_database;
	int rc;

	rc = sqlite3_open_v2(argv[1], &src_database, SQLITE_OPEN_READWRITE, NULL);

	if( rc != SQLITE_OK ) {
		std::cerr<<"Unable to open database file "<<argv[1]<<std::endl;
		return(1);
	}

	// ****************************************
	// Read somatic mutations from the database
	// ****************************************
	
	EventList_t events;

	// 1. CNVs
	CNV dummyCNV;
	DBObjRecordList_t vecCNV = dummyCNV.vecAllObjectsID(src_database);
	for(size_t i=0; i<vecCNV.size(); i++) {
		CNV *newCNV = new CNV();
		newCNV->unarchiveObjectFromDB(src_database, vecCNV[i]);
		events.push_back(newCNV);
	}

	// 2. LOHs
	LOH dummyLOH;
	DBObjRecordList_t vecLOH = dummyLOH.vecAllObjectsID(src_database);
	for(size_t i=0; i<vecLOH.size(); i++) {
		LOH *newLOH = new LOH();
		newLOH->unarchiveObjectFromDB(src_database, vecLOH[i]);
		events.push_back(newLOH);
	}

	// 3. SNPs
	SNP dummySNP;
	DBObjRecordList_t vecSNP = dummySNP.vecAllObjectsID(src_database);
	for(size_t i=0; i<vecSNP.size(); i++) {
		SNP *newSNP = new SNP();
		newSNP->unarchiveObjectFromDB(src_database, vecSNP[i]);
		events.push_back(newSNP);
	}

	if(events.size() == 0) {
		std::cerr<<" No event has been found in the database"<<std::endl;
		return(2);
	}

	// *******************************
	// Invoke the clustering algorithm
	// *******************************
	
	ClusterList_t clusteringResult = EventCluster::clustering(events, 0.05);

	// *****************************************
	// Remove old clusters by dropping the table
	// *****************************************

	sqlite3_stmt *statement;

	// check if table exist
	std::string table_check_str = "DROP TABLE IF EXISTS Clusters;";
	rc = sqlite3_prepare_v2(src_database, table_check_str.c_str(), -1, &statement, 0);
	if(rc != SQLITE_OK) {
		sqlite3_finalize(statement);
		std::cerr<<"Cannot prepare sql statement for dropping existing cluster table"<<std::endl;
		return(3);
	}

	rc = sqlite3_step(statement);
	sqlite3_finalize(statement);

	if(rc != SQLITE_DONE) {
		std::cerr<<"Unable to drop existing cluster table"<<std::endl;
		return(3);
	}

	for(size_t i=0; i<clusteringResult.size(); i++) {
		sqlite3_int64 clusterID = clusteringResult[i]->archiveObjectToDB(src_database);
		for(size_t j=0; j<clusteringResult[i]->members().size(); j++) {
			clusteringResult[i]->members()[j]->setClusterID(clusterID);
			sqlite3_int64 retID = clusteringResult[i]->members()[j]->archiveObjectToDB(src_database);
		}
	}

	sqlite3_close(src_database);
	return(0);
}
