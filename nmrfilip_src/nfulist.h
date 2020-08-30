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

#ifndef __nfulist_h__
#define __nfulist_h__

#include "nmrfilipcmn.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Produces either the pseudo-acqus-style ulists or the same legacy userlists as the Emil software does. **/
EXPORT int InitUserlist(NMRData *NMRDataStruct, UserlistParams *UParams);
EXPORT int ReadUserlist(NMRData *NMRDataStruct, char *UFileName, UserlistParams *UParams);
EXPORT int WriteUserlist(NMRData *NMRDataStruct, char *UFileName, UserlistParams *UParams);
EXPORT int FreeUserlist(NMRData *NMRDataStruct, UserlistParams *UParams);

#ifdef __cplusplus
}
#endif

#endif