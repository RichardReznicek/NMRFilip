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

#include "nfio.h"


/** Default error reporting functions **/

char *DefErrorReport(void *NMRDataStruct, int ErrorNumber, char *Activity) {
	char *ErrorDesc = strerror(ErrorNumber);

	if (errno == EINVAL)	/** invalid ErrorNumber **/
		return NULL;
	
	if (Activity != NULL) 
		fprintf(stderr, "%s failed: %s\n", Activity, ErrorDesc);
	else
		fprintf(stderr, "%s\n", ErrorDesc);
		
	return ErrorDesc;
}

char *DefErrorReportCustom(void *NMRDataStruct, char *ErrorDesc, char *Activity) {
	if (Activity != NULL) 
		fprintf(stderr, "%s failed: %s\n", Activity, ErrorDesc);
	else
		fprintf(stderr, "%s\n", ErrorDesc);
		
	return ErrorDesc;
}



/** Internal auxiliary functions **/

int IsAsciiStr(char *str) {
	unsigned char IsNotAscii;

	if (str == NULL) 
		return 0;
	
	for (IsNotAscii = '\0'; !IsNotAscii && (*str); str++) 
		IsNotAscii |= ((unsigned char) (*str)) & ~((unsigned char) 127);

	return (!IsNotAscii);
}

int IsAsciiSubstr(char *str, size_t len) {
	unsigned char IsNotAscii;
	size_t index;
	
	if (str == NULL) 
		return 0;
	
	for (index = 0, IsNotAscii = '\0'; !IsNotAscii && (index < len); index++) 
		IsNotAscii |= ((unsigned char) (str[index])) & ~((unsigned char) 127);
	
	return (!IsNotAscii);
}

char *DupSubstr(char *str, size_t len) {
	char *ptr = NULL;
	
	if (str == NULL) 
		return NULL;
	
	ptr = (char *) malloc((len + 1)*sizeof(char));
	if (ptr != NULL) {
		strncpy(ptr, str, len);
		ptr[len] = '\0';
	}
	
	return ptr;
}

char *CombineSubstr(char *str1, size_t len1, char *str2, size_t len2) {
	char *ptr = NULL;
	
	if (str1 == NULL)
		len1 = 0;
	
	if (str2 == NULL)
		len2 = 0;
	
	ptr = (char *) malloc((len1 + len2 + 1)*sizeof(char));
	if (ptr != NULL) {
		if (str1 != NULL)
			strncpy(ptr, str1, len1);
		
		if (str2 != NULL)
			strncpy(ptr + len1, str2, len2);
		
		ptr[len1 + len2] = '\0';
	}
	
	return ptr;
}

char *CombineStr(char *str1, char *str2) {
	return CombineSubstr(str1, ((str1)?(strlen(str1)):(0)), str2, ((str2)?(strlen(str2)):(0)));
}


