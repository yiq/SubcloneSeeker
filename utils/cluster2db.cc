#include "Subclone.h"
#include "EventCluster.h"
#include "SegmentalMutation.h"
#include <iostream>
#include <cstdio>
#include <sqlite3/sqlite3.h>
#include <cstdlib>
#include <cstring>

using namespace SubcloneSeeker;

int main(int argc, char* argv[]) {
	if(argc < 2) {
		std::cout<<"Usage "<<argv[0]<<" <Cluster-List-File>"<<std::endl;
		return(0);
	}

	FILE *in_file;
	in_file = fopen(argv[1], "r");
	if(in_file == NULL) {
		std::cerr<<"Unable to open file "<<argv[1]<<" for read"<<std::endl;
		return(1);
	}

	char *pri_db_fn = (char *)malloc(strlen(argv[1]) + 20);
	char *rel_db_fn = (char *)malloc(strlen(argv[1]) + 20);

	strcpy(pri_db_fn, argv[1]);
	strcat(pri_db_fn, "-pri.sqlite");

	strcpy(rel_db_fn, argv[1]);
	strcat(rel_db_fn, "-rel.sqlite");

	sqlite3 *pri_database;
	if(sqlite3_open(pri_db_fn, &pri_database) != SQLITE_OK) {
		std::cerr<<"Unable to create primary database file "<<pri_db_fn<<std::endl;
		return(2);
	}

	sqlite3 *rel_database;
	if(sqlite3_open(rel_db_fn, &rel_database) != SQLITE_OK) {
		std::cerr<<"Unable to create relapse database file "<<rel_db_fn<<std::endl;
		return(2);
	}

	float maxPriFreq, maxRelFreq;
	float priFreq, relFreq;

	maxPriFreq = -1;
	maxRelFreq = -1;

	int counter = 1;

	std::vector<CNV *> vec_primary_snps;
	std::vector<CNV *> vec_relapse_snps;

	while(!feof(in_file)) {
		if(fscanf(in_file, "%f %f", &priFreq, &relFreq) != 2)
			break;
		std::cerr<<"pri and rel freq "<<priFreq<<","<<relFreq<<" read"<<std::endl;
		if(maxPriFreq < priFreq)
			maxPriFreq = priFreq;
		if(maxRelFreq < relFreq)
			maxRelFreq = relFreq;

		CNV *priSNP, *relSNP;

		priSNP = new CNV(); relSNP = new CNV();
		
		priSNP->range.chrom = counter;
		priSNP->frequency = priFreq;

		relSNP->range.chrom = counter;
		relSNP->frequency = relFreq;

		vec_primary_snps.push_back(priSNP);
		vec_relapse_snps.push_back(relSNP);

		counter++;
	}

	fclose(in_file);

	if(maxPriFreq < 1e-3 || maxRelFreq < 1e-3) {
		std::cerr<<"Primary or Relapse has no cluster with a greater than 0 frequency"<<std::endl;
		return(3);
	}

	for(size_t i=0; i<vec_primary_snps.size(); i++) {
		CNV* priSNP = dynamic_cast<CNV *>(vec_primary_snps[i]);
		CNV* relSNP = dynamic_cast<CNV *>(vec_relapse_snps[i]);
#ifndef NO_NORMALIZE
		priSNP->frequency /= maxPriFreq;
		relSNP->frequency /= maxRelFreq;
#endif

		if(priSNP->frequency > 0.05) {
			EventCluster newCluster;
			newCluster.addEvent(priSNP, true);
			newCluster.setCellFraction(priSNP->frequency);
			newCluster.archiveObjectToDB(pri_database);
			priSNP->setClusterID(newCluster.getId());
			priSNP->archiveObjectToDB(pri_database);
		}


		if(relSNP->frequency > 0.05) {
			EventCluster newCluster;
			newCluster.addEvent(relSNP, true);
			newCluster.setCellFraction(relSNP->frequency);
			newCluster.archiveObjectToDB(rel_database);
			relSNP->setClusterID(newCluster.getId());
			relSNP->archiveObjectToDB(rel_database);
		}
	}

	sqlite3_close(pri_database);
	sqlite3_close(rel_database);
	return(0);
}
