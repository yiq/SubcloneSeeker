#include "RefGenome.h"

RefGenome * RefGenome::_refGenome = NULL;

RefGenome::RefGenome() 
{
	/* define human chromosomes */
	
	_chromNames.push_back("chr1");
	_chromIDs.push_back(queryChromID("chr1"));
	_chromNames.push_back("chr2");
	_chromIDs.push_back(queryChromID("chr2"));
	_chromNames.push_back("chr3");
	_chromIDs.push_back(queryChromID("chr3"));
	_chromNames.push_back("chr4");
	_chromIDs.push_back(queryChromID("chr4"));
	_chromNames.push_back("chr5");
	_chromIDs.push_back(queryChromID("chr5"));
	_chromNames.push_back("chr6");
	_chromIDs.push_back(queryChromID("chr6"));
	_chromNames.push_back("chr7");
	_chromIDs.push_back(queryChromID("chr7"));
	_chromNames.push_back("chr8");
	_chromIDs.push_back(queryChromID("chr8"));
	_chromNames.push_back("chr9");
	_chromIDs.push_back(queryChromID("chr9"));
	_chromNames.push_back("chr10");
	_chromIDs.push_back(queryChromID("chr10"));
	_chromNames.push_back("chr11");
	_chromIDs.push_back(queryChromID("chr11"));
	_chromNames.push_back("chr12");
	_chromIDs.push_back(queryChromID("chr12"));
	_chromNames.push_back("chr13");
	_chromIDs.push_back(queryChromID("chr13"));
	_chromNames.push_back("chr14");
	_chromIDs.push_back(queryChromID("chr14"));
	_chromNames.push_back("chr15");
	_chromIDs.push_back(queryChromID("chr15"));
	_chromNames.push_back("chr16");
	_chromIDs.push_back(queryChromID("chr16"));
	_chromNames.push_back("chr17");
	_chromIDs.push_back(queryChromID("chr17"));
	_chromNames.push_back("chr18");
	_chromIDs.push_back(queryChromID("chr18"));
	_chromNames.push_back("chr19");
	_chromIDs.push_back(queryChromID("chr19"));
	_chromNames.push_back("chr20");
	_chromIDs.push_back(queryChromID("chr20"));
	_chromNames.push_back("chr21");
	_chromIDs.push_back(queryChromID("chr21"));
	_chromNames.push_back("chr22");
	_chromIDs.push_back(queryChromID("chr22"));
	_chromNames.push_back("chrX");
	_chromIDs.push_back(queryChromID("chrX"));
	_chromNames.push_back("chrY");
	_chromIDs.push_back(queryChromID("chrY"));
	
	
	/* define the length of human chromosomes : hg19 */
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr1"), 249904550));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr2"), 243199373));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr3"), 198022430));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr4"), 191535534));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr5"), 180915260));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr6"), 171115067));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr7"), 159321559));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr8"), 146440111));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr9"), 141696573));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr10"), 135534747));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr11"), 135046619));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr12"), 133851895));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr13"), 115169878));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr14"), 107349540));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr15"), 102531392));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr16"), 90354753));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr17"), 81529607));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr18"), 78081510));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr19"), 59380841));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr20"), 63025520));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr21"), 48157577));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chr22"), 51304566));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chrX"), 155270560));
	_chromLengthMap.insert(std::pair<int, size_t>(queryChromID("chrY"), 59373566));
}

RefGenome * RefGenome::getInstance() {
	if(_refGenome == NULL)
		_refGenome = new RefGenome;
	
	return _refGenome;
}

int RefGenome::queryChromID(std::string chrom) {
	
	if(chrom.find_first_of("chr") != std::string::npos) {
		chrom = chrom.substr(3, chrom.length() - 3);
	}
	
	if(chrom.compare("X") == 0)
		return 23;
	else if(chrom.compare("Y") == 0)
		return 24;	
	
	return atoi(chrom.c_str());
}

size_t RefGenome::queryGenomeLength() {
	size_t len = 0;
	for(size_t i=0; i<_chromIDs.size(); i++)
		len += _chromLengthMap[_chromIDs[i]];
	return len;
}

size_t RefGenome::queryChromStartBase(int chromID) {
	size_t before = 0;
	for(int i=1; i<chromID; i++)
		before += _chromLengthMap[i];
	return before;
}

size_t RefGenome::queryChromLengthWithID(int chromID)
{
	if(_chromLengthMap.count(chromID) != 1)
		return 0;
	return _chromLengthMap[chromID];
}