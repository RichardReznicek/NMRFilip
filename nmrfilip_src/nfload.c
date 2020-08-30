/* 
 * NMRFilip LIB - the NMR data processing software - core library
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
#include <limits.h>
#include <inttypes.h>
#include "fftw3.h"

#include "nmrfilip.h"

#include "nfio.h"
#include "nfload.h"
#include "nfproc.h"


typedef struct {
	unsigned long vlistType;
	char *vlistName;
	char *vlistAcqus;
} vlistAttr;

int LoadVlist(NMRData *NMRDataStruct, const vlistAttr *vlistAttrib, char *path) {
	int RetVal = DATA_OK;
	int matched = 0;
	size_t count = 0, index = 0, len = 0;
	char *ptr1 = NULL;
	char *ptr2 = NULL;

	char *vlistName = NULL;
	char *vlistData = NULL;
	size_t vlistLength = 0;

	char *AssocValueVariable = NULL;
	char *AssocValueUnits = NULL;
	double *AssocValues = NULL;
	size_t AssocValuesLength = 0;

	FILE *test = NULL;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((vlistAttrib == NULL) || (path == NULL)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid parameter supplied", "Loading v?list/fq?list file");
		return INVALID_PARAMETER;
	}
	
	vlistName = CombineStr(path, vlistAttrib->vlistName);

	/** check silently if the file exists **/
	if ((vlistName != NULL) && (test = fopen(vlistName, "r"))) 
		fclose(test);
	else {
		free(vlistName);
		return (DATA_OLD | FILE_OPEN_ERROR);
	}
	
	if (((RetVal = LoadTextFile(NMRDataStruct, &vlistData, &vlistLength, vlistName)) == FILE_LOADED_OK) && (vlistData != NULL)) {
		ptr1 = vlistData;

		if (strncmp(ptr1, "\xEF\xBB\xBF", 3) == 0)
			ptr1 += 3;	/** skip UTF-8 BOM if present **/

		ptr1 += strspn(ptr1, " \t\r\n");
		len = strspn(ptr1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
		if (len > 0) {
			if (vlistAttrib->vlistType & ASSOC_FQLIST_MASK)
				len = strcspn(ptr1, "\r\n");	/** read even more complicated header line **/
			/** check if its ASCII **/
			if (IsAsciiSubstr(ptr1, len)) 
				AssocValueUnits = DupSubstr(ptr1, len);
			matched = 1;
		}
		
		switch (vlistAttrib->vlistType) {	/** set default units **/
			case ASSOC_VALIST:
				if (matched != 1) 
					AssocValueUnits = strdup("dB");
				AssocValueVariable = strdup("power");
				break;
				
			case ASSOC_VCLIST:
				AssocValueVariable = strdup("count");
				break;
				
			case ASSOC_VDLIST:
				if (matched != 1) 
					AssocValueUnits = strdup("s");
				AssocValueVariable = strdup("delay");
				break;
				
			case ASSOC_VPLIST:
				if (matched != 1) 
					AssocValueUnits = strdup("us");
				AssocValueVariable = strdup("pulse length");
				break;
				
			case ASSOC_VTLIST:
				if (matched != 1) 
					AssocValueUnits = strdup("K");
				AssocValueVariable = strdup("temperature");
				break;

			case ASSOC_FQ1LIST:
			case ASSOC_FQ2LIST:
			case ASSOC_FQ3LIST:
			case ASSOC_FQ4LIST:
			case ASSOC_FQ5LIST:
			case ASSOC_FQ6LIST:
			case ASSOC_FQ7LIST:
			case ASSOC_FQ8LIST:
				if (matched != 1) 
					AssocValueUnits = strdup("Hz");
				AssocValueVariable = strdup("frequency offset");
				break;
				
			default:
				;
		}
		
		/** count the number of vlist items **/
		count = 0;
		ptr1 = vlistData;
		if (matched) {	/** skip the header **/
			ptr1 += strspn(ptr1, " \t\r\n");
			ptr1 = strpbrk(ptr1, "\r\n");
		}
		if (ptr1 != NULL)
			while ((ptr1 = strpbrk(ptr1, "0123456789.+-"))) {
				count++;
				ptr1 += strspn(ptr1, "0123456789.+-");
			}
		
		/** get the memory space **/
		if (count > 0) {	
			AssocValues = (double *) malloc(count*sizeof(double));
			if (AssocValues == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating associated values array memory space");

				/** clean up and return **/
				free(vlistName);
				FreeText(NMRDataStruct, &vlistData, &vlistLength);

				free(AssocValueVariable);
				free(AssocValueUnits);
				
				return (DATA_OLD | MEM_ALLOC_ERROR);
			}
		}

		/** read the vlist items **/
		index = 0;
		ptr1 = vlistData;
		if (matched) {	/** skip the header **/
			ptr1 += strspn(ptr1, " \t\r\n");
			ptr1 = strpbrk(ptr1, "\r\n");
		}
		if (ptr1 != NULL)
			while ((ptr1 = strpbrk(ptr1, "0123456789.+-")) && (index < count)) {

				errno = 0;
				AssocValues[index] = strtod(ptr1, &ptr2);
				if (errno || (ptr1 == ptr2) || !isfinite(AssocValues[index])) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading v?list/fq?list parameter value");
					
					/** clean up and return **/
					free(vlistName);
					FreeText(NMRDataStruct, &vlistData, &vlistLength);

					free(AssocValues);
					free(AssocValueVariable);
					free(AssocValueUnits);
					
					return DATA_OLD;
				}
				
				if (vlistAttrib->vlistType == ASSOC_VDLIST) {
					switch (*ptr2) {
						case 's':
							break;

						case 'm':
							AssocValues[index] *= 1.0e-3;
							break;

						case 'u':
							AssocValues[index] *= 1.0e-6;
							break;
						default:
							;
					}
				}
				
				if (vlistAttrib->vlistType == ASSOC_VPLIST) {
					switch (*ptr2) {
						case 's':
							AssocValues[index] *= 1.0e6;
							break;

						case 'm':
							AssocValues[index] *= 1.0e3;
							break;

						case 'u':
							break;
						default:
							;
					}
				}
				
				ptr1 = ptr2;
				index++;
			}
			
		AssocValuesLength = index;

		/** copy the data to the NMRDataStruct **/
		NMRDataStruct->AcquInfo.AssocValueType = vlistAttrib->vlistType;
		free(NMRDataStruct->AcquInfo.AssocValueTypeName);	/** just in case **/
		NMRDataStruct->AcquInfo.AssocValueTypeName = strdup(vlistAttrib->vlistName);
		free(NMRDataStruct->AcquInfo.AssocValueVariable);	/** just in case **/
		NMRDataStruct->AcquInfo.AssocValueVariable = AssocValueVariable;
		free(NMRDataStruct->AcquInfo.AssocValueUnits);	/** just in case **/
		NMRDataStruct->AcquInfo.AssocValueUnits = AssocValueUnits;
		free(NMRDataStruct->AcquInfo.AssocValues);	/** just in case **/
		NMRDataStruct->AcquInfo.AssocValues = AssocValues;
		NMRDataStruct->AcquInfo.AssocValuesLength = AssocValuesLength;
		
		/** clean up **/
		free(vlistName);
		FreeText(NMRDataStruct, &vlistData, &vlistLength);
		
		return DATA_OK;
		
	} else {
		/** clean up and return **/
		free(vlistName);
		FreeText(NMRDataStruct, &vlistData, &vlistLength);
		
		return (RetVal | DATA_OLD);
	}
}

