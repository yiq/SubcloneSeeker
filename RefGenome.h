#ifndef REFGENOME_H
#define REFGENOME_H

#include <string>
#include <map>
#include <vector>


class RefGenome {
public:
	static RefGenome * getInstance();
	int queryChromID(std::string chrom);
	size_t queryChromLengthWithID(int chromID);
	size_t queryGenomeLength();
	size_t queryChromStartBase(int chromID);
	const std::vector<std::string>& vec_chroms() { return _chromNames; }
	const std::vector<int> & vec_chromIDs() { return _chromIDs; }

protected:
	RefGenome();
	static RefGenome * _refGenome;
	std::map<int, size_t> _chromLengthMap;
	std::vector<std::string> _chromNames;
	std::vector<int> _chromIDs;
	
};


#endif