int LoadTextFile(NMRData *NMRDataStruct, char **TextData, size_t *TextLength, char *TextFileName) {
	FILE *TextFD = NULL;
	long ByteSize = 0;
	char *AuxPointer = NULL;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if (TextFileName == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid text file name supplied", "Opening text file");
		return (FILE_OPEN_ERROR | INVALID_PARAMETER);
	}
	
	if ((TextData == NULL) || (TextLength == NULL)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid text data memory pointer supplied", "Opening text file");
		return (DATA_VOID | INVALID_PARAMETER);
	}
	
	TextFD = fopen(TextFileName, "rb");
	if (TextFD == NULL) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Opening text file");
		return (FILE_OPEN_ERROR);
	}

	
	if (fseek(TextFD, 0, SEEK_END)) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Determining text file size");
		
		if (fclose(TextFD) != 0) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing text file");
			return (FILE_IO_ERROR | DATA_OLD | FILE_NOT_CLOSED);
		}
		
		return (FILE_IO_ERROR | DATA_OLD);
	}
	
	if ((ByteSize = ftell(TextFD)) == -1) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Determining text file size");
		
		if (fclose(TextFD) != 0) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing text file");
			return (FILE_IO_ERROR | DATA_OLD | FILE_NOT_CLOSED);
		}
		
		return (FILE_IO_ERROR | DATA_OLD);
	}
	
	if (fseek(TextFD, 0, SEEK_SET)) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Seeking text file start");
		
		if (fclose(TextFD) != 0) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing text file");
			return (FILE_IO_ERROR | DATA_OLD | FILE_NOT_CLOSED);
		}
		
		return (FILE_IO_ERROR | DATA_OLD);
	}
	
	if (ByteSize == 0) {
		free(*TextData);
		*TextData = NULL;
		*TextLength = 0;
		
		if (fclose(TextFD) != 0) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing text file");
			return (FILE_LOADED_OK | DATA_OK | FILE_NOT_CLOSED);
		}
		
		return (FILE_LOADED_OK | DATA_OK);
	}

	
	if (((size_t) ByteSize != (*TextLength)) || ((*TextData) == NULL)) {
		AuxPointer = *TextData;
		*TextData = (char *) realloc(*TextData, (ByteSize + 1)*sizeof(char));
		
		if ((*TextData) == NULL) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating text data memory");
			free(AuxPointer);
			AuxPointer = NULL;
			*TextLength = 0;
		
			if (fclose(TextFD) != 0) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing text file");
				return (MEM_ALLOC_ERROR | DATA_EMPTY | FILE_NOT_CLOSED);
			}
		
			return (MEM_ALLOC_ERROR | DATA_EMPTY);
		}
		
		*TextLength = ByteSize;
	}
	

	if (fread(*TextData, 1, ByteSize, TextFD) != (size_t) ByteSize) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading text file");
		
		free(*TextData);
		*TextData = NULL;
		*TextLength = 0;

		if (fclose(TextFD) != 0) {
			NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing text file");
			return (FILE_IO_ERROR | DATA_EMPTY | FILE_NOT_CLOSED);
		}

		return (FILE_IO_ERROR | DATA_EMPTY);
	}

	(*TextData)[ByteSize] = '\0';
	
	
	if (fclose(TextFD) != 0) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Closing text file");
		return (FILE_NOT_CLOSED | DATA_OK);
	}
	
	return (FILE_LOADED_OK | DATA_OK);
}


int FreeText(NMRData *NMRDataStruct, char **TextData, size_t *TextLength) {
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if ((TextData == NULL) || (TextLength == NULL)) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid text data memory pointer supplied", "Destroying text data memory space");
		return (DATA_VOID | INVALID_PARAMETER);
	}
	
	free(*TextData);
	*TextData = NULL;
	*TextLength = 0;
	
	return DATA_EMPTY;
}


int WriteAcqusStyleParamValue(NMRData *NMRDataStruct, FILE *output, char *ParamName, void *ParamValue, unsigned int type) {
	int written = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if (output == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid output stream supplied", "Writing acqus-style parameter value");
		return INVALID_PARAMETER;
	}
	
	if (ParamName == NULL)
		return INVALID_PARAMETER;

	if (ParamValue == NULL)
		return INVALID_PARAMETER;

	switch (type) {
		case PARAM_LONG:
			written = fprintf(output, "##%s= %ld\n", ParamName, *((long *) ParamValue));
			break;
		
		case PARAM_ULONG:
			written = fprintf(output, "##%s= %lu\n", ParamName, *((unsigned long *) ParamValue));
			break;
			
		case PARAM_DOUBLE:
			written = fprintf(output, "##%s= %.15g\n", ParamName, *((double *) ParamValue));
			break;
			
		case PARAM_STRING:
			if (*((char **) ParamValue) != NULL)
				written = fprintf(output, "##%s= %s\n", ParamName, *((char **) ParamValue));
			else
				written = fprintf(output, "##%s= \n", ParamName);
			break;
		
		default:
			return INVALID_PARAMETER;
	}

	if (written < 0) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Writing to text file failed", "Writing acqus-style parameter value");
		return FILE_IO_ERROR;
	}
	
	return DATA_OK;
}


int WriteAcqusStyleParamSet(NMRData *NMRDataStruct, FILE *output, char *ParamName, void *ParamSet, size_t ParamSetLength, unsigned int type) {
	size_t i = 0;
	int written = 0, ferr = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;
	
	if (output == NULL) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Invalid output stream supplied", "Writing acqus-style parameter set");
		return INVALID_PARAMETER;
	}

	if (ParamName == NULL)
		return INVALID_PARAMETER;
	
	if (ParamSetLength == 0)
		return DATA_OK;
	
	if ((ParamSet == NULL) || (ParamSetLength > ULONG_MAX)) 
		return INVALID_PARAMETER;
	
	if ((type != PARAM_LONG) && (type != PARAM_ULONG) && (type != PARAM_DOUBLE))
		return INVALID_PARAMETER;

	/** ' ' instead of '\n' to enable using pseudo-acqus format for parameters import/export (see GetAcqusStyleParamSet()) **/
