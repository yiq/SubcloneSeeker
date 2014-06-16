Command line utilities {#utils}
===============================

This section will explain the purpose and usage of the many utilities included in the project. Notice that data varies from study to study, and often custom pre-processing and data transformation is needed (e.g. I've seen at least 4 different ways people store copy number data, with slightly different column configurations). Thus when the project was started, I put much more emphersize on the core library, hoping that it could serve as a common ground for different types of genomic variation data. Custom "client" application will be written to bridge the generic library, and each specific data type. That being said, several utilities are already included in the project that are able to tackle most of the situations.

In order to best utilize the tools, a rudimentary understanding of the library is helpful. 

SubcloneSeeker Library Overview
-------------------------------

### Basic classes for genomic variations and subclone structures

The class SomaticEvent abstracts the knowledge of all somatic events irregardless of their specific type (SNV, LOH, CNV, etc.). It contains one property, "frequency", that corresponds to the cell prevalence, and a method "isEqualTo" to check if two different SomaticEvent objects are actually representing the same event (e.g. two separated created CNV object that are all representing chr1:1,000,000-2,000,000). Several concrete classes are created, e.g. the class SNP for single nucleotide polymorphism (variation), and SegmentalMutation for CNV and LOH events.

A layer higher is the class EventCluster, which represents a group of SomaticEvent objects with the same or similar cell prevalence. EventCluster objects also has an intrinsic ordering, allowing them to be sorted by the cell pravelence of their member events. 

The class Subclone represent a signle subclone in a population mixture. It contains a set of EventCluster objects, representing its genotype, as well as a fraction field describes the fraction this subclone is taking up. It inherits from a generic class TreeNode, which allows a node to contain any number of child nodes. A Subclone object that has no parent node, in other words, a root node, along with all its descendend nodes (directly or indirectly) represent a tree structure (or, a solution). 

### Sqlite based object archiving

All the classes mentioned implements the protocol "Archivable", which provides the ability to serialize and unserialize objects of those classes into a sqlite v3 database. All the algorithms are designed to work directly with the objects, which means that the I/O of creating these objects from flat genomic variation file (e.g. vcf) belongs to the data type / format specific domain. Thus the commandline utilities are largely divided into two categories
  * Those that implements specific algorithms, and works with database files.
  * Those that create objects, and save them into a sqlite database, from textual input.

Command-line Utilities
----------------------

All the commandline utilities are found in the utils subdirectory

### Utilities that run algorithms
#### ssmain

`Usage: ./ssmain <cluster-archive-sqlite-db> [output-db]`

This is the main entrance to the SubcloneSeeker structure enumeration algorithm. It takes one required parameter, cluster-archive-sqlite-db, which is the filename to a sqlite database that already contains serialized EventCluster objects. If the second parameter, output-db is also provided, the resulting structures will be written to the named database, creating one if not already existing, by serializing the Subclone objects into it. For multiple solutions, each solution will have a unique subclone object that has no parent node. 

#### treemerge

`Usage: ./treemerge <tree-set 1 database file> <tree-set 2 database file>`

this is the implementation of the algorithm to trim solution space by merging e.g. primary and relapse solutions. It expects two filenames are arguments, each corresponds to a sample. The compatible structures will be directly reported to standard output. E.g.

<code>
Primary tree 1 is compatible with Secondary tree 1

Primary tree 1 is compatible with Secondary tree 5
</code>

This means that, for this specific sample pair, the primary subclone structure, whose root node has an ID number of 1 in the database, is compatible with both of the relapse structures, with a root node ID of 1 and 5, in the relapse database.

#### coexist_matrix

`Usage: ./colocal_matrix <subclone-sqlite-db>`

Calculate the co-localization matrix. The parameter subclone-sqlite-db is the filename of a databaes with potentially multiple solution structures. The utility calculates the number of subclones in which each EventCluster pair co-localize, and dump the result to standard output. The first line is the number of unique pairs, followed by lines that the first two columns are the EventCluster object IDs of a pair, and the third column is the number of subclones in which they co-localize.

### Utilities that handles flat file to database conversion
#### segtxt2db

Convert a common file format that describes segmental events into database objects. The file is in tab delimited format, with the first line being the column header. The header usually includes
  * ID
  * Chrom
  * StartLoc
  * EndLoc
  * numMark
  * segMean

The ID has nothing to do with the database ID. the numMark usually represents the number of probes located in a segment. The segMean needs to be in the format of log 2 of the segment's cell prevalence value. Some of the other parameters are

<code>
Usage: ./segtxt2db \<seg.txt file\> \<result database\>

    Options: 

     -p purity      [default = 1]           A number between 0-1 specifying the purity of the sample

     -q ploidy      [default = 2]           A integer number specifying the ploidy of the copy number neutral regions

     -n ratio       [default = 1]           A tumor/normal ratio where the copy number neutral regions are found

     -m                                     Fraction correction by modal value

     -r mask-file                           A mask file for regions to exclude

     -t threshold   [default = 0.05]        The ratio threshold for merging two segments into a cluster

     -e length      [default=0]             The minimal cumulative length of a cluster to be included in the result
</code>

Most of the parameters are self explainatory. if `-m` is specified, segMean will be normalized by the modal segMean value. `-r` can be used to specify a file, with three columns Chrom, StartLoc and endLoc without header line, that describes regions to be excluded from analysis (e.g. centromere). The result database will have both the segments serialized as SegmentalMutation objects, and clusters as EventCluster objects, which will be suitable for `ssmain` to perform subclone deconvolution

An example can be seen in the `run.sh` script in 02-sunc folder inside the example package

#### cluster2db

`Usage: ./cluster2db <Cluster-List-File>`

cluster2db is a very specialized tool, which was initially developed for the re-analysis of the WashU AML dataset (Ding et al.). The argument is the filename to a flat text file that describes the cell prevalence of each event cluster in the Primary - Relapse plane. Each line corresponds to a cluster, and the first column represent the CP in the primary sample, and second column the CP in the relapse sample. The utility will then create two files, ORIGINAL-FILENAME-pri.sqlite and ORIGINAL-FILENAME-rel.sqlite, that contains the EventCluster objects of each sample. Since only the cluster CP values are given, instead of the actual events, dummy events are created as copy number variations whose chromosome id field is reused as a generic serial id number, and `start` and `length` values uninitialized. The database files are suitable to be used as the input to `ssmain`.

An example can be seen in the `run.sh` script in 03-washu folder inside the example package.

#### treeprint

`Usage: ./treeprint <sqlite-db> <root-id> [-g]`

A simple utility to print the subclone structure in the database sqlite-db, with a root node id number root-id, to the standard output. Maybe more useful is the switch `-g`, which will print the structure in GraphViz format to be directly visualized.
