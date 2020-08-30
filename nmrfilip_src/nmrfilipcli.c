/* 
 * NMRFilip CLI - the NMR data processing software - command line interface
 * Copyright (C) 2010, 2011, 2020 Richard Reznicek
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#ifdef __WIN32__
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "nmrfilip.h"


void PrintUsage() {
	printf("Command-line syntax:\n\
nmrfilipcli [-<parameter>...] [--<output>[=<file>]...] [<other>...] [<datadir>...]\n\
 \n\
 Set processing <parameter>s:\n\
  Import of parameters\n\
  -view=<file> Load (overridable) processing parameters from <file>\n\
                (phase correction and other step-specific parameters are \n\
                applied only in the case of matching step count)\n\
  \n\
  Basic parameters\n\
  -B<start>    Set the first processed chunk to <start> (zero-based)\n\
  -E<end>      Set the last processed chunk to <end> (zero-based)\n\
  -b<start>    Set the processing starting point of chunk average to <start> \n\
                (zero-based)\n\
  -e<end>      Set the processing end point of chunk average to <end> \n\
                (zero-based)\n\
  -l<length>   Set length of Fourier transform to <length> points\n\
  -f<freq>     Set Fourier transform frequency filter to <freq> MHz\n\
  \n\
  Zero-order phase correction (pick one option at most)\n\
  -ph0:<phase>          Use manual zero-order phase correction of <phase> deg\n\
  -ph0auto              Use automatic zero-order phase correction - each step \n\
                         individually\n\
  -ph0autotogether      Use automatic zero-order phase correction - all steps \n\
                         together\n\
  -ph0followauto<step>  Use automatic zero-order phase correction - all steps \n\
                         together according to the selected <step> (zero-based)\n\
  \n\
  First-order phase correction\n\
  -ph1:<FIDstart>       Use manual first-order phase correction assuming FID \n\
                         start at <FIDstart> us.\n\
  \n\
  Offset correction (pick one option at most)\n\
  -scale1stpt      Use simple offset suppression by scaling the first processed\n\
                    point of chunk average before Fourier transform by 0.5\n\
  -removeoffset    Remove offset by taking into account the FID start time - \n\
                    see -ph1<FIDstart>\n\
  \n\
 Save text <output> to specified <file> or to \"export%s<output>.txt\" otherwise:\n\
  --tddata[=<file>]        Save time domain data\n\
  --echopeaks[=<file>]     Save echo peaks envelope\n\
  --chunkset[=<file>]      Save position of chunks in time domain data\n\
  --chunkavg[=<file>]      Save chunk averages\n\
  --fft[=<file>]           Save Fourier transforms of chunk averages\n\
  --spectrum[=<file>]      Save envelope of moduli of Fourier transforms\n\
  --realspectrum[=<file>]  Save envelope of real parts of Fourier transforms\n\
  --evaluation[=<file>]    Save experiment evaluation\n\
  \n\
 The <other> options:\n\
  --cl             Print copyright and license information\n\
  --help           Print this command-line parameter list\n\
  \n\
 The NMR dataset <datadir>s:\n\
  <datadir>    Specifies the NMR dataset directory to use. Default is current\n\
                working directory. Relative paths for parameter and output\n\
                files are specified with respect to the dataset directory.\n\
  \n", 
#ifdef __WIN32__
	"\\"
#else
	"/"
#endif
  );
}

void PrintLicenseInfo() {
	printf("\n\
NMRFilip CLI - the NMR data processing software - command line interface\n\
Copyright (C) 2010, 2011, 2020  Richard Reznicek\n\
\n\
This program is free software; you can redistribute it and/or\n\
modify it under the terms of the GNU General Public License\n\
as published by the Free Software Foundation; either version 2\n\
of the License, or (at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n\
\n\
--------------------------------------------------------------------------------\n\
\n\
This program is based on the NMRFilip LIB core library (Copyright (C) 2010, \n\
2011, 2020  Richard Reznicek) licensed under the terms of the GNU General \n\
Public License version 2 or any later version.\n\
\n\
--------------------------------------------------------------------------------\n\
\n\
The NMRFilip LIB core library uses the FFTW library (http://www.fftw.org) \n\
(Copyright (c) 2003, 2006 Matteo Frigo, Copyright (c) 2003, 2006 Massachusetts \n\
Institute of Technology) licensed under the terms of the GNU General Public \n\
License version 2 or any later version.\n\
\n");
	}

typedef struct {
	unsigned short GroupStart;
	unsigned long Param;
	char *Key;
	char *Desc;
	void *Var;
	unsigned short VarType;
	unsigned short VarIsStepNo;
} ParameterRelation;

typedef struct {
	unsigned long Output;
	char *Key;
	char *DefFile;
} OutputRelation;

int main(int argc, char * argv[]) {

	NMRData NMRDataStruct;
	FILE *output = NULL;
	FILE *test = NULL;
	long i = 0;
	long j = 0;
	long k = 0;
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	
	char *ViewName = NULL;
	char *OutputName = NULL;
	char *Pwd = NULL;

	long FirstChunk = 0;
	long LastChunk = 0;
	long ChunkStart = 0;
	long ChunkEnd = 0;
	long DFTLength = 0;
	double Freq = 0.0;
	
	long PhCorr0Pilot = 0;
	double PhCorr0 = 0.0;
	double PhCorr1 = 0.0;
	
	unsigned long ProcParsSet = 0;
	unsigned long OutputRequested = 0;
	
	unsigned short ShallPrintUsage = 0;
	unsigned short ShallPrintLicenseInfo = 0;
	unsigned short matched = 0;
	unsigned short UsePwd = 0;
	unsigned short failure = 0;
	unsigned short InGroup = 0;

	unsigned int DataType = 0;
	
	long AuxVal = 0;
	
	ParameterRelation ParRel[14] = {
		{0, 0, "-view=", "file with processing parameters", &ViewName, PARAM_STRING, 0},
		{1, PROC_PARAM_FirstChunk, "-B", "first processed chunk", &FirstChunk, PARAM_LONG, 0},
		{1, PROC_PARAM_LastChunk, "-E", "last processed chunk", &LastChunk, PARAM_LONG, 0},
		{1, PROC_PARAM_ChunkStart, "-b", "chunk average processing start point", &ChunkStart, PARAM_LONG, 0},
		{1, PROC_PARAM_ChunkEnd, "-e", "chunk average processing end point", &ChunkEnd, PARAM_LONG, 0},
		{1, PROC_PARAM_DFTLength, "-l", "Fourier transform length", &DFTLength, PARAM_LONG, 0},
		{1, PROC_PARAM_Filter, "-f", "filter halfwidth", &Freq, PARAM_DOUBLE, 0},
		{1, PROC_PARAM_RemoveOffset, "-removeoffset", "", NULL, PARAM_NONE, 0},
		{0, PROC_PARAM_ScaleFirstTDPoint, "-scale1stpt", "", NULL, PARAM_NONE, 0},
		{1, PROC_PARAM_PhaseCorr0FollowAuto, "-ph0followauto", "pilot step number", &PhCorr0Pilot, PARAM_LONG, 1},
		{0, PROC_PARAM_PhaseCorr0AutoAllTogether, "-ph0autotogether", "", NULL, PARAM_NONE, 0},
		{0, PROC_PARAM_PhaseCorr0Auto, "-ph0auto", "", NULL, PARAM_NONE, 0},
		{0, PROC_PARAM_PhaseCorr0Manual, "-ph0:", "phase shift value", &PhCorr0, PARAM_DOUBLE, 0},
		{1, PROC_PARAM_PhaseCorr1ManualRefDataStart, "-ph1:", "FID start value", &PhCorr1, PARAM_DOUBLE, 0}
	};
	
#ifdef __WIN32__
	OutputRelation OutRel[8] = {
		{EXPORT_EchoPeaksEnvelope, "--echopeaks", "export\\echopeaks.txt"},
		{EXPORT_TDD, "--tddata", "export\\tddata.txt"}, 
		{EXPORT_ChunkSet, "--chunkset", "export\\chunkset.txt"}, 
		{EXPORT_ChunkAvg, "--chunkavg", "export\\chunkavg.txt"}, 
		{EXPORT_DFTPhaseCorrResult, "--fft", "export\\fft.txt"},
		{EXPORT_DFTEnvelope, "--spectrum", "export\\spectrum.txt"},
		{EXPORT_DFTPhaseCorrRealEnvelope, "--realspectrum", "export\\realspectrum.txt"},
		{EXPORT_Evaluation, "--evaluation", "export\\evaluation.txt"}
	};
#else
	OutputRelation OutRel[8] = {
		{EXPORT_EchoPeaksEnvelope, "--echopeaks", "export/echopeaks.txt"},
		{EXPORT_TDD, "--tddata", "export/tddata.txt"}, 
		{EXPORT_ChunkSet, "--chunkset", "export/chunkset.txt"}, 
		{EXPORT_ChunkAvg, "--chunkavg", "export/chunkavg.txt"}, 
		{EXPORT_DFTPhaseCorrResult, "--fft", "export/fft.txt"},
		{EXPORT_DFTEnvelope, "--spectrum", "export/spectrum.txt"},
		{EXPORT_DFTPhaseCorrRealEnvelope, "--realspectrum", "export/realspectrum.txt"},
		{EXPORT_Evaluation, "--evaluation", "export/evaluation.txt"}
	};
#endif
	
	/** No arguments provided -> display the usage of this program and exit **/
	if (argc < 2) {
		PrintUsage();
		return 0;
	}

	
	/** Get the processing parameters first... **/
	for (i = 1, matched = 0; i < argc; i++, matched = 0) {
		
		if ((!matched) && (strncmp(argv[i], "--help", 6) == 0)) {
			matched = 1;
			ShallPrintUsage = 1;
		} 
		
		if ((!matched) && (strncmp(argv[i], "--cl", 4) == 0)) {
			matched = 1;
			ShallPrintLicenseInfo = 1;
		} 
		
		for (j = 0; (!matched) && (j < 14); j++) {
			
			if (strncmp(argv[i], ParRel[j].Key, strlen(ParRel[j].Key)) == 0) {
				matched = 1;
				ptr1 = argv[i] + strlen(ParRel[j].Key);
				errno = 0;
				
				switch (ParRel[j].VarType) {
					case PARAM_LONG:
						*((long *) ParRel[j].Var) = strtol(ptr1, &ptr2, 0);
						if (errno || (ptr1 == ptr2) || (*((long *) ParRel[j].Var) < 0)) {
							fprintf(stderr, "Invalid %s supplied.\n", ParRel[j].Desc);
							free(ViewName);
							return -1;
						}
						break;
					
				/*	case PARAM_ULONG:
						*((unsigned long *) ParRel[j].Var) = strtoul(ptr1, &ptr2, 0);
						if (errno || (ptr1 == ptr2) || (*((unsigned long *) ParRel[j].Var) > (unsigned long) LONG_MAX)) {
							fprintf(stderr, "Invalid %s supplied.\n", ParRel[j].Desc);
							free(ViewName);
							return -1;
						}
						break;
				*/	
					case PARAM_DOUBLE:
						*((double *) ParRel[j].Var) = strtod(ptr1, &ptr2);
						if (errno || (ptr1 == ptr2) || !isfinite(*((double *) ParRel[j].Var))) {
							fprintf(stderr, "Invalid %s supplied.\n", ParRel[j].Desc);
							free(ViewName);
							return -1;
						}
						break;
					
					case PARAM_STRING:
						*((char **) ParRel[j].Var) = strdup(ptr1);
						break;
					
					default:
						;
				}
				
				ProcParsSet |= Flag(ParRel[j].Param);
			}
		}

		for (j = 0; (!matched) && (j < 8); j++) {
			if (strncmp(argv[i], OutRel[j].Key, strlen(OutRel[j].Key)) == 0) {
				matched = 1;
				OutputRequested |= Flag(OutRel[j].Output);
			}
		}

	}
	
	
	if (ShallPrintUsage)
		PrintUsage();
	
	if (ShallPrintLicenseInfo)
		PrintLicenseInfo();
	
	/** No need to process anything **/
	if (!OutputRequested) {
		free(ViewName);
		return 0;
	}
		
	for (k = argc - 1, UsePwd = 1; UsePwd && (k > 0); k--) 
		if (argv[k][0] != '-')
			UsePwd = 0;
		
	if (!UsePwd) {
#ifdef __WIN32__
		Pwd = _getcwd(NULL, 0);
#else
		Pwd = getcwd(NULL, 0);
#endif
		if (!Pwd) {
			fprintf(stderr, "Cannot get the current working directory.\n");
			free(ViewName);
			return -1;
		}
	}
	
	k = argc;
	while (1) {
		if (!UsePwd) {
			k--;

			if (k <= 0)
				break;
			
			if (argv[k][0] == '-')
				continue;
			
#ifdef __WIN32__
			if (_chdir(argv[k])) {
#else
			if (chdir(argv[k])) {
#endif
				fprintf(stderr, "Cannot switch to the specified dataset directory \"%s\".\n", argv[k]);
				continue;
			}
		}
		
		if (InitNMRData(&NMRDataStruct) != DATA_OK) {
			fprintf(stderr, "Cannot initialize NMRData structure.\n");
			free(ViewName);
			free(Pwd);
			return -1;
		}

		test = fopen("ser", "r");
		if (test) {
			NMRDataStruct.SerName = "ser";
			fclose(test);
		} else {
			test = fopen("fid", "r");
			if (test) {
				NMRDataStruct.SerName = "fid";
				fclose(test);
			} else {
				fprintf(stderr, "Cannot access ser nor fid file.\n");
				free(ViewName);
				free(Pwd);
				return -1;
			}
		}
		
		
		/** ...then set the processing parameters or load reasonable defaults... **/
		if (ViewName) {
			if (ImportProcParams(&NMRDataStruct, ViewName) != DATA_OK) {
				fprintf(stderr, "Cannot load processing parameters from the file \"%s\".\n", ViewName);
				free(ViewName);
				free(Pwd);
				return -1;
			}
		}
		
		
		for (j = 1; j < 14; j++) {
			if (ParRel[j].GroupStart)
				InGroup = 1;
			
			if ((ProcParsSet & Flag(ParRel[j].Param)) && InGroup) {
				InGroup = 0;
				AuxVal = 1;
				if (
					SetProcParam(
						&NMRDataStruct, 
						ParRel[j].Param, 
						(ParRel[j].VarType == PARAM_NONE)?(PARAM_LONG):(ParRel[j].VarType), 
						((ParRel[j].VarType == PARAM_NONE) || ParRel[j].VarIsStepNo)?(&AuxVal):(ParRel[j].Var), 
						(ParRel[j].VarIsStepNo)?(ParRel[j].Var):(NULL)
					) != DATA_OK
				) 
					fprintf(stderr, "Setting %s failed.\n", ParRel[j].Desc);
			}
		}
		
		/** ...and finally process the data. **/
		for (i = 1; i < argc; i++) {
			output = NULL;
			DataType = 0;
			
			for (matched = 0, failure = 0, j = 0; (!matched) && (j < 8); j++) {
				if ((OutputRequested & Flag(OutRel[j].Output)) && (strncmp(argv[i], OutRel[j].Key, strlen(OutRel[j].Key)) == 0)) {
					matched = 1;
					DataType = OutRel[j].Output;
					if (argv[i][strlen(OutRel[j].Key)] == '=') 	/** get the output file name **/
						OutputName = strdup(&(argv[i][strlen(OutRel[j].Key) + 1]));
					else 	/** use default output file name **/
						OutputName = strdup(OutRel[j].DefFile);
					
					/** create the directories in the output path, if necessary **/
					
					ptr1 = OutputName;
					
#ifdef __WIN32__
					/** handle UNC paths and drive letters **/
					if (strncmp(ptr1, "\\\\?\\UNC\\", strlen("\\\\?\\UNC\\")) == 0) {
						ptr1 += strlen("\\\\?\\UNC\\");
						ptr1 = strchr(ptr1, '\\');
						if (!ptr1)
							failure =1;
					}
					else
					if (strncmp(ptr1, "\\\\?\\", strlen("\\\\?\\")) == 0) {
						ptr1 += strlen("\\\\?\\");
						ptr1 = strchr(ptr1, '\\');
						if (!ptr1)
							failure =1;
					}
					else 
					if (strncmp(ptr1, "\\\\.\\", strlen("\\\\.\\")) == 0) {
						ptr1 += strlen("\\\\.\\");
						ptr1 = strchr(ptr1, '\\');
						if (!ptr1)
							failure =1;
					}
					else 
					if (strlen(ptr1) >= 2) {
						if (ptr1[1] == ':') 
							ptr1 += 2;
					}
					
					ptr2 = ptr1;
					while ((!failure) && (ptr2 = strpbrk(ptr2, "\\/"))) {
						if (ptr2 > ptr1) {	/** do nothing for root dir **/
							*ptr2 = '\0';
							if (_mkdir(OutputName) && (errno != EEXIST)) {
								failure = 1;
								fprintf(stderr, "Unable to create directory \"%s\".\n", OutputName); 
							}
							*ptr2 = '\\';
						}
						ptr2++;
					}
#else
					ptr2 = ptr1;
					while ((!failure) && (ptr2 = strchr(ptr2, '/'))) {
						if (ptr2 > ptr1) {	/** do nothing for root dir **/
							*ptr2 = '\0';
							if (mkdir(OutputName, 0755) && (errno != EEXIST)) {
								failure = 1;
								fprintf(stderr, "Unable to create directory \"%s\".\n", OutputName); 
							}
							*ptr2 = '/';
						}
						ptr2++;
					}
#endif
					
					if (!failure) {
						output = fopen(OutputName, "w");
						if (!output)
							fprintf(stderr, "Cannot open the output file \"%s\".\n", OutputName);
					}
				}
			}
			
			if (output) {
				if ((!failure) && (DataToText(&NMRDataStruct, output, NULL, NULL, EXPORT_AcquInfo) != DATA_OK)) 
					failure = 1;

				if ((!failure) && (DataToText(&NMRDataStruct, output, NULL, NULL, EXPORT_ProcParams) != DATA_OK)) 
					failure = 1;

				if ((!failure) && (DataToText(&NMRDataStruct, output, NULL, NULL, DataType) != DATA_OK)) 
					failure = 1;
				
				fclose(output);
				
				if (failure)
					fprintf(stderr, "Failure when exporting data to the output file \"%s\".\n", OutputName);
			} 
			
			free(OutputName);
			OutputName = NULL;
		}
		
		if (FreeNMRData(&NMRDataStruct) != DATA_EMPTY) {
			fprintf(stderr, "Cannot free NMRData structure.\n");
			free(ViewName);
			free(Pwd);
			CleanupOnExit();
			return -1;
		}

		if (UsePwd) {
			break;
		} else {
#ifdef __WIN32__
			if (_chdir(Pwd)) {
#else
			if (chdir(Pwd)) {
#endif
				fprintf(stderr, "Cannot switch back to the original current working directory.\n");
				free(ViewName);
				free(Pwd);
				CleanupOnExit();
				return -1;
			}
		}
	
	}
	
	free(ViewName);
	free(Pwd);

	CleanupOnExit();

	return 0;
}