int GetAcquParams(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	char *vlist = NULL;
	char *ptr1 = NULL, *ptr2 = NULL;
	char *path = NULL;
	char *filename = NULL;
	size_t i = 0;
	int RetVal = DATA_OK;
	long td = 0;
	long bo = 0;
	long dtypa = 0;
	double GroupDelay = 0.0;
	unsigned long ParamFlag = 0;
	FILE *test = NULL;
	UserlistParams UParams;

	const vlistAttr vlistAttribs[13] = {
		{ASSOC_VALIST, "valist", "$VALIST"}, 
		{ASSOC_VCLIST, "vclist", "$VCLIST"}, 
		{ASSOC_VDLIST, "vdlist", "$VDLIST"}, 
		{ASSOC_VPLIST, "vplist", "$VPLIST"}, 
		{ASSOC_VTLIST, "vtlist", "$VTLIST"}, 

		{ASSOC_FQ1LIST, "fq1list", "$FQ1LIST"}, 
		{ASSOC_FQ2LIST, "fq2list", "$FQ2LIST"}, 
		{ASSOC_FQ3LIST, "fq3list", "$FQ3LIST"}, 
		{ASSOC_FQ4LIST, "fq4list", "$FQ4LIST"}, 
		{ASSOC_FQ5LIST, "fq5list", "$FQ5LIST"}, 
		{ASSOC_FQ6LIST, "fq6list", "$FQ6LIST"}, 
		{ASSOC_FQ7LIST, "fq7list", "$FQ7LIST"}, 
		{ASSOC_FQ8LIST, "fq8list", "$FQ8LIST"}
	};

	const char *VarsUnits [ASSOC_UserlistHighest + 1][3] = {
		{NULL, NULL, NULL}, 	/** ASSOC_NOT_SET **/
		{NULL, NULL, "Variable"}, 	/** ASSOC_UNKNOWN, ASSOC_VARIABLE **/
		{"power", "dB", "RF power"}, 	/** ASSOC_PWR_DB **/
		{"trigger delay", "s", "Trigger delay"}, 	/** ASSOC_TRIG_S **/
		{"evolution time", "s", "Inversion recovery"}, 	/** ASSOC_INVREC_S **/
		{"echo delay", "s", "T2 relaxation"}, 	/** ASSOC_T2_S **/
		{"pulse length", "us", "Nutation"}, 	/** ASSOC_NUTATION_US **/
		{"frequency", "MHz", "Spectrum"} 	/** ASSOC_FREQ_MHZ **/
	};

	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	InitUserlist(NMRDataStruct, &UParams);
	
	FreeAcquInfo(NMRDataStruct);
	InitAcquInfo(NMRDataStruct);

	/** extracting the path (if any) from ser/fid filename **/
	ptr1 = NULL;
	if (NMRDataStruct->SerName != NULL) {
#ifdef __WIN32__
		ptr1 = strrchr(NMRDataStruct->SerName, '\\');
		if (!ptr1)
			ptr1 = strrchr(NMRDataStruct->SerName, ':');
#else
		ptr1 = strrchr(NMRDataStruct->SerName, '/');
#endif
	}
	
	if (ptr1 != NULL)
		path = DupSubstr(NMRDataStruct->SerName, ptr1 + 1 - NMRDataStruct->SerName);
	else
		path = strdup("");
	
	
	/** crucial acqus parameter file loading and processing **/
	filename = CombineStr(path, "acqus");
	if ((LoadTextFile(NMRDataStruct, &(NMRDataStruct->AcqusData), &(NMRDataStruct->AcqusLength), filename) == FILE_LOADED_OK) && (NMRDataStruct->AcqusData != NULL)) {
		
		if (GetAcqusParamValue(NMRDataStruct, "$TD", &td, PARAM_LONG) == DATA_OK)
			if (td > 0) {
				NMRDataStruct->TimeDomain = td;
				ParamFlag |= PARAM_SET_TimeDomain;
				
				if ((NMRDataStruct->TimeDomain) > 0) {
					NMRDataStruct->PointLine = ((NMRDataStruct->TimeDomain)&(~((int)0xff)));
					if (((NMRDataStruct->TimeDomain)&(0xff)) != 0x00)
						NMRDataStruct->PointLine += 0x100;
					NMRDataStruct->PointLine >>= 1;
					
					ParamFlag |= PARAM_SET_PointLine;
			
				} else
					NMRDataStruct->PointLine = 0;
			}
			
		if (GetAcqusParamValue(NMRDataStruct, "$BYTORDA", &bo, PARAM_LONG) == DATA_OK) {
			NMRDataStruct->ByteOrder = bo;
			ParamFlag |= PARAM_SET_ByteOrder;
		}
			
		if (GetAcqusParamValue(NMRDataStruct, "$SW_h", &(NMRDataStruct->AcquInfo.SWh), PARAM_DOUBLE) == DATA_OK) {
			/** just a sanity check to avoid strange values like zero, NaN, Inf or negative values **/
			if ((NMRDataStruct->AcquInfo.SWh < 1.0) || (!isnormal(NMRDataStruct->AcquInfo.SWh)))
				NMRDataStruct->AcquInfo.SWh = 1.0;
			
			NMRDataStruct->SWMh = NMRDataStruct->AcquInfo.SWh * 1.0e-6;
			ParamFlag |= PARAM_SET_SWh;
		}
		
		if (GetAcqusParamValue(NMRDataStruct, "$SFO1", &(NMRDataStruct->AcquInfo.Freq), PARAM_DOUBLE) == DATA_OK) {
			ParamFlag |= PARAM_SET_Freq;
		}
		
		
		/** are the raw data integers (DTYPA == 0) or doubles (DTYPA == 2) ? **/
		if (GetAcqusParamValue(NMRDataStruct, "$DTYPA", &dtypa, PARAM_LONG) == DATA_OK)
			NMRDataStruct->DTypA = ((dtypa < 127) && (dtypa > -127))?(dtypa):(0);
		else
			NMRDataStruct->DTypA = 0;
		
		
		/** load parameters related to the digital filter used during the acquisition (should the loading fail, the defaults are reasonable) **/
		GetAcqusParamValue(NMRDataStruct, "$DIGMOD", &(NMRDataStruct->AcquInfo.DigMod), PARAM_LONG);
		GetAcqusParamValue(NMRDataStruct, "$DSPFIRM", &(NMRDataStruct->AcquInfo.DSPFirm), PARAM_LONG);
		GetAcqusParamValue(NMRDataStruct, "$DSPFVS", &(NMRDataStruct->AcquInfo.DSPFVS), PARAM_LONG);
		GetAcqusParamValue(NMRDataStruct, "$DECIM", &(NMRDataStruct->AcquInfo.Decim), PARAM_LONG);
		GetAcqusParamValue(NMRDataStruct, "$GRPDLY", &(NMRDataStruct->AcquInfo.GrpDly), PARAM_DOUBLE);

		
		if (	(NMRDataStruct->AcquInfo.DigMod != 0) /* && (NMRDataStruct->AcquInfo.Decim > 1) */ && 
			(NMRDataStruct->AcquInfo.DSPFirm != 1) && (NMRDataStruct->AcquInfo.DSPFirm != 4)) {
			
			if ((NMRDataStruct->AcquInfo.DSPFVS >= 20) && (NMRDataStruct->AcquInfo.DSPFVS <= 23) && (NMRDataStruct->AcquInfo.GrpDly > 0.0)) 
			/** use GRPDLY **/
				GroupDelay = NMRDataStruct->AcquInfo.GrpDly;
			else 
			if ((NMRDataStruct->AcquInfo.DSPFVS >= 10) && (NMRDataStruct->AcquInfo.DSPFVS <= 13) && (NMRDataStruct->AcquInfo.Decim > 1)) {
			/** use group delay table consistent with http://sbtools.uchc.edu/help/nmr/nmr_toolkit/bruker_dsp_table.asp **/
				if (NMRDataStruct->AcquInfo.DSPFVS == 10) {
					switch (NMRDataStruct->AcquInfo.Decim) {
						case 2: GroupDelay = 44.75; break;
						
						case 4: 
						case 8: 
						case 16: 
						case 32: 
						case 64: 
						case 128: 
						case 256: 
						case 512: 
						case 1024: 
						case 2048: 
							GroupDelay = 70.5 - 31.0/(2.0*(NMRDataStruct->AcquInfo.Decim));
							break;
						
						case 3:	GroupDelay = 33.5; break;
							
						case 6: 
						case 12: 
						case 24: 
						case 48: 
						case 96: 
						case 192: 
						case 384: 
						case 768: 
						case 1536: 
							GroupDelay = 61.0 + 2.0/3.0 - 31.0/(2.0*(NMRDataStruct->AcquInfo.Decim));
							break;
						
						default:
							;
					}
				}
				
				if (NMRDataStruct->AcquInfo.DSPFVS == 11) {
					switch (NMRDataStruct->AcquInfo.Decim) {
						case 2:	GroupDelay = 46.0; break;
						case 4:	GroupDelay = 48.0; break;
						case 8:	GroupDelay = 53.25; break;
						case 16:	GroupDelay = 72.25; break;
						case 32:	GroupDelay = 72.75; break;
						
						case 64: 
						case 128: 
						case 256: 
						case 512: 
						case 1024: 
						case 2048: 
							GroupDelay = 72.0 + 128.0/(2.0*(NMRDataStruct->AcquInfo.Decim));
							break;
						
						case 3:	GroupDelay = 36.5; break;
						case 6:	GroupDelay = 50.0 + 1.0/6.0; break;
						case 12:	GroupDelay = 69.5; break;
						case 24:	GroupDelay = 70 + 1.0/6.0; break;
						case 48:	GroupDelay = 70.5; break;
							
						case 96: 
						case 192: 
						case 384: 
						case 768: 
						case 1536: 
							GroupDelay = 72.0 - 256.0/(2.0*(NMRDataStruct->AcquInfo.Decim));
							break;
						
						default:
							;
					}
				}
				
				if (NMRDataStruct->AcquInfo.DSPFVS == 12) {
					switch (NMRDataStruct->AcquInfo.Decim) {
						case 2:	GroupDelay = 46.311; break;
						case 4:	GroupDelay = 47.87; break;
						case 8:	GroupDelay = 53.289; break;
						case 16:	GroupDelay = 71.6; break;
						case 32:	GroupDelay = 72.138; break;
						case 64:	GroupDelay = 72.348; break;
						case 128:	GroupDelay = 72.524; break;
						
						case 256: 
						case 512: 
						case 1024: 
						case 2048: 
							GroupDelay = 72.0 + 128.0/(2.0*(NMRDataStruct->AcquInfo.Decim));
							break;
						
						case 3:	GroupDelay = 36.53; break;
						case 6:	GroupDelay = 50.229; break;
						case 12:	GroupDelay = 69.551; break;
						case 24:	GroupDelay = 70.184; break;
						case 48:	GroupDelay = 70.528; break;
						case 96:	GroupDelay = 70.7; break;
							
						case 192: 
						case 384: 
						case 768: 
						case 1536: 
							GroupDelay = 72.0 - 256.0/(2.0*(NMRDataStruct->AcquInfo.Decim));
							break;
						
						default:
							;
					}
				}
				
				if (NMRDataStruct->AcquInfo.DSPFVS == 13) {
					switch (NMRDataStruct->AcquInfo.Decim) {
						case 2: 
						case 4: 
						case 8: 
						case 16: 
						case 32: 
						case 64: 
						
						case 3: 
						case 6: 
						case 12: 
						case 24: 
						case 48: 
						case 96: 
							GroupDelay = 3.0 - 1.0/(2.0*(NMRDataStruct->AcquInfo.Decim));
							break;
						
						default:
							;
					}
				}
			}
		}
	} 
	
	free(filename);
	filename = NULL;

	/** Should the parameters above be unavailable, this function shall fail. **/
	if ((ParamFlag & PARAM_SET_MASK) != PARAM_SET_MASK) {
		free(path);
		path = NULL;

		return DATA_INVALID;
	}
	
#if DIGITAL_FILTER
	/** Set up the correction of digital filter artifacts **/
	if (GroupDelay > 0.0) {
		NMRDataStruct->SkipPoints = lround(ceil(GroupDelay));
		NMRDataStruct->TimeOffset = (ceil(GroupDelay) - GroupDelay)/(NMRDataStruct->SWMh);
	} else {
		/** no correction of digital filter artifacts **/
		NMRDataStruct->SkipPoints = 0;
		NMRDataStruct->TimeOffset = 0.0;
	}
#endif
	
	/** The following parameters are important, but not critical. Thus, any failure in the rest of the function does not make the whole function fail. Missing file should not result in an error message. **/

	/** extracting the filename part from ser/fid filename **/
	ptr1 = NULL;
	if (NMRDataStruct->SerName != NULL) {
#ifdef __WIN32__
		ptr1 = strrchr(NMRDataStruct->SerName, '\\');
		if (!ptr1)
			ptr1 = strrchr(NMRDataStruct->SerName, ':');
#else
		ptr1 = strrchr(NMRDataStruct->SerName, '/');
#endif
	}
	
	if (ptr1 != NULL)
		ptr1++;
	else
		ptr1 = NMRDataStruct->SerName;
	
	/** no userlist or v?list/fq?list to load in the case of fid file **/
	if ((ptr1 != NULL) && (strlen(ptr1) == 3) && ((strcmp(ptr1, "fid") == 0) || (strcmp(ptr1, "FID") == 0))) {
		free(path);
		path = NULL;

		return DATA_OK;
	}
	
	/** userlist file loading and processing **/
	
	/** check the availability of userlist silently **/
	/* filename = CombineStr(path, "ulist"); */
	filename = CombineStr(path, "ulist.out");
	if ((filename != NULL) && (test = fopen(filename, "r")))
		fclose(test);
	else {
		free(filename);
		filename = NULL;
	}
	
	if (filename == NULL) {
		filename = CombineStr(path, "userlist");
		if ((filename != NULL) && (test = fopen(filename, "r")))
			fclose(test);
		else {
			free(filename);
			filename = NULL;
		}
	}
	
	if (filename != NULL) {
		if (ReadUserlist(NMRDataStruct, filename, &UParams) == DATA_OK) {
			NMRDataStruct->AcquInfo.AssocValueType = UParams.AssocValueType;
			NMRDataStruct->AcquInfo.AssocValueVariable = UParams.AssocValueVariable;
			
			NMRDataStruct->AcquInfo.AssocValueStart = UParams.AssocValueStart;
			NMRDataStruct->AcquInfo.AssocValueStep = UParams.AssocValueStep;
			NMRDataStruct->AcquInfo.AssocValueCoef = UParams.AssocValueCoef;
			
			NMRDataStruct->AcquInfo.AssocValues = UParams.AssocValues;
			NMRDataStruct->AcquInfo.AssocValuesLength = ((UParams.AssocValues)?(UParams.StepCount):(0));

			NMRDataStruct->AcquInfo.WobbStep = UParams.WobbStep;

			/** free the userlist except the pointers assigned to AcquInfo **/
			UParams.AssocValueVariable = NULL;
			UParams.AssocValues = NULL;
			FreeUserlist(NMRDataStruct, &UParams);

			if ((NMRDataStruct->AcquInfo.AssocValueType != ASSOC_VARIABLE) && (NMRDataStruct->AcquInfo.AssocValueVariable == NULL) && (NMRDataStruct->AcquInfo.AssocValueUnits == NULL) && (NMRDataStruct->AcquInfo.AssocValueType <= ASSOC_UserlistHighest)) {
				if (VarsUnits[NMRDataStruct->AcquInfo.AssocValueType][0] != NULL)
					NMRDataStruct->AcquInfo.AssocValueVariable = strdup(VarsUnits[NMRDataStruct->AcquInfo.AssocValueType][0]);
				
				if (VarsUnits[NMRDataStruct->AcquInfo.AssocValueType][1] != NULL)
					NMRDataStruct->AcquInfo.AssocValueUnits = strdup(VarsUnits[NMRDataStruct->AcquInfo.AssocValueType][1]);
			}
			
			if ((NMRDataStruct->AcquInfo.AssocValueTypeName == NULL) && (NMRDataStruct->AcquInfo.AssocValueType <= ASSOC_UserlistHighest)) 
				if (VarsUnits[NMRDataStruct->AcquInfo.AssocValueType][2] != NULL)
					NMRDataStruct->AcquInfo.AssocValueTypeName = strdup(VarsUnits[NMRDataStruct->AcquInfo.AssocValueType][2]);
				
			free(path);
			path = NULL;
			free(filename);
			filename = NULL;
			
			return DATA_OK;
		}
	}
	
	free(filename);
	filename = NULL;
	
	
	RetVal = DATA_EMPTY;
	
	for (i = 0; i < 13; i++) {
		if (GetAcqusParamValue(NMRDataStruct, vlistAttribs[i].vlistAcqus, &(vlist), PARAM_STRING) == DATA_OK) {
			ptr1 = strchr(vlist, '<');
			ptr2 = strrchr(vlist, '>');
			if ((ptr1 != NULL) && (ptr2 != NULL) && ((strrchr(vlist, '>') - strchr(vlist, '<')) > 1)) { /** is non-empty **/
				RetVal = LoadVlist(NMRDataStruct,  &(vlistAttribs[i]), path);
				if (RetVal == DATA_OK) 
					break; 
			}
		}
	}
	
	free(vlist);
	vlist = NULL;
	free(path);
	path = NULL;
	
	return DATA_OK;
}