/*	written = fprintf(output, "##%s= (0..%lu)\n", ParamName, (unsigned long) (ParamSetLength - 1));	*/
	written = fprintf(output, "##%s= (0..%lu) ", ParamName, (unsigned long) (ParamSetLength - 1));
	ferr = ferr || (written < 0);

	/** NOTE: Differences from acqus file format: Line wrapping not implemented, last parameter value is followed by space **/
	for (i = 0; (i < ParamSetLength) && !ferr; i++) {
		switch (type) {
			case PARAM_LONG:
				written = fprintf(output, "%ld ", ((long *) ParamSet)[i]);
				ferr = ferr || (written < 0);
				break;
			
			case PARAM_ULONG:
				written = fprintf(output, "%lu ", ((unsigned long *) ParamSet)[i]);
				ferr = ferr || (written < 0);
				break;
				
			case PARAM_DOUBLE:
				written = fprintf(output, "%.15g ", ((double *) ParamSet)[i]);
				ferr = ferr || (written < 0);
				break;
				
		/** not implemented - uncertainity about string delimiters in acqus + complicated reallocation **/
		/*	case PARAM_STRING:
				break;
		*/	
			default:
				return INVALID_PARAMETER;
		}
	}

	if (!ferr) {
		written = fprintf(output, "\n");
		ferr = ferr || (written < 0);
	}

	if (ferr) {
		NMRDataStruct->ErrorReportCustom(NMRDataStruct, "Writing to text file failed", "Writing acqus-style parameter set");
		return FILE_IO_ERROR;
	}
	
	return DATA_OK;
}


int GetAcqusStyleParamValue(NMRData *NMRDataStruct, char *AcqusData, size_t AcqusLength, char *ParamName, void *ParamValue, unsigned int type) {
	char buf1[256];
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	char *AuxPointer = NULL;
	long AuxLong = 0;
	unsigned long AuxULong = 0;
	double AuxDouble = 0.0;
	size_t length = 0;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if ((AcqusData == NULL) || (AcqusLength == 0)) 
		return (DATA_OLD | DATA_EMPTY);
	
	if ((ParamName == NULL) || (ParamValue == NULL))
		return INVALID_PARAMETER;

	*buf1 = '\0';
	strcat(buf1, "##");
	strncat(buf1, ParamName, 128);
	strcat(buf1, "= ");

	ptr1 = AcqusData;
	if (strncmp(ptr1, "\xEF\xBB\xBF", 3) == 0)
		ptr1 += 3;	/** skip UTF-8 BOM if present **/
	
	for (length = strlen(buf1); *ptr1 != '\0'; ptr1 += strcspn(ptr1, "\r\n")) {
		ptr1 += strspn(ptr1, " \t\v\f\r\n");
		if (strncmp(ptr1, buf1, length) == 0) 
			break;
	}

	if (*ptr1 == '\0')
		return (DATA_OLD | DATA_EMPTY);

	ptr1 += length;
	
	switch (type) {
		case PARAM_LONG:
			errno = 0;
			AuxLong = strtol(ptr1, &ptr2, 0);
			if (errno || (ptr1 == ptr2)) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
				return (DATA_OLD | DATA_INVALID);
			}
			*((long *) ParamValue) = AuxLong;
		break;
		
		case PARAM_ULONG:
			errno = 0;
			AuxULong = strtoul(ptr1, &ptr2, 0);
			if (errno || (ptr1 == ptr2)) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
				return (DATA_OLD | DATA_INVALID);
			}
			*((unsigned long *) ParamValue) = AuxULong;
		break;
			
		case PARAM_DOUBLE:
			errno = 0;
			AuxDouble = strtod(ptr1, &ptr2);
			if (errno || (ptr1 == ptr2) || !isfinite(AuxDouble)) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
				return (DATA_OLD | DATA_INVALID);
			}
			*((double *) ParamValue) = AuxDouble;
		break;
			
		case PARAM_STRING:
			length = strcspn(ptr1, "\n\r");
			AuxPointer = realloc(*((char **) ParamValue), (length + 1)*sizeof(char));
			if (AuxPointer == NULL) {
				NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Memory allocation during reading acqus parameter value");
				return (DATA_OLD | MEM_ALLOC_ERROR);
			}
			*((char **) ParamValue) = AuxPointer;
			
			strncpy(*((char **) ParamValue), ptr1, length);
			(*((char **) ParamValue))[length] = '\0';
		break;
		
		default:
			return (DATA_OLD | INVALID_PARAMETER);
	}

	return DATA_OK;
}


