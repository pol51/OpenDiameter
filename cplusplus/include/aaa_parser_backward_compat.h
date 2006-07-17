/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2006 Open Diameter Project                          */
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

#ifndef __AAA_PARSER_BACKWARD_COMPAT_H__
#define __AAA_PARSER_BACKWARD_COMPAT_H__

/*!
 * Windows specific export declarations
 */
#if defined (WIN32)
#  if defined (DIAMETERPARSER_EXPORTS)
#    define DIAMETERPARSER_EXPORT AAAPARSER_EXPORT
#    define DIAMETERPARSER_EXPORT_ONLY AAAPARSER_EXPORT_ONLY
#    define DIAMETERPARSER_SINGLETON_DECLARATION(T) AAAPARSER_SINGLETON_DECLARATION(T)
#    define DIAMETERPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) AAAPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else
#    define DIAMETERPARSER_EXPORT AAAPARSER_EXPORT
#    define DIAMETERPARSER_EXPORT_ONLY AAAPARSER_EXPORT_ONLY
#    define DIAMETERPARSER_SINGLETON_DECLARATION(T) AAAPARSER_SINGLETON_DECLARATION (T)
#    define DIAMETERPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) AAAPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif   /* ! DIAMETERPARSER_EXPORTS */
#else
#  define DIAMETERPARSER_EXPORT AAAPARSER_EXPORT
#  define DIAMETERPARSER_EXPORT_ONLY AAAPARSER_EXPORT_ONLY
#  define DIAMETERPARSER_SINGLETON_DECLARATION(T) AAAPARSER_SINGLETON_DECLARATION(T)
#  define DIAMETERPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) AAAPARSER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif     /* WIN32 */

#endif // __AAA_PARSER_BACKWARD_COMPAT_H__