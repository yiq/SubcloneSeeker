/*
 * @file segtxt2db.cc
 * The main source for the utility 'segtxt2db', which reads in
 * .seg.txt file and save the events into a sqlite database
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

#include <iostream>
#include <fstream>
#include <cmath>
#include <getopt.h>
#include <cstdlib>
#include <cstring>

#include "SomaticEvent.h"
#include "SegmentalMutation.h"
#include "EventCluster.h"
#include "RefGenome.h"

#define _EPISLON 1e-3

#define CORR_AUTO 1
#define CORR_PROXIMITY 2

static int _ploidy;
static double _purity;
static double _neutral_level;
static int _correct_model;
static double _threshold;
static char *_prog_name;
static char *_mask_fn;
static unsigned long _min_length;

using namespace SubcloneSeeker;

void SegmentalMeanCorrection(std::vector<EventCluster *>& clusters);
void SegmentalMean2Frequency(std::vector<EventCluster *>& clusters);
double SegmentalMeanModal(const EventClusterPtr_vec& clusters);
void printClusters(std::vector<EventCluster *>& clusters)  {
	for(size_t i=0; i<clusters.size(); i++) {
		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			CNV *theEvent = dynamic_cast<CNV*>(clusters[i]->members()[j]);
			if(theEvent == NULL)
				continue;
			//std::cout<<"\tchr"<<theEvent->range.chrom<<"\t"<<theEvent->range.position<<"\t\t"<<theEvent->range.position + theEvent->range.length<<std::endl;
			theEvent->frequency = clusters[i]->cellFraction();
		}

		if(clusters[i]->cellFraction() < 1)
			std::cout<<"Cluster "<<i<<"\t frequency: "<<clusters[i]->cellFraction()<<std::endl;
	}
}

void usage() {
	std::cout<<"Usage: "<<_prog_name<<" <seg.txt file> <result database>"<<std::endl;
	std::cout<<"\t\t Options:"<<std::endl;
	std::cout<<"\t\t -p purity\t[default = 1]\t\tA number between 0-1 specifying the purity of the sample"<<std::endl;
	std::cout<<"\t\t -q ploidy\t[default = 2]\t\tA integer number specifying the ploidy of the copy number neutral regions"<<std::endl;
	std::cout<<"\t\t -n ratio\t[default = 1]\t\tA tumor/normal ratio at where the copy number neutral regions are found"<<std::endl;
	std::cout<<"\t\t -m model\t[default = 2]\t\tFraction correction model. 1=MODAL, 2=PROXIMITY"<<std::endl;
	std::cout<<"\t\t -r mask-file\t\t\t\tA mask file for regions to exclude"<<std::endl;
	std::cout<<"\t\t -t threshold\t[default = 0.05]\tThe ratio threshold for merging two segments into a cluster"<<std::endl;
	std::cout<<"\t\t -e length\t[default=0]\tThe minimal cumulative length of a cluster to be included in the result"<<std::endl;
	exit(0);
}

int main(int argc, char* argv[]) {
	_prog_name = *argv;
	_ploidy = 2;
	_purity = 1;
	_neutral_level = 1;
	_correct_model = CORR_PROXIMITY;
	_threshold = 0.05;
	_mask_fn=NULL;
	_min_length = 0;

	int c;
	while((c = getopt(argc, argv, "p:q:n:m:r:t:e:h")) != -1) {
		switch(c) {
			case 'p':
				_purity = atof(optarg); break;
			case 'q':
				_ploidy = atoi(optarg); break;
			case 'n':
				_neutral_level = atof(optarg); break;
			case 'm':
				_correct_model = atoi(optarg); break;
			case 'r':
				_mask_fn = strdup(optarg); break;
			case 't':
				_threshold = atof(optarg); break;
			case 'e':
				_min_length = atoi(optarg); break;
			default:
				std::cerr<<"Unrecognized option "<<(char)c<<std::endl;
				usage();
				break;
		}
	}

	argc -= optind; argv += optind;

	if(argc < 2) {
		usage();
	}

	RefGenome *refGenome = RefGenome::getInstance();

	// open mask file if supplied
	SomaticEventPtr_vec maskEvents;

	if(_mask_fn != NULL) {
		std::ifstream in_mask_file;
		in_mask_file.open(_mask_fn);
		if(!in_mask_file.is_open()) {
			std::cerr<<"Unable to open mask file "<<_mask_fn<<std::endl;
			return(1);
		}

		std::string chrom;
		long startLoc, endLoc;

		in_mask_file >> chrom >> startLoc >> endLoc;
		while(!in_mask_file.eof()) {
			CNV *cnv = new CNV();
			cnv->range.chrom = refGenome->queryChromID(chrom);
			cnv->range.position = startLoc;
			cnv->range.length = endLoc - startLoc;

			maskEvents.push_back(cnv);
			in_mask_file >> chrom >> startLoc >> endLoc;
		}

		in_mask_file.close();
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

	// read event lists
	std::vector<SomaticEvent *> events;
	std::string id, chrom;
	long startLoc, endLoc, numMark;
	double segMean;

	in_segtxt_file >> id >> id >> id >> id >> id >> id; // Skip the header line
	while(!in_segtxt_file.eof()) {
		in_segtxt_file >> id >> chrom >> startLoc >> endLoc >> numMark >> segMean;
		if(in_segtxt_file.eof())
			break;

		segMean = pow(2, segMean);
		
		CNV *cnv = new CNV();
		cnv->range.chrom = refGenome->queryChromID(chrom);
		cnv->range.position = startLoc;
		cnv->range.length = endLoc - startLoc;
		cnv->frequency = segMean;

		bool masked = false;
		for(size_t i=0; i<maskEvents.size(); i++) {
			CNV *otherEvent = dynamic_cast<CNV*>(maskEvents[i]);
			if(otherEvent == NULL) continue;
			if(otherEvent->range.overlaps(cnv->range)) {
				masked = true;
				break;
			}
		}

		if(not masked) events.push_back(cnv);
	}
	in_segtxt_file.close();

	// ********************
	// Open output database
	// ********************

	sqlite3 *database;
	if(sqlite3_open(*argv, &database) != SQLITE_OK) {
		std::cerr<<"Unable to open database for writing result"<<std::endl;
		return(1);
	}

	// *******************************
	// Cluster the CNVs based on ratio
	// *******************************
	std::vector<EventCluster *> clusters = EventCluster::clustering(events, _threshold);

	// ************************************************
	// Correct the clusters by purity and neutral level
	// ************************************************
	
	// -------> If correction mode is automatic, find the modal neutral level <------
	
	if(_correct_model == CORR_AUTO) 
		_neutral_level = SegmentalMeanModal(clusters);

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

		unsigned long cumLen = 0;
		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			SegmentalMutation *theSeg = dynamic_cast<SegmentalMutation *>(clusters[i]->members()[j]);
			if(theSeg == NULL)
				cumLen += 1;
			else
				cumLen += theSeg->range.length;
		}
		if(cumLen < _min_length) {
			std::cerr<<"cluster "<<i<<" removed because too short"<<std::endl;
			continue;
		}

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

	double corrRatio = clusters[closestClusterIdx]->cellFraction();

	std::cerr<<"correcting clusters to "<<corrRatio<<std::endl;

	// --------> First pass, center around the neutral cluster <-------- //
	for(size_t i=0; i<clusters.size(); i++) {
		clusters[i]->setCellFraction(clusters[i]->cellFraction() / corrRatio);
		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			clusters[i]->members()[j]->frequency /= corrRatio;
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

double SegmentalMeanModal(const EventClusterPtr_vec& clusters) {
	unsigned long maxLen = 0;
	size_t maxLenIdx = 0;


	for(size_t i=0; i<clusters.size(); i++) {
		unsigned long len = 0;
		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			CNV *member = dynamic_cast<CNV *>(clusters[i]->members()[j]);
			if(member != NULL)
				len += member->range.length;
		}

		if(len > maxLen) {
			maxLen = len;
			maxLenIdx = i;
		}
	}
	std::cerr<<"correcting cluster len "<<maxLenIdx<<std::endl;

	return clusters[maxLenIdx]->cellFraction();
}

void SegmentalMean2Frequency(std::vector<EventCluster *>& clusters) {
	std::vector<size_t> toBeRemoved;
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

		if(cnDelta > 0) {
			toBeRemoved.push_back(i);
		}

		if(cnDelta < -0.5 && _mask_fn==NULL) {
			//output mask
			for(size_t j=0; j<clusters[i]->members().size(); j++) {
				CNV * member = dynamic_cast<CNV*>(clusters[i]->members()[j]);
				if(member != NULL) {
					std::cerr<<member->range.chrom<<"\t"<<member->range.position<<"\t"<<member->range.position+member->range.length<<std::endl;
				}
			}
		}
		
		if(_mask_fn != NULL)
			std::cerr<<"setting cluster "<<clusters[i]->getId()<<" fraction to "<<freq<<std::endl;
		
		clusters[i]->setCellFraction(freq);

		for(size_t j=0; j<clusters[i]->members().size(); j++) {
			clusters[i]->members()[j]->frequency = freq;
		}
	}

	for(int i=toBeRemoved.size()-1; i>=0; i--) {
		clusters.erase(clusters.begin() + toBeRemoved[i]);
	}
}
