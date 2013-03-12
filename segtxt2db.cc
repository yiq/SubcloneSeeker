/*
 * @file segtxt2db.cc
 * The main source for the utility 'segtxt2db', which reads in
 * .seg.txt file and save the events into a sqlite database
 *
 * @author Yi Qiao
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <getopt.h>

#include "SomaticEvent.h"
#include "SegmentalMutation.h"
#include "EventCluster.h"
#include "RefGenome.h"

#define _EPISLON 1e-3

static int _ploidy;
static double _purity;
static double _neutral_level;
static double _threshold;
static char *_prog_name;

using namespace SubcloneExplorer;

void SegmentalMeanCorrection(std::vector<EventCluster *>& clusters);
void SegmentalMean2Frequency(std::vector<EventCluster *>& clusters);

void usage() {
	std::cout<<"Usage: "<<_prog_name<<" <seg.txt file> <result database>"<<std::endl;
	std::cout<<"\t\t Options:"<<std::endl;
	std::cout<<"\t\t -p purity\t[default = 1]\t\tA number between 0-1 specifying the purity of the sample"<<std::endl;
	std::cout<<"\t\t -q ploidy\t[default = 2]\t\tA integer number specifying the ploidy of the copy number neutral regions"<<std::endl;
	std::cout<<"\t\t -n ratio\t[default = 1]\t\tA tumor/normal ratio at where the copy number neutral regions are found"<<std::endl;
	std::cout<<"\t\t -t threshold\t[default = 0.05]\tThe ratio threshold for merging two segments into a cluster"<<std::endl;
	exit(0);
}

int main(int argc, char* argv[]) {
	_prog_name = *argv;
	_ploidy = 2;
	_purity = 1;
	_neutral_level = 1;
	_threshold = 0.05;

	int c;
	while((c = getopt(argc, argv, "p:q:n:t:")) != -1) {
		switch(c) {
			case 'p':
				_purity = atof(optarg); break;
			case 'q':
				_ploidy = atoi(optarg); break;
			case 'n':
				_neutral_level = atof(optarg); break;
			case 't':
				_threshold = atof(optarg); break;
			default:
				break;
		}
	}

	argc -= optind; argv += optind;

	if(argc < 2) {
		usage();
	}

	// *************************************
	// Read content of .seg.txt file as CNVs
	// *************************************
	std::ifstream in_segtxt_file;
	in_segtxt_file.open(*argv);
	if(!in_segtxt_file.is_open()) {
		perror("Unable to open seg.txt file");
		return(1);
	}

	argc--; argv++;

	// ********************
	// Open output database
	// ********************

	sqlite3 *database;
	if(sqlite3_open(*argv, &database) != SQLITE_OK) {
		std::cerr<<"Unable to open database for writing result"<<std::endl;
		return(1);
	}

	std::vector<SomaticEvent *> events;

	std::string id, chrom;
	long startLoc, endLoc, numMark;
	double segMean;

	RefGenome *refGenome = RefGenome::getInstance();

	in_segtxt_file >> id >> id >> id >> id >> id >> id; // Skip the header line
	while(!in_segtxt_file.eof()) {
		in_segtxt_file >> id >> chrom >> startLoc >> endLoc >> numMark >> segMean;
		if(in_segtxt_file.eof())
			break;

		segMean = pow(2, segMean);
		
		// ignoring amplifications
		if(segMean > _neutral_level)
			continue;

		CNV *cnv = new CNV();
		cnv->range.chrom = refGenome->queryChromID(chrom);
		cnv->range.position = startLoc;
		cnv->range.length = endLoc - startLoc;
		cnv->frequency = segMean;

		events.push_back(cnv);
	}

	// *******************************
	// Cluster the CNVs based on ratio
	// *******************************
	std::vector<EventCluster *> clusters = EventCluster::clustering(events, _threshold);

	// ************************************************
	// Correct the clusters by purity and neutral level
	// ************************************************
	SegmentalMeanCorrection(clusters);

	// ************************
	// Calculate Cell Frequency
	// ************************
	SegmentalMean2Frequency(clusters);

	// ****************
	// Save the results
	// ****************
	for(size_t i=0; i<clusters.size(); i++) {
		// do not save neutral segments
		if(clusters[i]->cellFraction() < _EPISLON)
			continue;
		sqlite3_int64 newClusterID = clusters[i]->archiveObjectToDB(database);
		if(newClusterID == -1) {
			std::cerr<<"Error occurred while writing cluster "<<i<<" into database"<<std::endl;
		}
		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			clusters[i]->members()[j]->setClusterID(newClusterID);
			clusters[i]->members()[j]->archiveObjectToDB(database);
		}
	}

	sqlite3_close(database);
	return(0);
}

void SegmentalMeanCorrection(std::vector<EventCluster *>& clusters) {
	// identify the cluster which is copy number neutral
	size_t closestClusterIdx = 0;
	double closestClusterDiff = -1;
	for(size_t i=0; i<clusters.size(); i++) {
		double diff = fabs(clusters[i]->cellFraction() - _neutral_level);
		if(closestClusterDiff == -1 || closestClusterDiff > diff) {
			closestClusterDiff = diff;
			closestClusterIdx = i;
		}
	}

	std::cerr<<"correcting clusters to "<<clusters[closestClusterIdx]->cellFraction()<<std::endl;

	// --------> First pass, center around the neutral cluster <-------- //

	for(size_t i=0; i<clusters.size(); i++) {
		clusters[i]->setCellFraction(clusters[i]->cellFraction() / clusters[closestClusterIdx]->cellFraction());
		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			clusters[i]->members()[j]->frequency /= clusters[closestClusterIdx]->cellFraction();
		}
	}

	// --------> Correct for purity and ploidy <-------- //
	/*
	 * ActualMean * Purity + 2/Ploidy*(1-Purity) = ObservedMean
	 * ActualMean = [ ObservedMean - 2/Ploidy * (1-Purity) ] / Purity
	 */
	for(size_t i=0; i<clusters.size(); i++) {
		double newMean = (clusters[i]->cellFraction() - (2/(double)_ploidy) * (1-_purity)) / _purity;
		clusters[i]->setCellFraction(newMean);

		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			newMean = (clusters[i]->members()[j]->frequency - (2/(double)_ploidy) * (1-_purity)) / _purity;
			clusters[i]->members()[j]->frequency = newMean;
		}
	}
}

void SegmentalMean2Frequency(std::vector<EventCluster *>& clusters) {
	for(size_t i=0; i<clusters.size(); i++) {
		double offset = clusters[i]->cellFraction() - 1;
		double cnDelta = ceil(fabs(offset)*_ploidy)/(double)_ploidy;
		double freq;

		if(offset<0)
			cnDelta = -cnDelta;
		
		if(fabs(offset) < _EPISLON || fabs(cnDelta) < _EPISLON)
			freq = 0;
		else
			freq = offset / cnDelta;

		std::cerr<<"setting cluster fraction to "<<freq<<std::endl;
		clusters[i]->setCellFraction(freq);
		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			clusters[i]->members()[j]->frequency = freq;
		}
	}
}
