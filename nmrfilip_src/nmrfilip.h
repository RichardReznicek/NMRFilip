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

#ifndef __nmrfilip_h__
#define __nmrfilip_h__

#include "nmrfilipcmn.h"

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int MarkNMRDataOld(NMRData *NMRDataStruct, unsigned int NMRDataType, long StepNo);

/** Functions intended to be called from application **/
EXPORT int InitNMRData(NMRData *NMRDataStruct);
EXPORT int CheckNMRData(NMRData *NMRDataStruct, unsigned int NMRDataType, long StepNo);
EXPORT int RefreshNMRData(NMRData *NMRDataStruct);
EXPORT int ReloadNMRData(NMRData *NMRDataStruct);
EXPORT int FreeNMRData(NMRData *NMRDataStruct);

EXPORT void CleanupOnExit();

EXPORT int GetProcParam(NMRData *NMRDataStruct, unsigned int ParamType, unsigned int type, void *ParamValue, long *StepNo);
EXPORT int SetProcParam(NMRData *NMRDataStruct, unsigned int ParamType, unsigned int type, void *ParamValue, long *StepNo);
EXPORT int CheckProcParam(NMRData *NMRDataStruct, unsigned int ParamType, unsigned int type, void *ParamValue, long *StepNo);

EXPORT int ImportProcParams(NMRData *NMRDataStruct, char *TextFileName);

/** Text data export functions **/
EXPORT int DataToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength, unsigned int DataType);

#ifdef __cplusplus
}
#endif

#include "nfulist.h"

#endif
