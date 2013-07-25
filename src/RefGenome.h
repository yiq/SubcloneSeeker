#ifndef REFGENOME_H
#define REFGENOME_H

#include <string>
#include <map>
#include <vector>


/**
 * @brief Encapsulates a reference genome
 *
 * It right now has a HG19 reference genome built in, in terms of chromosome length.
 */
class RefGenome {
public:
	/**
	 * @brief The method that returns the singleton object for the reference genome
	 *
	 * @return The singleton object of RefGenome
	 */
	static RefGenome * getInstance();

	/**
	 * returns an integer id for a given chromosome in string. The main purpose for the id representation is to unify 
	 * different notions of the same chromosome, e.g. "chr10" and "10" will both be mapped to int value 10.
	 *
	 * @param chrom The string representation of a chromosome, usually read in from a .seg.txt file
	 * @return an integer value representing the chromosome. 
	 */
	int queryChromID(std::string chrom);

	/**
	 * Returns the length of a chromsome
	 *
	 * @param chromID the integer representation of a chromosome
	 * @return the length of the chromosome
	 */
	size_t queryChromLengthWithID(int chromID);

	/**
	 * Returns the length of the entire genome
	 *
	 * @return the length of the entire genome
	 */
	size_t queryGenomeLength();

	/**
	 * Returns the starting position of a given chromosome, in the context of the entire genome
	 *
	 * @param chromID the integer representation of a chromosome
	 * @return the 0-base position of the first base of the given chromosome, in the entire genome
	 */
	size_t queryChromStartBase(int chromID);

	/**
	 * Returns all the chromosomes in the genome
	 *
	 * @return a vector of all chromosomes, in the format of string
	 */
	const std::vector<std::string>& vec_chroms() { return _chromNames; }

	/**
	 * Returns all the chromosome IDs in the genome
	 *
	 * @return a vector of all chromosomes, in the format of integer ID
	 */
	const std::vector<int> & vec_chromIDs() { return _chromIDs; }

protected:
	RefGenome();
	static RefGenome * _refGenome;			/**< The singleton object */
	std::map<int, size_t> _chromLengthMap; 	/**< The map between an chromosome id and its length */
	std::vector<std::string> _chromNames; 	/**< The vector of all chromosomes, in string format */
	std::vector<int> _chromIDs; 			/**< The vector of all chromosomes, in id format */
	
};


#endif
