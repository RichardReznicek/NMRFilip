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

#ifndef __nfio_h__
#define __nfio_h__

#include "nmrfilipcmn.h"

/** Default error reporting functions **/
char *DefErrorReport(void *NMRDataStruct, int ErrorNumber, char *Activity);
char *DefErrorReportCustom(void *NMRDataStruct, char *ErrorDesc, char *Activity);

/** Internal auxiliary functions **/
int IsAsciiStr(char *str);
int IsAsciiSubstr(char *str, size_t len);
char *DupSubstr(char *str, size_t len);
char *CombineSubstr(char *str1, size_t len1, char *str2, size_t len2);
char *CombineStr(char *str1, char *str2);

int LoadTextFile(NMRData *NMRDataStruct, char **TextData, size_t *TextLength, char *TextFileName);
int FreeText(NMRData *NMRDataStruct, char **TextData, size_t *TextLength);
int WriteAcqusStyleParamValue(NMRData *NMRDataStruct, FILE *output, char *ParamName, void *ParamValue, unsigned int type);
int WriteAcqusStyleParamSet(NMRData *NMRDataStruct, FILE *output, char *ParamName, void *ParamSet, size_t ParamSetLength, unsigned int type);
int GetAcqusStyleParamValue(NMRData *NMRDataStruct, char *AcqusData, size_t AcqusLength, char *ParamName, void *ParamValue, unsigned int type);
int GetAcqusStyleParamSet(NMRData *NMRDataStruct, char *AcqusData, size_t AcqusLength, char *ParamName, void **ParamSet, size_t *ParamSetLength, unsigned int type);
int GetAcqusParamValue(NMRData *NMRDataStruct, char *ParamName, void *ParamValue, unsigned int type);
int GetAcqusParamSet(NMRData *NMRDataStruct, char *ParamName, void **ParamSet, size_t *ParamSetLength, unsigned int type);
int WriteAcqusStyleParamValueWEC(NMRData *NMRDataStruct, FILE *output, char *ParamName, void *ParamValue, unsigned int type, int *RetVal);
int WriteAcqusStyleParamSetWEC(NMRData *NMRDataStruct, FILE *output, char *ParamName, void *ParamSet, size_t ParamSetLength, unsigned int type, int *RetVal);

#endif