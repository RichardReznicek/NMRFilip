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

#ifndef __nfexport_h__
#define __nfexport_h__

#include "nmrfilipcmn.h"

/** Text data export functions **/
int AcquInfoToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int ProcParamsToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int TDDToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int ChunkSetToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int ChunkAvgToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int DFTResultToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int DFTPhaseCorrectedResultToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int DFTEnvelopeToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int DFTPhaseCorrRealEnvelopeToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int EvaluationToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);
int EchoPeaksEnvelopeToText(NMRData *NMRDataStruct, FILE *foutput, char **soutput, size_t *slength);

#endif