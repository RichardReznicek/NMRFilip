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

#include "nmrfilip.h"

#include "nfulist.h"
#include "nfio.h"


int GetUserlistStyleParamValue(NMRData *NMRDataStruct, char *UserlistData, size_t UserlistLength, char *ParamName, void *ParamValue, unsigned int type) {
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	char *AuxPointer = NULL;
	long AuxLong = 0;
	unsigned long AuxULong = 0;
	double AuxDouble = 0.0;
	size_t length = 0;
	int IsNotAscii = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if ((UserlistData == NULL) || (UserlistLength == 0)) 
		return (DATA_OLD | DATA_EMPTY);
	
	if (ParamName == NULL)
		return INVALID_PARAMETER;

	if (ParamValue == NULL)
		return INVALID_PARAMETER;

	ptr1 = UserlistData;
	if (strncmp(ptr1, "\xEF\xBB\xBF", 3) == 0)
		ptr1 += 3;	/** skip UTF-8 BOM if present **/
	
	for (length = strlen(ParamName); *ptr1 != '\0'; ptr1 += strcspn(ptr1, "\r\n")) {
		//~ ptr1 += strspn(ptr1, " \t\r\n");
		ptr1 += strspn(ptr1, " \t\v\f\r\n");
		if (strncmp(ptr1, ParamName, length) == 0) 
			break;
	}

	if (*ptr1 == '\0')
		return (DATA_OLD | DATA_EMPTY);

	ptr1 += length;
	ptr1 += strspn(ptr1, " \t");
	
	switch (type) {
		case PARAM_LONG:
			errno = 0;
			AuxLong = strtol(ptr1, &ptr2, 0);
			if (errno || (ptr1 == ptr2)) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading userlist parameter value");
				return (DATA_OLD | DATA_INVALID);
			}
			*((long *)ParamValue) = AuxLong;
		break;
		
		case PARAM_ULONG:
			errno = 0;
			AuxULong = strtoul(ptr1, &ptr2, 0);
			if (errno || (ptr1 == ptr2)) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading userlist parameter value");
				return (DATA_OLD | DATA_INVALID);
			}
			*((unsigned long *)ParamValue) = AuxULong;
		break;
			
		case PARAM_DOUBLE:
			errno = 0;
			AuxDouble = strtod(ptr1, &ptr2);
			if (errno || (ptr1 == ptr2) || !isfinite(AuxDouble)) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading userlist parameter value");
				return (DATA_OLD | DATA_INVALID);
			}
			*((double *)ParamValue) = AuxDouble;
		break;
			
		case PARAM_STRING:
			length = strcspn(ptr1, " \t\n\r");
	
			/** check if its ASCII **/
			IsNotAscii = !IsAsciiSubstr(ptr1, length);
			if (IsNotAscii) {
				length = 0;
				NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Non-ASCII string skipped", "Reading userlist parameter value");
			}
			
			AuxPointer = realloc(*((char **)ParamValue), (length + 1)*sizeof(char));
			if (AuxPointer == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Memory allocation during reading userlist parameter value");
				return (DATA_OLD | MEM_ALLOC_ERROR);
			}
			*((char **)ParamValue) = AuxPointer;
			
			if (!IsNotAscii) 
				strncpy(*((char **)ParamValue), ptr1, length);
			(*((char **)ParamValue))[length] = '\0';
		break;
		
		default:
			return (DATA_OLD | INVALID_PARAMETER);
	}

	return DATA_OK;
}



EXPORT int InitUserlist(NMRData *NMRDataStruct, UserlistParams *UParams) {
	UParams->AssocValueType = ASSOC_NOT_SET;
	UParams->AssocValueVariable = NULL;
	
	UParams->AssocValueStart = NAN;
	UParams->AssocValueStep = NAN;
	UParams->AssocValueCoef = NAN;
	
	UParams->AssocValues = NULL;
	
	UParams->StepCount = 0;
	
	UParams->StepOrder = STEP_ORDER_SEQUENTIAL;
	UParams->WobbStep = LONG_MAX;
	
	UParams->Destination = NULL;
	
	UParams->RunBeforeExpWrk = NULL;
	UParams->RunBeforeExpDst = NULL;
	UParams->RunAfterExpWrk = NULL;
	UParams->RunAfterExpDst = NULL;
	UParams->RunBeforeStepWrk = NULL;
	UParams->RunBeforeStepDst = NULL;
	UParams->RunAfterStepWrk = NULL;
	UParams->RunAfterStepDst = NULL;
	
	return DATA_OK;
}


