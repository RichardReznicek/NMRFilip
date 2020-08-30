/* 
 * NMRFilip GUI - the NMR data processing software - graphical user interface
 * Copyright (C) 2010, 2020 Richard Reznicek
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

#ifndef __nmrdata_cd__
#define __nmrdata_cd__

#include "nmrfilipdyn.h"
#include "plotgen_cd.h"

typedef size_t (*GetNMRIndexRangeFuncPtr)(NMRData*, size_t);
typedef unsigned long (*GetNMRFlagFuncPtr)(NMRData*, size_t);
typedef wxPoint (*GetNMRPtFuncPtr)(NMRData*, size_t, size_t, NFGScale);
typedef wxRealPoint (*GetNMRRPtFuncPtr)(NMRData*, size_t, size_t);
typedef void (*GetNMRRPtBBFuncPtr)(NMRData*, size_t, double &, double &, double &, double &);
typedef void (*GetNMRPtsFuncPtr)(NMRData*, size_t, wxPoint*, NFGScale);

#endif