int InitAcquInfo(NMRData *NMRDataStruct) {
	NMRDataStruct->AcquInfo.AcquFlag = ACQU_None;
	
	NMRDataStruct->AcquInfo.StepCount = 0;
	NMRDataStruct->AcquInfo.ChunkCount = 0;
	
	NMRDataStruct->AcquInfo.AssocValueType = ASSOC_NOT_SET;
	NMRDataStruct->AcquInfo.AssocValueTypeName = NULL;
	NMRDataStruct->AcquInfo.AssocValueVariable = NULL;
	NMRDataStruct->AcquInfo.AssocValueUnits = NULL;
	NMRDataStruct->AcquInfo.AssocValueStart = NAN;
	NMRDataStruct->AcquInfo.AssocValueStep = NAN;
	NMRDataStruct->AcquInfo.AssocValueCoef = NAN;
	NMRDataStruct->AcquInfo.AssocValues = NULL;
	NMRDataStruct->AcquInfo.AssocValuesLength = 0;
	
	NMRDataStruct->AcquInfo.AssocValueMin = NAN;
	NMRDataStruct->AcquInfo.AssocValueMax = NAN;
	
	NMRDataStruct->AcquInfo.WobbStep = LONG_MAX;
	
	NMRDataStruct->AcquInfo.Title = NULL;
	NMRDataStruct->AcquInfo.Date = 0;
	
	NMRDataStruct->AcquInfo.Freq = 0.0;
	NMRDataStruct->AcquInfo.SWh = 1.0e6;
	NMRDataStruct->AcquInfo.PulProg = NULL;

	NMRDataStruct->AcquInfo.NS = 0;
	
	NMRDataStruct->AcquInfo.D = NULL;
	NMRDataStruct->AcquInfo.Dlength = 0;
	NMRDataStruct->AcquInfo.P = NULL;
	NMRDataStruct->AcquInfo.Plength = 0;
	NMRDataStruct->AcquInfo.PL = NULL;
	NMRDataStruct->AcquInfo.PLlength = 0;
	NMRDataStruct->AcquInfo.PLW = NULL;
	NMRDataStruct->AcquInfo.PLWlength = 0;

	NMRDataStruct->AcquInfo.RG = 0.0;
	NMRDataStruct->AcquInfo.FW = 0.0;
	NMRDataStruct->AcquInfo.DE = 0.0;

	NMRDataStruct->AcquInfo.DigMod = 0;
	NMRDataStruct->AcquInfo.DSPFirm = 0;
	NMRDataStruct->AcquInfo.DSPFVS = 0;
	NMRDataStruct->AcquInfo.Decim = 1;
	NMRDataStruct->AcquInfo.GrpDly = 0.0;

	return DATA_EMPTY;
}