/** Read the same userlists as Emil does. **/
int ReadLegacyUserlist(NMRData *NMRDataStruct, char *UFileName, UserlistParams *UParams) {
	char *ExpType = NULL;
	char *TextData = NULL;
	size_t TextLength = 0;
	UserlistParams UPars;
	int RetVal = DATA_OK;
	int RVal = DATA_OK;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if (UFileName == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist file name supplied", "Reading userlist file");
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	}

	if (UParams == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist parameter structure supplied", "Reading userlist file");
		return INVALID_PARAMETER;
	}
	
	RetVal = LoadTextFile(NMRDataStruct, &TextData, &TextLength, UFileName);
	if (RetVal != FILE_LOADED_OK)
		return (RetVal | DATA_OLD);
	
	InitUserlist(NMRDataStruct, &UPars);
	

	RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "newname", &(UPars.Destination), PARAM_STRING);

	RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "NumExp", &(UPars.StepCount), PARAM_LONG);
	
	if ((RVal = GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "ExpType", &ExpType, PARAM_STRING)) == DATA_OK) {
		
		if (strcmp(ExpType, "spektrum") == 0) {
			UPars.AssocValueType = ASSOC_FREQ_MHZ;
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StartFreq", &(UPars.AssocValueStart), PARAM_DOUBLE);
			if ((RVal = GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "FreqStep", &(UPars.AssocValueStep), PARAM_DOUBLE)) == DATA_OK) 
				UPars.AssocValueStep /= 1.0e3;
			
			RetVal |= RVal;
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Koeficient", &(UPars.AssocValueCoef), PARAM_DOUBLE);
		} else 
		
		if (strcmp(ExpType, "buzeni") == 0) {
			UPars.AssocValueType = ASSOC_PWR_DB;
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StartDb", &(UPars.AssocValueStart), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StepDb", &(UPars.AssocValueStep), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Koeficient", &(UPars.AssocValueCoef), PARAM_DOUBLE);
		} else 
		
		if (strcmp(ExpType, "trigger") == 0) {
			UPars.AssocValueType = ASSOC_TRIG_S;
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StartTrig", &(UPars.AssocValueStart), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StepTrig", &(UPars.AssocValueStep), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Koeficient", &(UPars.AssocValueCoef), PARAM_DOUBLE);
		} else 
		
		if (strcmp(ExpType, "t2") == 0) {
			UPars.AssocValueType = ASSOC_T2_S;
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StartT2", &(UPars.AssocValueStart), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StepT2", &(UPars.AssocValueStep), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Koeficient", &(UPars.AssocValueCoef), PARAM_DOUBLE);
		} else 
		
		if (strcmp(ExpType, "invrec") == 0) {
			UPars.AssocValueType = ASSOC_INVREC_S;
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StartDelay", &(UPars.AssocValueStart), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "StepDelay", &(UPars.AssocValueStep), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Koeficient", &(UPars.AssocValueCoef), PARAM_DOUBLE);
		} else 
		
		if (strcmp(ExpType, "variable") == 0) {
			UPars.AssocValueType = ASSOC_VARIABLE;
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Variable", &(UPars.AssocValueVariable), PARAM_STRING);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Start", &(UPars.AssocValueStart), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Step", &(UPars.AssocValueStep), PARAM_DOUBLE);
			RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "Koeficient", &(UPars.AssocValueCoef), PARAM_DOUBLE);
		} else {
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Uknown experiment type", "Reading userlist file");
			RetVal |= DATA_INVALID;
		}
		
		free(ExpType);
		ExpType = NULL;
	} else {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Unable to determine experiment type", "Reading userlist file");
		RetVal |= RVal;
	}
	
	RetVal |= GetUserlistStyleParamValue(NMRDataStruct, TextData, TextLength, "WobbStep", &(UPars.WobbStep), PARAM_LONG);
	
	if ((UPars.StepCount < 0) || (UPars.AssocValueCoef < 0.0) || (UPars.WobbStep <= 0))
		RetVal |= DATA_INVALID;
	
	if (RetVal == DATA_OK) {
		FreeUserlist(NMRDataStruct, UParams);
		*UParams = UPars;
	} else {
		FreeUserlist(NMRDataStruct, &UPars);
		RetVal |= DATA_OLD;
	}
	
	if((RVal = FreeText(NMRDataStruct, &TextData, &TextLength)) != DATA_EMPTY)
		return (RetVal | RVal);

	return RetVal;
}


