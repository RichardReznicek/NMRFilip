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

#ifndef __nfproc_h__
#define __nfproc_h__

#include "nmrfilipcmn.h"

int GetChunkSet(NMRData *NMRDataStruct, long StepNo, unsigned long Components);
int FreeChunkSet(NMRData *NMRDataStruct);
int GetEchoPeaksEnvelope(NMRData *NMRDataStruct, long StepNo, unsigned long Components);
int GetChunkAvg(NMRData *NMRDataStruct, long StepNo, unsigned long Components);
int GetDFTResult(NMRData *NMRDataStruct, long StepNo, unsigned long Components);
int FreeDFTResult(NMRData *NMRDataStruct);
int GetDFTPhaseCorrPrep(NMRData *NMRDataStruct, long StepNo, unsigned long Components);
int GetDFTPhaseCorr(NMRData *NMRDataStruct, long StepNo, unsigned long Components);
int CompareDouble(const void * dVal1, const void * dVal2);
int GetDFTEnvelope(NMRData *NMRDataStruct, long StepNo, unsigned long Components);
int FreeDFTEnvelope(NMRData *NMRDataStruct);
int GetDFTRealEnvelope(NMRData *NMRDataStruct, long StepNo, unsigned long Components);
int FreeDFTRealEnvelope(NMRData *NMRDataStruct);
int GetEvaluation(NMRData *NMRDataStruct, long StepNo, unsigned long Components);

#endif