int GetAcquInfo(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	size_t i = 0;
	int AcqusRetval = DATA_OK;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	NMRDataStruct->AcquInfo.AcquFlag = ACQU_None;
	
	NMRDataStruct->AcquInfo.StepCount = StepNoRange(NMRDataStruct);
	NMRDataStruct->AcquInfo.ChunkCount = ChunkNoRange(NMRDataStruct);
	NMRDataStruct->AcquInfo.AcquFlag |= ACQU_Counts;
	

	if (StepNoRange(NMRDataStruct) > 0) {
		NMRDataStruct->AcquInfo.AssocValueMin = HUGE_VAL;
		NMRDataStruct->AcquInfo.AssocValueMax = -HUGE_VAL;
		
		for (i = 0; i < StepNoRange(NMRDataStruct); i++) {
			if (StepAssocValue(NMRDataStruct, i) < NMRDataStruct->AcquInfo.AssocValueMin) 
				NMRDataStruct->AcquInfo.AssocValueMin = StepAssocValue(NMRDataStruct, i);
			
			if (StepAssocValue(NMRDataStruct, i) > NMRDataStruct->AcquInfo.AssocValueMax) 
				NMRDataStruct->AcquInfo.AssocValueMax = StepAssocValue(NMRDataStruct, i);
		}
		
	} else {
		NMRDataStruct->AcquInfo.AssocValueMin = NAN;
		NMRDataStruct->AcquInfo.AssocValueMax = NAN;
	}
	
	/** AssocValues based on userlist **/
	if ((NMRDataStruct->AcquInfo.AssocValueType > ASSOC_NOT_SET) && (NMRDataStruct->AcquInfo.AssocValueType <= ASSOC_UserlistHighest)) 
		NMRDataStruct->AcquInfo.AcquFlag |= ACQU_Userlist;
		
	/** AssocValues from vlist **/
	if (NMRDataStruct->AcquInfo.AssocValueType & (ASSOC_VLIST_MASK | ASSOC_FQLIST_MASK)) 
		NMRDataStruct->AcquInfo.AcquFlag |= ACQU_vlist;

	
	AcqusRetval |= GetAcqusParamValue(NMRDataStruct, "TITLE", &(NMRDataStruct->AcquInfo.Title), PARAM_STRING);
	AcqusRetval |= GetAcqusParamValue(NMRDataStruct, "$DATE", &(NMRDataStruct->AcquInfo.Date), PARAM_LONG);
	AcqusRetval |= GetAcqusParamValue(NMRDataStruct, "$PULPROG", &(NMRDataStruct->AcquInfo.PulProg), PARAM_STRING);
	AcqusRetval |= GetAcqusParamValue(NMRDataStruct, "$NS", &(NMRDataStruct->AcquInfo.NS), PARAM_LONG);

	AcqusRetval |= GetAcqusParamSet(NMRDataStruct, "$D", (void **)(&(NMRDataStruct->AcquInfo.D)), &(NMRDataStruct->AcquInfo.Dlength), PARAM_DOUBLE);
	AcqusRetval |= GetAcqusParamSet(NMRDataStruct, "$P", (void **)(&(NMRDataStruct->AcquInfo.P)), &(NMRDataStruct->AcquInfo.Plength), PARAM_DOUBLE);
	AcqusRetval |= GetAcqusParamSet(NMRDataStruct, "$PL", (void **)(&(NMRDataStruct->AcquInfo.PL)), &(NMRDataStruct->AcquInfo.PLlength), PARAM_DOUBLE);
	/*AcqusRetval |=*/ GetAcqusParamSet(NMRDataStruct, "$PLW", (void **)(&(NMRDataStruct->AcquInfo.PLW)), &(NMRDataStruct->AcquInfo.PLWlength), PARAM_DOUBLE);	/** PLW is not available in data from older spectrometers **/
	
	AcqusRetval |= GetAcqusParamValue(NMRDataStruct, "$RG", &(NMRDataStruct->AcquInfo.RG), PARAM_DOUBLE);
	AcqusRetval |= GetAcqusParamValue(NMRDataStruct, "$FW", &(NMRDataStruct->AcquInfo.FW), PARAM_DOUBLE);
	AcqusRetval |= GetAcqusParamValue(NMRDataStruct, "$DE", &(NMRDataStruct->AcquInfo.DE), PARAM_DOUBLE);
	
	if (AcqusRetval == DATA_OK)
		NMRDataStruct->AcquInfo.AcquFlag |= ACQU_Acqus;
	
	if (NMRDataStruct->AcquInfo.AcquFlag == ACQU_None)
		return DATA_INVALID;
	
	return DATA_OK;
}