int GetAcqusStyleParamSet(NMRData *NMRDataStruct, char *AcqusData, size_t AcqusLength, char *ParamName, void **ParamSet, size_t *ParamSetLength, unsigned int type) {
	long i = 0;
	long ParamCount = 0;
	size_t ParamSetSize = 0;
	void *AuxParamPointer = NULL;
	char *StartPointer = NULL;
	double AuxDouble = 0.0;
	size_t length = 0;
	char buf1[256];
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	
	if (NMRDataStruct == NULL)
		return NMR_DATA_STRUCT_VOID;
	
	if ((NMRDataStruct->ErrorReport == NULL) || (NMRDataStruct->ErrorReportCustom == NULL))
		return ERROR_REPORT_VOID;

	if ((AcqusData == NULL) || (AcqusLength == 0)) {
		return (DATA_OLD | DATA_EMPTY);
	}
	
	if ((ParamName == NULL) || (ParamSet == NULL) || (ParamSetLength == NULL))
		return INVALID_PARAMETER;
	
	*buf1 = 0;
	
	strcat(buf1, "##");
	strncat(buf1, ParamName, 128);
	strcat(buf1, "= (0..");
	
	ptr1 = AcqusData;
	if (strncmp(ptr1, "\xEF\xBB\xBF", 3) == 0)
		ptr1 += 3;	/** skip UTF-8 BOM if present **/
	
	for (length = strlen(buf1); *ptr1 != '\0'; ptr1 += strcspn(ptr1, "\r\n")) {
		ptr1 += strspn(ptr1, " \t\v\f\r\n");
		if (strncmp(ptr1, buf1, length) == 0) 
			break;
	}

	if (*ptr1 == '\0') 
		return (DATA_OLD | DATA_EMPTY);
	
	ptr1 += strlen(buf1);
	
	errno = 0;
	ParamCount = strtol(ptr1, &ptr2, 0);
	if (errno || (ptr1 == ptr2)) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter set range");
		return (DATA_OLD | DATA_INVALID);
	}
	ptr1 = ptr2;
	ptr2 = NULL;
	
	/** Included ' ' char to enable using pseudo-acqus format for parametes import/export (see WriteAcqusStyleParamSet()) **/
