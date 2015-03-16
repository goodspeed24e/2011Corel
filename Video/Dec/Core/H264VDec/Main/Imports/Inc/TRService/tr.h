//=============================================================================
//  THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
//  ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//  ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE
//  COMPANY.
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2000  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _TR_H
#define _TR_H

#include	"../TRService.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Below block is for backward-compatible purpose. Don't use them anymore.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//
// This is the fundamental tamper resistance include file
//

// the following config files (Default, Production, Evaluation)
// turns on/off debug print, tamper resistance, time bomb

// MANUAL build: uncomment the following to overwrite default config
// AUTO build: no need to change
#if defined(_DEBUGABLE) || defined (USE_TAKARA)
// eval-dbg config: DP off, TR off, TB on NO-CSS - this is for customers who need debug copies. cannot play css discs.
#include "trcfgdbg.h"
#else
// default config: DP on, TR off, TB off
#include "trcfg.h"
#endif // _DEBUGABLE
// production config: DP off, TR on, TB off
//#include "trcfgprod.h"
// eval config: DP off, TR on, TB on
//#include "trcfgeval.h"

/////////////////////////////////////////////////////////////////////////
// timb bomb days from start of 1998 (assuming months are 30 days long and years 365 days long).
// 7-15-1999 = 560, 12-30-1998= 360
#define TR_TIMEBOMB_DAY_LOW			455
#define TR_TIMEBOMB_DAY_HIGH		560

// Prevent macro redefinition because TR_TREXE_ENABLE will typically be defined 
// by compiler /D options.
#if !defined(TR_TREXE_ENABLE) && !defined(_XENON) && !defined(_WIN32_WCE) && !defined(USE_OVIA)
#define TR_TREXE_ENABLE				// always enable the tamper resistance functions
#endif

#define TR_MAX_PLAY_DELAY_16MC		((400/16)*3)	// 3 seconds on a PII-400


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Below block is for new TR mechanism. They will be maintained by TR team.
// Do any new update on this block.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// always enable IVIScramble definition because TREXE can apply on modules even when new TR is off
#include "IVIScramble.h"
#include "LIBTR.h"

#endif // _TR_H