int FreeAcquInfo(NMRData *NMRDataStruct) {

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	NMRDataStruct->AcquInfo.AcquFlag = ACQU_None;
	
	free(NMRDataStruct->AcquInfo.AssocValueTypeName);
	NMRDataStruct->AcquInfo.AssocValueTypeName = NULL;
	
	free(NMRDataStruct->AcquInfo.AssocValueVariable);
	NMRDataStruct->AcquInfo.AssocValueVariable = NULL;
	
	free(NMRDataStruct->AcquInfo.AssocValueUnits);
	NMRDataStruct->AcquInfo.AssocValueUnits = NULL;
	
	free(NMRDataStruct->AcquInfo.AssocValues);
	NMRDataStruct->AcquInfo.AssocValues = NULL;
	NMRDataStruct->AcquInfo.AssocValuesLength = 0;
	
	free(NMRDataStruct->AcquInfo.Title);
	NMRDataStruct->AcquInfo.Title = NULL;
	
	free(NMRDataStruct->AcquInfo.PulProg);
	NMRDataStruct->AcquInfo.PulProg = NULL;
	
	free(NMRDataStruct->AcquInfo.D);
	NMRDataStruct->AcquInfo.D = NULL;
	NMRDataStruct->AcquInfo.Dlength = 0;
	
	free(NMRDataStruct->AcquInfo.P);
	NMRDataStruct->AcquInfo.P = NULL;
	NMRDataStruct->AcquInfo.Plength = 0;
	
	free(NMRDataStruct->AcquInfo.PL);
	NMRDataStruct->AcquInfo.PL = NULL;
	NMRDataStruct->AcquInfo.PLlength = 0;
	
	free(NMRDataStruct->AcquInfo.PLW);
	NMRDataStruct->AcquInfo.PLW = NULL;
	NMRDataStruct->AcquInfo.PLWlength = 0;
	
	/* InitAcquInfo(NMRDataStruct); */
	
	return DATA_EMPTY;
}


