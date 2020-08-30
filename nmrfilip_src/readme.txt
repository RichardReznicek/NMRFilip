Introduction
============

NMRFilip is the NMR data processing software. This directory contains the source code of these program parts:
NMRFilip CLI - command line interface
NMRFilip LIB - core library

PLEASE CONSIDER THAT THIS IS A DEVELOPMENT BETA VERSION. 

Since the beginning, the development of the NMRFilip software has been a private hobby activity. Therefore, the number of implemented features is determined by the amount of spare time dedicated to the development. 

Copyright and licensing information is provided mainly in the files "COPYRIGHT_INFO.txt", "COPYRIGHT" and "COPYING" and at several other places in addition.

The NMRFilip LIB core library uses the FFTW library version 3 (http://www.fftw.org). 


The description of the ulist format used to specify NMR experiments is provided in the subdirectory "ulist_format". 


Features
========

NMRFilip is primarily intended for processing of data from broadband solid-state NMR experiments. This may include for instance composition of final spectrum from a number of measurement steps at different excitation frequencies, routine evaluation of signal intensity in dependence on pulse sequence parameters, or improving signal-to-noise ratio by averaging acquired echo train from CPMG sequence. 

The processing consists of several basic parts (terminology introduced in quotes): 

1) The dataset is opened, one (usually in a fid file) or more (in a pseudo-2D ser file) time domain records ("steps") are identified, and associated values based on a v?list/fq?list or a ulist.out/userlist file are assigned to corresponding steps. (Currently, only the acquisition data format used by Bruker NMR spectrometers is supported.)

2) The group delay caused by digital DSP filter is determined, and the time position of data points with respect to the actual start of recording is adjusted accordingly (unless the experimental support of this feature is disabled during a compilation - see below). The points corresponding to negative time are ignored in the subsequent processing, whereas the time shift of the remaining points is taken into account if a first-order phase correction is later applied.

3) A pattern consisting of one (such as FID) or more (e.g. all echoes from a CPMG sequence) signal segments ("chunks") of non-zero points in the time domain records is detected.

4) For each step, an average of selected chunks is calculated and a selected part of the "chunk average" is Fourier transformed.

5) The outputs of the Fourier transform ("FFT") can be limited to a desired bandwidth ("filter") and phase corrected. 

6) Subsequently, an envelope of these FFT outputs from all (non-excluded) steps is constructed, which presents the final "spectrum".

7) The "evaluation" of signal intensity in particular steps can be carried out using various criteria (such as a maximum or an integral in time or frequency domain).


Building the software from the sources
======================================

Please see the "build.txt" file for instructions how to compile the software. Refer to the corresponding documentation (and/or the websites) of the FFTW library for details concerning its installation.


NMRFilip CLI parameters
=======================

Command-line syntax:
nmrfilipcli [-<parameter>...] [--<output>[=<file>]...] [<other>...] [<datadir>...]

 Set processing <parameter>s:
  Import of parameters
  -view=<file> Load (overridable) processing parameters from <file>
                (phase correction and other step-specific parameters are
                applied only in the case of matching step count)

  Basic parameters
  -B<start>    Set the first processed chunk to <start> (zero-based)
  -E<end>      Set the last processed chunk to <end> (zero-based)
  -b<start>    Set the processing starting point of chunk average to <start>
                (zero-based)
  -e<end>      Set the processing end point of chunk average to <end>
                (zero-based)
  -l<length>   Set length of Fourier transform to <length> points
  -f<freq>     Set Fourier transform frequency filter to <freq> MHz

  Zero-order phase correction (pick one option at most)
  -ph0:<phase>          Use manual zero-order phase correction of <phase> deg
  -ph0auto              Use automatic zero-order phase correction - each step
                         individually
  -ph0autotogether      Use automatic zero-order phase correction - all steps
                         together
  -ph0followauto<step>  Use automatic zero-order phase correction - all steps
                         together according to the selected <step> (zero-based)

  First-order phase correction
  -ph1:<FIDstart>       Use manual first-order phase correction assuming FID
                         start at <FIDstart> us.

  Offset correction (pick one option at most)
  -scale1stpt      Use simple offset suppression by scaling the first processed
                    point of chunk average before Fourier transform by 0.5
  -removeoffset    Remove offset by taking into account the FID start time -
                    see -ph1<FIDstart>

 Save text <output> to specified <file> or to "export\<output>.txt" otherwise:
  --tddata[=<file>]        Save time domain data
  --echopeaks[=<file>]     Save echo peaks envelope
  --chunkset[=<file>]      Save position of chunks in time domain data
  --chunkavg[=<file>]      Save chunk averages
  --fft[=<file>]           Save Fourier transforms of chunk averages
  --spectrum[=<file>]      Save envelope of moduli of Fourier transforms
  --realspectrum[=<file>]  Save envelope of real parts of Fourier transforms
  --evaluation[=<file>]    Save experiment evaluation

 The <other> options:
  --cl             Print copyright and license information
  --help           Print this command-line parameter list

 The NMR dataset <datadir>s:
  <datadir>    Specifies the NMR dataset directory to use. Default is current
                working directory. Relative paths for parameter and output
                files are specified with respect to the dataset directory.


Reporting bugs
==============

You are kindly asked to include all the following in any bug report:

1) Short description of the bug

2) How to reproduce the bug

3) What is expected to happen when following the instructions in 2)

4) What actually happens

If the bug appears only during processing of some specific data, it might be very helpful if you could provide that data together with the bug report.


