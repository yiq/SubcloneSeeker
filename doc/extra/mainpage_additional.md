Running the examples
====================

There are four examples prepared for you in a easy-to-run fashion. Each of the
example represents a figure in the article. In order reproduce the figures,
make sure you have R installed, and the binary can be found through PATH
lookup. If you run the following command

	which R

And see an absolute path, such as '/usr/bin/R', then the requirement is
satisfied

Getting the example tarball
---------------------------

The example tarball consists of four results presented in the paper. The
tarball is at the root directory of the source tree.

https://github.com/yiq/SubcloneSeeker/blob/master/examples.tar.gz



Setup the variables
-------------------

Navigate to the example directory on a commandline. Edit the file 'setup.sh' so
that it reflects how you organized the project files. Make sure that you have
compiled the binaries in the SubcloneSeeker project. If you haven't done so,
please refer to the 'INSTALLATION' section in this document.

###Example-1: Simulation

__Directory: 01-simulation__

The first example simulates 1000 tumors, each with 3, 4, ..., 8 subclones.
Subclone Deconvolution is then performed on all the samples, and statistics
will be collected as in how many possible solutions for each tumor. You can
initiate the simulation by running the 'sim.sh' script

	./sim.sh

Note that this will take some time to finish, especially in the case 7 or 8
subclones per tumor. Results will be stored in 'result' directory. To plot the
results, launch R in the example directory and run the 'plot.r' scriot

	R
	source('plot.r');

A result set from a run executed on my computer is also provided, if you just
want to plot the results. In that case, please use the 'plot\_prerun.r' script

###Example-2: Purity estimation with the snuc cancer cell-line dataset

__Directory: 02-snuc__

In this example, a snuc cancer cell line (thus very high purity) was
sequenced, and its sequencing reads were combined with those reads from
sequencing the paired-normal sample at various ratios. Subclone deconvolution
is performed on each of the digitally created samples, and purity estimation is
reported. In order to run the example, simple type

	make

in the example directory, as it will take care of removing old results (if you
run it before), creating required directories, running the deconvolution and
plotting the result. After the execution, a file 'SamplePurity.pdf' should be
created in the same directory, which contains the plotted result. 

###Example-3: Re-analysis of the dataset presented in Ding et, al. 2012

execute

	make

(More content to be written......)

###Example-4: Analysis of two OvCa cancer sample Single Nucleotide Polymorphism dataset

execute

	make

(More content to be written......)
