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

#ifndef __nmrfilipdyn__
#define __nmrfilipdyn__

#include "nmrfilipcmn.h"

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Functions intended to be called from application **/
typedef int (*InitNMRDataFunc)(NMRData *);
typedef int (*CheckNMRDataFunc)(NMRData *, unsigned int, long);
typedef int (*RefreshNMRDataFunc)(NMRData *);
typedef int (*ReloadNMRDataFunc)(NMRData *);
typedef int (*FreeNMRDataFunc)(NMRData *);

typedef void (*CleanupOnExitFunc)();

typedef int (*GetProcParamFunc)(NMRData *, unsigned int, unsigned int, void *, long *);
typedef int (*SetProcParamFunc)(NMRData *, unsigned int, unsigned int, void *, long *);
typedef int (*CheckProcParamFunc)(NMRData *, unsigned int, unsigned int, void *, long *);
typedef int (*ImportProcParamsFunc)(NMRData *, char *);

/** Text data export functions **/
typedef int (*DataToTextFunc)(NMRData *, FILE *, char **, size_t *, unsigned int );

/** Functions handling ulist files and legacy userlists **/
typedef int (*InitUserlistFunc)(NMRData *, UserlistParams *);
typedef int (*ReadUserlistFunc)(NMRData *, char *, UserlistParams *);
typedef int (*WriteUserlistFunc)(NMRData *, char *, UserlistParams *);
typedef int (*FreeUserlistFunc)(NMRData *, UserlistParams *);

#ifdef __cplusplus
}
#endif

#endif