int GetRawData(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	const int DFOK = (DATA_OK | FILE_LOADED_OK);
	int RetVal = DFOK;

	FILE *ser = NULL;
	
	unsigned char ByteLineBuffer[1024];
	char *FileIOBufffer = NULL;
	
	unsigned int ByteLine = 0;
	long ByteSize = 0;
	
	int32_t *AuxPointer = NULL;
	long i, j;
	
	register uint32_t RealB0 = 0;
	register uint32_t RealB1 = 0;
	register uint32_t RealB2 = 0;
	register uint32_t RealB3 = 0;
	
	register uint32_t ImagB0 = 0;
	register uint32_t ImagB1 = 0;
	register uint32_t ImagB2 = 0;
	register uint32_t ImagB3 = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if (NMRDataStruct->SerName == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid datafile name supplied", "Opening datafile");
		return (FILE_OPEN_ERROR | DATA_OLD | INVALID_PARAMETER);
	}
	
	
	if (NMRDataStruct->PointLine == 0) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid data parameters supplied", "Opening datafile");
		return (INVALID_PARAMETER | DATA_OLD);
	}
	
	if (NMRDataStruct->DTypA != 0) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Unsupported data format", "Opening datafile");
		return (INVALID_PARAMETER | DATA_OLD);
	}
	
	/** Open the file **/
	ser = fopen(NMRDataStruct->SerName, "rb");
	if (ser == NULL) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Opening datafile");
		return (FILE_OPEN_ERROR | DATA_OLD);
	}

	ByteLine = NMRDataStruct->PointLine * 8;

	
	/** Buffer two times longer than one step should speed up the reading **/
	FileIOBufffer = malloc(2*ByteLine);
	if (FileIOBufffer == NULL) 
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating datafile I/O buffer");
	else
		if (setvbuf(ser, FileIOBufffer, _IOFBF, 2*ByteLine) != 0) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Setting datafile I/O buffer");
			RetVal |= (FILE_IO_ERROR | DATA_OLD);
		}
	
	
	if ((RetVal == DFOK) && fseek(ser, 0, SEEK_END)) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Determining datafile size");
		RetVal |= (FILE_IO_ERROR | DATA_OLD);
	}
	
	if ((RetVal == DFOK) && ((ByteSize = ftell(ser)) == -1)) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Determining datafile size");
		RetVal |= (FILE_IO_ERROR | DATA_OLD);
	}
	
	if ((RetVal == DFOK) && fseek(ser, 0, SEEK_SET)) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Seeking datafile start");
		RetVal |= (FILE_IO_ERROR | DATA_OLD);
	}
	
	/** Is it non-empty? Is it alligned to 1024 B? **/
	if ((RetVal == DFOK) && ((ByteSize <= 0) || ((ByteSize % 1024) != 0))) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Wrong datafile size", "Checking datafile size");
		RetVal |= (FILE_WRONG_SIZE | DATA_OLD);
	}
	
	/** Memory space allocation **/
	if ((RetVal == DFOK) && ((NMRDataStruct->DataSize != ((size_t) ByteSize/4)) || (NMRDataStruct->DataSpace == NULL))) {
		AuxPointer = NMRDataStruct->DataSpace;
		NMRDataStruct->DataSpace = (int32_t *) realloc(NMRDataStruct->DataSpace, ByteSize/4*sizeof(int32_t));
		NMRDataStruct->DataSize = ByteSize/4;
		
		if (NMRDataStruct->DataSpace == NULL) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating data memory space");
			free(AuxPointer);
			AuxPointer = NULL;
			NMRDataStruct->DataSize = 0;
			RetVal |= (MEM_ALLOC_ERROR | DATA_EMPTY);
		}
	}
	
	/** Read the data to memory **/
	if (RetVal == DFOK) {
		if (NMRDataStruct->ByteOrder) {
			/** Big endian **/
			for (i = 0; i < ByteSize/1024; i++) {
				if (fread(ByteLineBuffer, 1, 1024, ser) != 1024) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading datafile");
					RetVal |= (FILE_IO_ERROR | DATA_INVALID);
					break;
				}
				
				for (j = 0; j < 128; j++) {
					RealB0 = ByteLineBuffer[j*8+0];
					RealB1 = ByteLineBuffer[j*8+1];
					RealB2 = ByteLineBuffer[j*8+2];
					RealB3 = ByteLineBuffer[j*8+3];
				
					ImagB0 = ByteLineBuffer[j*8+4];
					ImagB1 = ByteLineBuffer[j*8+5];
					ImagB2 = ByteLineBuffer[j*8+6];
					ImagB3 = ByteLineBuffer[j*8+7];
				
					(NMRDataStruct->DataSpace)[256*i+2*j+0] = (int32_t) ((RealB0 << 24) | (RealB1 << 16) | (RealB2 << 8) | (RealB3 << 0));
					(NMRDataStruct->DataSpace)[256*i+2*j+1] = (int32_t) ((ImagB0 << 24) | (ImagB1 << 16) | (ImagB2 << 8) | (ImagB3 << 0));
				}
			}
		} else {
			/** Little endian **/
			for (i = 0; i < ByteSize/1024; i++) {
				if (fread(ByteLineBuffer, 1, 1024, ser) != 1024) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading datafile");
					RetVal |= (FILE_IO_ERROR | DATA_INVALID);
					break;
				}
				
				for (j = 0; j < 128; j++) {
					RealB0 = ByteLineBuffer[j*8+0];
					RealB1 = ByteLineBuffer[j*8+1];
					RealB2 = ByteLineBuffer[j*8+2];
					RealB3 = ByteLineBuffer[j*8+3];
				
					ImagB0 = ByteLineBuffer[j*8+4];
					ImagB1 = ByteLineBuffer[j*8+5];
					ImagB2 = ByteLineBuffer[j*8+6];
					ImagB3 = ByteLineBuffer[j*8+7];
				
					(NMRDataStruct->DataSpace)[256*i+2*j+0] = (int32_t) ((RealB0 << 0) | (RealB1 << 8) | (RealB2 << 16) | (RealB3 << 24));
					(NMRDataStruct->DataSpace)[256*i+2*j+1] = (int32_t) ((ImagB0 << 0) | (ImagB1 << 8) | (ImagB2 << 16) | (ImagB3 << 24));
				}
			}
		}
	}
	
	/** Close the file **/
	if (fclose(ser) != 0) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing datafile");
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Datafile still opened, file I/O buffer not freed.", "Closing datafile");
		RetVal |= FILE_NOT_CLOSED;
	} else {
		free(FileIOBufffer);
		FileIOBufffer = NULL;
	}
	
	return RetVal;
}