/*	ptr1 = strpbrk(ptr1, "\r\n");	*/
	ptr1 = strpbrk(ptr1, " \r\n");
	if (ptr1 == NULL) {
		return (DATA_OLD | DATA_EMPTY);
	}
	StartPointer = ptr1;
	
	if ((ParamCount < 0) || (ParamCount >= 1024)) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter set range");
		return (DATA_OLD | DATA_INVALID);
	}
	
	ParamCount++;
	
	switch (type) {
		case PARAM_LONG:
			ParamSetSize = ParamCount*sizeof(long);
		break;
		
		case PARAM_ULONG:
			ParamSetSize = ParamCount*sizeof(unsigned long);
		break;
			
		case PARAM_DOUBLE:
			ParamSetSize = ParamCount*sizeof(double);
		break;
			
	/** not implemented - uncertainity about string delimiters in acqus + complicated reallocation **/
	/*	case PARAM_STRING:
			ParamSetSize = ParamCount*sizeof(char *);
		break;
	*/	
		default:
			return (DATA_OLD | INVALID_PARAMETER);		
	}
	
	/** check the validity of the read data before touching the original ParamSet **/
	for (i = 0; i < ParamCount; i++) {
		switch (type) {
			case PARAM_LONG:
				errno = 0;
				strtol(ptr1, &ptr2, 0);
				if (errno || (ptr1 == ptr2)) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
					return (DATA_OLD | DATA_INVALID);
				}
			break;
			
			case PARAM_ULONG:
				errno = 0;
				strtoul(ptr1, &ptr2, 0);
				if (errno || (ptr1 == ptr2)) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
					return (DATA_OLD | DATA_INVALID);
				}
			break;
				
			case PARAM_DOUBLE:
				errno = 0;
				AuxDouble = strtod(ptr1, &ptr2);
				if (errno || (ptr1 == ptr2) || !isfinite(AuxDouble)) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
					return (DATA_OLD | DATA_INVALID);
				}
			break;
				
		/** not implemented - uncertainity about string delimiters in acqus + complicated reallocation **/
		/*	case PARAM_STRING:
			break;
		*/	
			
			default:
				return (DATA_OLD | INVALID_PARAMETER);
		}
		
		ptr1 = ptr2;
		ptr2 = NULL;
	}

	ptr1 = StartPointer;
	
	
	AuxParamPointer = (void *) realloc(*ParamSet, ParamSetSize);

	if (AuxParamPointer == NULL) {
		NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Allocating auxiliary memory space during reading acqus parameter set");
		return (MEM_ALLOC_ERROR | DATA_OLD);
	}

	*ParamSetLength = ParamCount;
	*ParamSet = AuxParamPointer;
	
	/** conversion errors are very unlikely here **/
	for (i = 0; i < ParamCount; i++) {
		switch (type) {
			case PARAM_LONG:
				errno = 0;
				((long *) (*ParamSet))[i] = strtol(ptr1, &ptr2, 0);
				if (errno || (ptr1 == ptr2)) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
					return DATA_INVALID;
				}
			break;
			
			case PARAM_ULONG:
				errno = 0;
				((unsigned long *) (*ParamSet))[i] = strtoul(ptr1, &ptr2, 0);
				if (errno || (ptr1 == ptr2)) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
					return DATA_INVALID;
				}
			break;
				
			case PARAM_DOUBLE:
				errno = 0;
				((double *) (*ParamSet))[i] = strtod(ptr1, &ptr2);
				if (errno || (ptr1 == ptr2) || !isfinite(((double *) (*ParamSet))[i])) {
					NMRDataStruct->ErrorReport(NMRDataStruct, errno, "Reading acqus parameter value");
					return DATA_INVALID;
				}
			break;
				
		/** not implemented - uncertainity about string delimiters in acqus + complicated reallocation **/
		/*	case PARAM_STRING:
			break;
		*/	
			
			default:
				return (DATA_INVALID | INVALID_PARAMETER);
		}
		
		ptr1 = ptr2;
		ptr2 = NULL;
	}

	return DATA_OK;
}


int GetAcqusParamValue(NMRData *NMRDataStruct, char *ParamName, void *ParamValue, unsigned int type) {
	return GetAcqusStyleParamValue(NMRDataStruct, NMRDataStruct->AcqusData, NMRDataStruct->AcqusLength, ParamName, ParamValue, type);
}


int GetAcqusParamSet(NMRData *NMRDataStruct, char *ParamName, void **ParamSet, size_t *ParamSetLength, unsigned int type) {
	return GetAcqusStyleParamSet(NMRDataStruct, NMRDataStruct->AcqusData, NMRDataStruct->AcqusLength, ParamName, ParamSet, ParamSetLength, type);
}


/** shortcuts for checking previous return value before writing the parameter value(s) **/
int WriteAcqusStyleParamValueWEC(NMRData *NMRDataStruct, FILE *output, char *ParamName, void *ParamValue, unsigned int type, int *RetVal) {
	if (RetVal == NULL)
		return INVALID_PARAMETER;
	
	if ((*RetVal) == DATA_OK)
		(*RetVal) |= WriteAcqusStyleParamValue(NMRDataStruct, output, ParamName, ParamValue, type);
	
	return (*RetVal);
}

int WriteAcqusStyleParamSetWEC(NMRData *NMRDataStruct, FILE *output, char *ParamName, void *ParamSet, size_t ParamSetLength, unsigned int type, int *RetVal) {
	if (RetVal == NULL)
		return INVALID_PARAMETER;
	
	if ((*RetVal) == DATA_OK)
		(*RetVal) |= WriteAcqusStyleParamSet(NMRDataStruct, output, ParamName, ParamSet, ParamSetLength, type);
	
	return (*RetVal);
}
