/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open Diameter Project                          */
/*                                                                        */
/* This library is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU Lesser General Public License as         */
/* published by the Free Software Foundation; either version 2.1 of the   */
/* License, or (at your option) any later version.                        */
/*                                                                        */
/* This library is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      */
/* Lesser General Public License for more details.                        */
/*                                                                        */
/* You should have received a copy of the GNU Lesser General Public       */
/* License along with this library; if not, write to the Free Software    */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307    */
/* USA.                                                                   */
/*                                                                        */
/* In addition, when you copy and redistribute some or the entire part of */
/* the source code of this software with or without modification, you     */
/* MUST include this copyright notice in each copy.                       */
/*                                                                        */
/* If you make any changes that are appeared to be useful, please send    */
/* sources that include the changed part to                               */
/* diameter-developers@lists.sourceforge.net so that we can reflect your  */
/* changes to one unified version of this software.                       */
/*                                                                        */
/* END_COPYRIGHT                                                          */

#ifndef __PANA_EXPORTS_H__
#define __PANA_EXPORTS_H__

#include "ace/config-all.h"

/*! \brief Exports file
 * 
 * This file is provided as a platform independent
 * support for exporting symbols from this DLL (windows)
 * or library (UNIX(s))
 */

#if defined(WIN32)

#include "StdAfx.h"

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PANA_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PANA_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
/*!
 * Windows specific export declarations
 */
#if defined (WIN32)
#  if defined (PANA_EXPORTS)
#    define PANA_EXPORT ACE_Proper_Export_Flag
#    define PANA_EXPORT_ONLY ACE_Proper_Export_Flag
#    define PANA_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define PANA_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else
#    define PANA_EXPORT ACE_Proper_Import_Flag
#    define PANA_EXPORT_ONLY
#    define PANA_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define PANA_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif   /* ! PANA_EXPORTS */
#else
#  define PANA_EXPORT
#  define PANA_EXPORT_ONLY
#  define PANA_SINGLETON_DECLARATION(T)
#  define PANA_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif     /* WIN32 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#else

#define PANA_EXPORT // stub only

#endif // WIN32



#endif /* __PANA_EXPORTS_H__ */