int FreeRawData(NMRData *NMRDataStruct) {

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	free(NMRDataStruct->DataSpace);
	NMRDataStruct->DataSpace = NULL;
	NMRDataStruct->DataSize = 0;

	return DATA_EMPTY;
}



int AllocStepSet(NMRData *NMRDataStruct, size_t StepCount) {
	size_t i = 0;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if ((StepCount != NMRDataStruct->StepCount) || (NMRDataStruct->Steps == NULL)) {
		
		FreeStepSet(NMRDataStruct);
		
		if (StepCount > 0) {
			NMRDataStruct->Steps = (StepStruct *) malloc(StepCount*sizeof(StepStruct));
			if ((NMRDataStruct->Steps == NULL) && (StepCount != 0)) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating step set memory space");
				NMRDataStruct->StepCount = 0;
				return (MEM_ALLOC_ERROR | DATA_EMPTY);
			}
			
			NMRDataStruct->StepCount = StepCount;
			
			/** Initialize the newly allocated steps **/
			for (i = 0; i < NMRDataStruct->StepCount; i++) {
				NMRDataStruct->Steps[i].Flags = NMRDataStruct->Flags & (Flag(CHECK_AcquParams) | Flag(CHECK_RawData));
				NMRDataStruct->Steps[i].StepFlag = STEP_OK;
				NMRDataStruct->Steps[i].AssocValue = 0.0;
				NMRDataStruct->Steps[i].Freq = 0.0;
				NMRDataStruct->Steps[i].RawData = NULL;
				NMRDataStruct->Steps[i].RawDataLength = 0;
				NMRDataStruct->Steps[i].ChunkAvgData = NULL;
				NMRDataStruct->Steps[i].ChunkAvgAmp = NULL;
				NMRDataStruct->Steps[i].ChunkAvgLength = 0;
				NMRDataStruct->Steps[i].EchoPeaksEnvelope = NULL;
				NMRDataStruct->Steps[i].EchoPeaksEnvelopeLength = 0;
				NMRDataStruct->Steps[i].DFTInput = NULL;
				NMRDataStruct->Steps[i].DFTOutput = NULL;
				NMRDataStruct->Steps[i].DFTOutAmp = NULL;
				NMRDataStruct->Steps[i].DFTLength = 0;
				NMRDataStruct->Steps[i].PhaseCorrFlag = 0;
				NMRDataStruct->Steps[i].PhaseCorr0 = 0;
				NMRDataStruct->Steps[i].PhaseCorr1 = 0;
				NMRDataStruct->Steps[i].PhaseCorr1Ref = -1;
				NMRDataStruct->Steps[i].DFTPhaseCorrOutput = NULL;
				NMRDataStruct->Steps[i].DFTPhaseCorrOutAmp = NULL;
				NMRDataStruct->Steps[i].ChunkAvgAmpMax = 0.0;
				NMRDataStruct->Steps[i].ChunkAvgAmpInt = 0.0;
				NMRDataStruct->Steps[i].DFTAmpMax = 0.0;
				NMRDataStruct->Steps[i].DFTAmpMaxPoint = 0;
				NMRDataStruct->Steps[i].DFTAmpMean = 0.0;
				NMRDataStruct->Steps[i].DFTPhaseCorrRealMax = 0.0;
				NMRDataStruct->Steps[i].DFTPhaseCorrRealMaxPoint = 0;
				NMRDataStruct->Steps[i].DFTPhaseCorrRealMean = 0.0;
				NMRDataStruct->Steps[i].DFTPhaseCorrAmpMax = 0.0;
				NMRDataStruct->Steps[i].DFTPhaseCorrAmpMaxPoint = 0;
				NMRDataStruct->Steps[i].DFTPhaseCorrAmpMean = 0.0;
			}
		}
	}

	return DATA_OK;
}

