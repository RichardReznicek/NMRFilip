Introduction
============

NMRFilip is the NMR data processing software. This directory contains the source code of this program part:
NMRFilip GUI - graphical user interface
Note that this program part is required: 
NMRFilip LIB - core library

PLEASE CONSIDER THAT THIS IS A DEVELOPMENT BETA VERSION. 

Since the beginning, the development of the NMRFilip software has been a private hobby activity. Therefore, the number of implemented features is determined by the amount of spare time dedicated to the development. 

Copyright and licensing information is provided mainly in the files "COPYRIGHT_INFO.txt", "COPYRIGHT" and "COPYING" and at several other places in addition.

The NMRFilip GUI program is implemented using the wxWidgets library version 3.1.2 (http://www.wxwidgets.org/). 


The description of the ulist format used to specify NMR experiments is provided together with the source code of the NMRFilip CLI and NMRFilip LIB. 


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

Please see the "build.txt" file for instructions how to compile the software. Refer to the corresponding documentation (and/or the websites) of the wxWidgets library for details concerning its installation.


NMRFilip GUI tips
=================

General
-------
* Please bear in mind that all indices used in the program start from zero (e.g. let each step contain 8 chunks, then the index of the first chunk is 0 and it is 7 for the last one).

* Mouse wheel (as well as PageUp/PageDown and up/down arrow keys) can be used to gradually increase or decrease the parameters in the fields with spin button. This is usefull especially in combination with the Auto-Apply function.

* Adjusting the shortcut to the NMRFilip GUI program to set the current directory to the parent directory of a directory tree with your data before starting the program can save a bit of your time every time you start it. See also 2) in the section "Installation in Windows".

* A basic support for screens with high pixel density is implemented. Should the user interface elements render too big or too small, ensure that your operating system is aware of correct physical dimensions and pixel density of your screen(s) and that it reports the appropriate values to the NMRFilip GUI.

* The child window which is considered by the NMRFilip GUI program as active has a bright window icon and a window title enclosed between the symbols > and <. This hint is important in the SDI build of the program as it lets you know which data you are processing when adjusting the parameter fields in the main application window (in this situation, the child window is inactive from the perspective of the window manager as the active one is the main application window).

* In unix-like systems, using the SDI build of the NMRFilip GUI on a dedicated desktop (assuming your window manager supports multiple desktops) can help you to avoid a clutter of numerous windows.

* Corresponding part of the directory subtree is scanned every time a parent folder item in the Browse panel is expanded. The whole displayed folder tree can be refreshed by pressing the designated button. 

* Directories, ulist and text files can be created and deleted using the context menu accessible by a secondary mouse button click in the Browse panel. 

* Text file edited in the simple built-in text editor can be saved using the corresponding item in the context menu accessible by a secondary mouse button click. 

Offset correction
-----------------
* Two offset correction methods intended for FID(-like) signals are implemented. The Remove offset checkbox in the Phase panel allows for a simple offset correction taking into account the assumed position of the FID start, which is used for the first order phase correction. If no first order phase correction is applied, the Remove offset functionality is equivalent to (simpler and faster) scaling the first chunk point by 0.5 (see the Analyze panel). 

Plots
-----
* A primary mouse button click in the graph area of ser or fid child window shows the Display panel.

* Particular step to display or highlight can be selected by entering its index in the corresponding field in the Display panel (and confirming by Enter) or from the plot using mouse wheel or PageUp/PageDown keys.

* The settings (axis ranges etc.) of each plot are independent from other plots of the same or any other ser of fid file. The settings can be easily duplicated in whole or in part using the Store and Load functionality of the Plot panel as desired.

* The plots support a zoom control by mouse wheel in combination with Ctrl (or Meta) (horizontal zoom) and Shift (vertical zoom) keys. Dragging using the mouse middle button provides an alternative to scrolling in the curve area of a plot.

* The zoom can be also controled by PageUp/PageDown keys in combination with Ctrl (or Meta) (horizontal zoom) and Shift (vertical zoom) keys. Arrow keys can be used for scrolling.

* The plots are by default horizontally constrained within the x-range of valid data. These constraints can be overriden by manual adjustment of the horizontal limits beyond the valid data x-range using the Plot panel. The new limits remain in effect until horizontal autoscale is used or until subsequent manual setting of horizontal axis range takes place.

* If the plotted data are by their general nature non-negative, the lower limit of the vertical range of the plot is by default constrained to zero. This constraint can be turned off by manual adjustment of the vertical minimum to negative value using the Plot panel. Vertical autoscale or subsequent manual setting of vertical minimum to non-negative value turn the constraint on again.

* The reference point [x',y'], coordinates of which are used in the Cursor box in the Display panel, can be set by a secondary mouse button click and removed by a primary mouse button click in the graph area.

* The cursor coordinates can be copied to the clipboard by primary mouse button click combined with Ctrl (or Meta) key. The coordinates can be appended to previously copied values by Ctrl (or Meta) + Shift + primary mouse button click.

Processing parameters
---------------------
* A set of parameters used for processing some particular data can be easily saved and reloaded by using the Views panel. 

* Exported text data files contain the set of necessary processing parameters in their header, thus when the raw data are opened, they can be processed again using the same parameters just by loading the corresponding exported text data file from the Views panel either directly from the list (with "> " prefix) if it resides in the "export" subdirectory of the dataset or using the Load other... button. 


Reporting bugs
==============

You are kindly asked to include all the following in any bug report:

1) Short description of the bug

2) How to reproduce the bug

3) What is expected to happen when following the instructions in 2)

4) What actually happens

If the bug appears only during processing of some specific data, it might be very helpful if you could provide that data together with the bug report.