EXPORT int ReadUserlist(NMRData *NMRDataStruct, char *UFileName, UserlistParams *UParams) {
	char *TextData = NULL;
	size_t TextLength = 0;
	size_t AssocValuesLength = 0;
	UserlistParams UPars;
	char *ptr1 = NULL;
	int RetVal = DATA_OK;
	int RVal = DATA_OK;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;

	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if (UFileName == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist file name supplied", "Reading userlist file");
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	}

	if (UParams == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist parameter structure supplied", "Reading userlist file");
		return INVALID_PARAMETER;
	}

	/** extracting the filename part from UFileName **/
#ifdef __WIN32__
	ptr1 = strrchr(UFileName, '\\');
	if (!ptr1)
		ptr1 = strrchr(UFileName, ':');
#else
	ptr1 = strrchr(UFileName, '/');
#endif
	if (ptr1 != NULL)
		ptr1++;
	else
		ptr1 = UFileName;
	
	/** check if legacy userlist format is expected **/
	if ((ptr1 != NULL) && (strlen(ptr1) == 8) &&  ((strcmp(ptr1, "userlist") == 0) || (strcmp(ptr1, "USERLIST") == 0)) ) 
		return ReadLegacyUserlist(NMRDataStruct, UFileName, UParams);

	
	RetVal = LoadTextFile(NMRDataStruct, &TextData, &TextLength, UFileName);
	if (RetVal != FILE_LOADED_OK)
		return (RetVal | DATA_OLD);
	
	InitUserlist(NMRDataStruct, &UPars);

	/** get the parameters **/
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%AssocValueType", &(UPars.AssocValueType), PARAM_ULONG);
	if (UPars.AssocValueType == ASSOC_VARIABLE) 
		GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%AssocValueVariable", &(UPars.AssocValueVariable), PARAM_STRING);

	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%AssocValueStart", &(UPars.AssocValueStart), PARAM_DOUBLE);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%AssocValueStep", &(UPars.AssocValueStep), PARAM_DOUBLE);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%AssocValueCoef", &(UPars.AssocValueCoef), PARAM_DOUBLE);

	GetAcqusStyleParamSet(NMRDataStruct, TextData, TextLength, "%AssocValues", (void **) &(UPars.AssocValues), &AssocValuesLength, PARAM_DOUBLE);

	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%StepCount", &(UPars.StepCount), PARAM_LONG);

	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%StepOrder", &(UPars.StepOrder), PARAM_ULONG);

	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%WobbStep", &(UPars.WobbStep), PARAM_LONG);

	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%Destination", &(UPars.Destination), PARAM_STRING);

	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%RunBeforeExpWrk", &(UPars.RunBeforeExpWrk), PARAM_STRING);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%RunBeforeExpDst", &(UPars.RunBeforeExpDst), PARAM_STRING);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%RunAfterExpWrk", &(UPars.RunAfterExpWrk), PARAM_STRING);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%RunAfterExpDst", &(UPars.RunAfterExpDst), PARAM_STRING);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%RunBeforeStepWrk", &(UPars.RunBeforeStepWrk), PARAM_STRING);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%RunBeforeStepDst", &(UPars.RunBeforeStepDst), PARAM_STRING);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%RunAfterStepWrk", &(UPars.RunAfterStepWrk), PARAM_STRING);
	GetAcqusStyleParamValue(NMRDataStruct, TextData, TextLength, "%RunAfterStepDst", &(UPars.RunAfterStepDst), PARAM_STRING);

	/** check the parameters **/
	if ((UPars.AssocValueType == ASSOC_NOT_SET) || (UPars.AssocValueType > ASSOC_UserlistHighest)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "No valid associated value type found", "Reading userlist file");
		RetVal |= INVALID_PARAMETER;
	}
	
	if ((UPars.AssocValueType == ASSOC_VARIABLE) && ((UPars.AssocValueVariable == NULL) || (strlen(UPars.AssocValueVariable) == 0) || !IsAsciiStr(UPars.AssocValueVariable))) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "No valid associated value variable found", "Reading userlist file");
	}
	
	if (UPars.AssocValues != NULL) 
		UPars.StepCount = AssocValuesLength;
	
	if (UPars.StepCount <= 0) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "No valid step count found", "Reading userlist file");
		RetVal |= INVALID_PARAMETER;
	}
	
	if ((UPars.AssocValues == NULL) && (!isfinite(UPars.AssocValueStart) || !isfinite(UPars.AssocValueStep) || !isfinite(UPars.AssocValueCoef) || (UPars.AssocValueCoef < 0.0))) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "No valid associated value specifications found", "Reading userlist file");
		RetVal |= INVALID_PARAMETER;
	}

	/** not critical for reading **/
	if (UPars.StepOrder > STEP_ORDER_Highest) 
		UPars.StepOrder = STEP_ORDER_SEQUENTIAL;
	
	if (UPars.WobbStep <= 0) 
		UPars.WobbStep = LONG_MAX;

	
	if (RetVal == DATA_OK) {
		FreeUserlist(NMRDataStruct, UParams);
		*UParams = UPars;
	} else {
		FreeUserlist(NMRDataStruct, &UPars);
		RetVal |= DATA_OLD;
	}
	
	if((RVal = FreeText(NMRDataStruct, &TextData, &TextLength)) != DATA_EMPTY)
		return (RetVal | RVal);

	return RetVal;
}