int GetStepSet(NMRData *NMRDataStruct, long StepNo, unsigned long Components) {
	size_t i = 0;
	size_t j = 0;
	int32_t OrVal = 0;
	size_t AuxStepCount = 0;
	double AssocValue = 0.0;
	double AssocStep = 0.0;
	double AssocCoef = 1.0;
	int RetVal = DATA_OK;
	
	size_t PointLine = 0;
	size_t TimeDomain = 0;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	PointLine = NMRDataStruct->PointLine;
	TimeDomain = NMRDataStruct->TimeDomain;
	
	if ((TimeDomain > 2*PointLine) || (PointLine == 0)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid time domain or point line supplied", "Creating step set");
		return (DATA_OLD | INVALID_PARAMETER);
	}
	
	/** Find out the number of steps **/
	AuxStepCount = NMRDataStruct->DataSize / 2 / PointLine;
	if ((NMRDataStruct->DataSpace == NULL) || (NMRDataStruct->DataSize == 0)) 
		AuxStepCount = 0;
	
	/** Make sure that memory is available **/
	if ((RetVal = AllocStepSet(NMRDataStruct, AuxStepCount)) != DATA_OK)
		return RetVal;
	
	/** Set the pointers to starts of individual step records and set the flags if necessary **/
	for (i = 0; i < NMRDataStruct->StepCount; i++) {
		NMRDataStruct->Steps[i].RawData = NMRDataStruct->DataSpace + 2*i*PointLine;
		NMRDataStruct->Steps[i].RawDataLength = TimeDomain / 2;
	}
	
	/** Check if the step is empty or not **/
	for (i = 0; i < NMRDataStruct->StepCount; i++) {
		for (j = 0, OrVal = 0; (j < NMRDataStruct->Steps[i].RawDataLength) && (!OrVal); j++) 
			OrVal |= NMRDataStruct->Steps[i].RawData[2*j + 0] | NMRDataStruct->Steps[i].RawData[2*j + 1];
		
		if (OrVal == 0)
			NMRDataStruct->Steps[i].StepFlag |= STEP_BLANK;
		else
			NMRDataStruct->Steps[i].StepFlag &= ~STEP_BLANK;
	}
	
	
	if (isfinite(NMRDataStruct->AcquInfo.AssocValueStart) && isfinite(NMRDataStruct->AcquInfo.AssocValueStep) && isfinite(NMRDataStruct->AcquInfo.AssocValueCoef)) {
		AssocValue = NMRDataStruct->AcquInfo.AssocValueStart;
		AssocStep = NMRDataStruct->AcquInfo.AssocValueStep;
		AssocCoef = NMRDataStruct->AcquInfo.AssocValueCoef;
	} else {	/** better to use step number rather than NaN or 0 for all steps **/
		AssocValue = 0.0;
		AssocStep = 1.0;
		AssocCoef = 1.0;
	}
	
	/** Assign associated values to the steps **/
	for (i = 0; i < NMRDataStruct->StepCount; i++) {
		if (	(NMRDataStruct->AcquInfo.AssocValues != NULL) && (NMRDataStruct->AcquInfo.AssocValuesLength > 0) &&
			((i < NMRDataStruct->AcquInfo.AssocValuesLength) || (NMRDataStruct->AcquInfo.AssocValueType & (ASSOC_VLIST_MASK | ASSOC_FQLIST_MASK))) ) 
			StepAssocValue(NMRDataStruct, i) = NMRDataStruct->AcquInfo.AssocValues[(i%(NMRDataStruct->AcquInfo.AssocValuesLength))];	/** allows for wrapping the v?list/fq?list **/
		else
			StepAssocValue(NMRDataStruct, i) = AssocValue;	/**  use the value calculated from the start, step, coefficient triplet if AssocValues set is either unavailable or too short and shall be not wrapped **/
		
		if (NMRDataStruct->AcquInfo.AssocValueType == ASSOC_FREQ_MHZ)
			StepFreq(NMRDataStruct, i) = StepAssocValue(NMRDataStruct, i);
		else
			StepFreq(NMRDataStruct, i) = NMRDataStruct->AcquInfo.Freq;
		
		AssocValue += AssocStep;
		AssocStep *= AssocCoef;
	}
	
	return DATA_OK;
}


int FreeStepSet(NMRData *NMRDataStruct) {
	size_t i = 0;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if (NMRDataStruct->Steps != NULL) {
		for (i = 0; i < NMRDataStruct->StepCount; i++) {
			free(NMRDataStruct->Steps[i].ChunkAvgData);
			free(NMRDataStruct->Steps[i].ChunkAvgAmp);
			free(NMRDataStruct->Steps[i].EchoPeaksEnvelope);
		}
		
		FreeDFTResult(NMRDataStruct);
	}
	
	free(NMRDataStruct->Steps);
	NMRDataStruct->Steps = NULL;
	NMRDataStruct->StepCount = 0;

	return DATA_EMPTY;
}