/** Should produce the same userlists as Emil does **/
int WriteLegacyUserlist(NMRData *NMRDataStruct, char *UFileName, UserlistParams *UParams) {
	int RetVal = DATA_OK;
	FILE *output = NULL;
	int written = 0;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if (UFileName == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist file name supplied", "Generating userlist file");
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	}

	if (UParams == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist structure supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if (!isfinite(UParams->AssocValueStart) || !isfinite(UParams->AssocValueStep) || !isfinite(UParams->AssocValueCoef) || 
		(UParams->StepCount < 0) || (UParams->AssocValueCoef < 0.0) || (UParams->WobbStep <= 0) ) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist parameter supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	switch (UParams->AssocValueType) {
		case ASSOC_VARIABLE:
			if (UParams->AssocValueVariable == NULL) {
				NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist parameter 'Variable' supplied", "Generating userlist file");
				return INVALID_PARAMETER;
			}
			
			/** check if it is ASCII **/
			if (!IsAsciiStr(UParams->AssocValueVariable)) {
				NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Non-ASCII characters in userlist parameter 'Variable' are not supported", "Generating userlist file");
				return INVALID_PARAMETER;
			}
			
			if (UParams->Destination == NULL) {
				NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist parameter 'newname' supplied", "Generating userlist file");
				return INVALID_PARAMETER;
			}
			break;
			
		case ASSOC_T2_S:
		case ASSOC_INVREC_S:
		case ASSOC_TRIG_S:
		case ASSOC_PWR_DB:
		case ASSOC_FREQ_MHZ:
			if (UParams->Destination == NULL) {
				NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist parameter 'newname' supplied", "Generating userlist file");
				return INVALID_PARAMETER;
			}
			break;
		
		default:
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid experiment type supplied", "Generating userlist file");
			return INVALID_PARAMETER;
	}
	
	/** check if its ASCII **/
	if (!IsAsciiStr(UParams->Destination)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Non-ASCII characters in userlist parameter 'newname' are not supported", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	output = fopen(UFileName, "w");
	if (output) {
		
		written = fprintf(output, "newname  \t%s   \n"  "NumExp  \t%ld   \n", UParams->Destination, UParams->StepCount);
		
		if (written >= 0) {
			switch (UParams->AssocValueType) {
				case ASSOC_T2_S:
					written = fprintf(output, "ExpType   \tt2   \n"  "Koeficient   \t%lf   \n"  "StartT2   \t%lf  in s \n"  "StepT2   \t%lf  in s \n", 
						UParams->AssocValueCoef, UParams->AssocValueStart, UParams->AssocValueStep);
					break;
					
				case ASSOC_INVREC_S:
					written = fprintf(output, "ExpType   \tinvrec   \n"  "Koeficient   \t%lf   \n"  "StartDelay   \t%lf  in s \n"  "StepDelay   \t%lf  in s \n", 
						UParams->AssocValueCoef, UParams->AssocValueStart, UParams->AssocValueStep);
					break;
					
				case ASSOC_TRIG_S:
					written = fprintf(output, "ExpType   \ttrigger   \n"  "Koeficient   \t%lf   \n"  "StartTrig  \t%lf  in s \n"  "StepTrig  \t%lf  in s \n", 
						UParams->AssocValueCoef, UParams->AssocValueStart, UParams->AssocValueStep);
					break;
					
				case ASSOC_PWR_DB:
					written = fprintf(output, "ExpType   \tbuzeni   \n"  "Koeficient   \t%lf   \n"  "StartDb   \t%lf  in dB \n"  "StepDb   \t%lf  in dB \n", 
						UParams->AssocValueCoef, UParams->AssocValueStart, UParams->AssocValueStep);
					break;
					
				case ASSOC_FREQ_MHZ:
					written = fprintf(output, "ExpType   \tspektrum   \n"  "Koeficient   \t%lf   \n"  "StartFreq   \t%lf  in MHz \n"  "FreqStep   \t%lf  in kHz \n", 
						UParams->AssocValueCoef, UParams->AssocValueStart, (UParams->AssocValueStep)*1.0e3);
					break;
					
				case ASSOC_VARIABLE:
					written = fprintf(output, "ExpType   \tvariable   \n"  "Koeficient   \t%lf   \n"  "Variable   \t%s  \n"  "Start   \t%lf  in ? \n"  "Step   \t%lf  in ? \n", 
						UParams->AssocValueCoef, UParams->AssocValueVariable, UParams->AssocValueStart, UParams->AssocValueStep);
					break;
					
				default:	/** should never happen (see above) **/
					NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid experiment type supplied", "Generating userlist file");
					return INVALID_PARAMETER;
			}
		}
		
		if (written >= 0) 
			written = fprintf(output, "WobbStep  \t%ld   \n"  "WobbRout   \t1 \n"  "SwitchFreq  \t1200.000000   in MHz \n", UParams->WobbStep);
		
		if (written < 0) {
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Writing to userlist file failed", "Generating userlist file");
			RetVal |= FILE_IO_ERROR;
		}
		
		if (fclose(output) != 0) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing userlist file");
			RetVal |= FILE_NOT_CLOSED;
		}			
		
	} else {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Cannot open/create userlist file", "Generating userlist file");
		return FILE_OPEN_ERROR;
	}
	
	return RetVal;
}


EXPORT int WriteUserlist(NMRData *NMRDataStruct, char *UFileName, UserlistParams *UParams) {
	int RetVal = DATA_OK;
	FILE *foutput = NULL;
	char *ptr1 = NULL;

	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if (UFileName == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist file name supplied", "Generating userlist file");
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	}

	if (UParams == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid userlist structure supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}

	/** extracting the filename part from UFileName **/
#ifdef __WIN32__
	ptr1 = strrchr(UFileName, '\\');
	if (!ptr1)
		ptr1 = strrchr(UFileName, ':');
#else
	ptr1 = strrchr(UFileName, '/');
#endif
	if (ptr1 != NULL)
		ptr1++;
	else
		ptr1 = UFileName;
	
	/** check if legacy userlist format is expected **/
	if ((ptr1 != NULL) && (strlen(ptr1) == 8) && ((strcmp(ptr1, "userlist") == 0) || (strcmp(ptr1, "USERLIST") == 0))) 
		return WriteLegacyUserlist(NMRDataStruct, UFileName, UParams);
	
	
	/** check the parameters **/
	if ((UParams->AssocValueType == ASSOC_NOT_SET) || (UParams->AssocValueType > ASSOC_UserlistHighest)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid associated value type supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if ((UParams->AssocValueType == ASSOC_VARIABLE) && ((UParams->AssocValueVariable == NULL) || (strlen(UParams->AssocValueVariable) == 0) || !IsAsciiStr(UParams->AssocValueVariable))) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid associated value variable supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if (UParams->StepCount <= 0) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid step count supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if ((UParams->AssocValues == NULL) && (!isfinite(UParams->AssocValueStart) || !isfinite(UParams->AssocValueStep) || !isfinite(UParams->AssocValueCoef) || (UParams->AssocValueCoef < 0.0))) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid associated value specifications supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if (UParams->StepOrder > STEP_ORDER_Highest) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid step order supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if (UParams->WobbStep <= 0) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid tuning request supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
/*	if ((UParams->Destination != NULL) && !IsAsciiStr(UParams->Destination)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid destination name supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
*/	
	if ((UParams->RunBeforeExpWrk != NULL) && !IsAsciiStr(UParams->RunBeforeExpWrk)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid command to run in working dataset before experiment supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	if ((UParams->RunBeforeExpDst != NULL) && !IsAsciiStr(UParams->RunBeforeExpDst)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid command to run in destination dataset before experiment supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if ((UParams->RunAfterExpWrk != NULL) && !IsAsciiStr(UParams->RunAfterExpWrk)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid command to run in working dataset after experiment supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	if ((UParams->RunAfterExpDst != NULL) && !IsAsciiStr(UParams->RunAfterExpDst)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid command to run in destination dataset after experiment supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if ((UParams->RunBeforeStepWrk != NULL) && !IsAsciiStr(UParams->RunBeforeStepWrk)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid command to run in working dataset before each step supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	if ((UParams->RunBeforeStepDst != NULL) && !IsAsciiStr(UParams->RunBeforeStepDst)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid command to run in destination dataset before each step supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	if ((UParams->RunAfterStepWrk != NULL) && !IsAsciiStr(UParams->RunAfterStepWrk)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid command to run in working dataset after each step supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	if ((UParams->RunAfterStepDst != NULL) && !IsAsciiStr(UParams->RunAfterStepDst)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid command to run in destination dataset after each step supplied", "Generating userlist file");
		return INVALID_PARAMETER;
	}
	
	/** write to the userlist **/
	foutput = fopen(UFileName, "w");
	if (foutput) {
		
		WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueType", &(UParams->AssocValueType), PARAM_ULONG, &RetVal);
		if (UParams->AssocValueType == ASSOC_VARIABLE) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueVariable", &(UParams->AssocValueVariable), PARAM_STRING, &RetVal);
		
		if (isfinite(UParams->AssocValueStart) && isfinite(UParams->AssocValueStep) && isfinite(UParams->AssocValueCoef) && !(UParams->AssocValueCoef < 0.0)) {
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueStart", &(UParams->AssocValueStart), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueStep", &(UParams->AssocValueStep), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%AssocValueCoef", &(UParams->AssocValueCoef), PARAM_DOUBLE, &RetVal);
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%StepCount", &(UParams->StepCount), PARAM_LONG, &RetVal);
		}
		
		if (UParams->AssocValues != NULL)
			WriteAcqusStyleParamSetWEC(NMRDataStruct, foutput, "%AssocValues", UParams->AssocValues, UParams->StepCount, PARAM_DOUBLE, &RetVal);

		WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%StepOrder", &(UParams->StepOrder), PARAM_ULONG, &RetVal);
		
		if (UParams->WobbStep <= UParams->StepCount) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%WobbStep", &(UParams->WobbStep), PARAM_LONG, &RetVal);

		if ((UParams->Destination != NULL) && (strlen(UParams->Destination) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%Destination", &(UParams->Destination), PARAM_STRING, &RetVal);

		if ((UParams->RunBeforeExpWrk != NULL) && (strlen(UParams->RunBeforeExpWrk) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RunBeforeExpWrk", &(UParams->RunBeforeExpWrk), PARAM_STRING, &RetVal);
		if ((UParams->RunBeforeExpDst != NULL) && (strlen(UParams->RunBeforeExpDst) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RunBeforeExpDst", &(UParams->RunBeforeExpDst), PARAM_STRING, &RetVal);

		if ((UParams->RunAfterExpWrk != NULL) && (strlen(UParams->RunAfterExpWrk) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RunAfterExpWrk", &(UParams->RunAfterExpWrk), PARAM_STRING, &RetVal);
		if ((UParams->RunAfterExpDst != NULL) && (strlen(UParams->RunAfterExpDst) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RunAfterExpDst", &(UParams->RunAfterExpDst), PARAM_STRING, &RetVal);

		if ((UParams->RunBeforeStepWrk != NULL) && (strlen(UParams->RunBeforeStepWrk) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RunBeforeStepWrk", &(UParams->RunBeforeStepWrk), PARAM_STRING, &RetVal);
		if ((UParams->RunBeforeStepDst != NULL) && (strlen(UParams->RunBeforeStepDst) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RunBeforeStepDst", &(UParams->RunBeforeStepDst), PARAM_STRING, &RetVal);

		if ((UParams->RunAfterStepWrk != NULL) && (strlen(UParams->RunAfterStepWrk) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RunAfterStepWrk", &(UParams->RunAfterStepWrk), PARAM_STRING, &RetVal);
		if ((UParams->RunAfterStepDst != NULL) && (strlen(UParams->RunAfterStepDst) > 0)) 
			WriteAcqusStyleParamValueWEC(NMRDataStruct, foutput, "%RunAfterStepDst", &(UParams->RunAfterStepDst), PARAM_STRING, &RetVal);


		if (RetVal != DATA_OK) {
			NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Writing to userlist file failed", "Generating userlist file");
			RetVal |= FILE_IO_ERROR;
		}
		
		if (fclose(foutput) != 0) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing userlist file");
			RetVal |= FILE_NOT_CLOSED;
		}
		
	} else {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Cannot open/create userlist file", "Generating userlist file");
		return FILE_OPEN_ERROR;
	}
	
	return RetVal;
}


EXPORT int FreeUserlist(NMRData *NMRDataStruct, UserlistParams *UParams) {
	free(UParams->AssocValueVariable);
	UParams->AssocValueVariable = NULL;
	
	free(UParams->AssocValues);
	UParams->AssocValues = NULL;
	UParams->StepCount = 0;
	
	free(UParams->Destination);
	UParams->Destination = NULL;
	
	free(UParams->RunBeforeExpWrk);
	UParams->RunBeforeExpWrk = NULL;
	free(UParams->RunBeforeExpDst);
	UParams->RunBeforeExpDst = NULL;

	free(UParams->RunAfterExpWrk);
	UParams->RunAfterExpWrk = NULL;
	free(UParams->RunAfterExpDst);
	UParams->RunAfterExpDst = NULL;

	free(UParams->RunBeforeStepWrk);
	UParams->RunBeforeStepWrk = NULL;
	free(UParams->RunBeforeStepDst);
	UParams->RunBeforeStepDst = NULL;

	free(UParams->RunAfterStepWrk);
	UParams->RunAfterStepWrk = NULL;
	free(UParams->RunAfterStepDst);
	UParams->RunAfterStepDst = NULL;
	
	return DATA_OK;
}
