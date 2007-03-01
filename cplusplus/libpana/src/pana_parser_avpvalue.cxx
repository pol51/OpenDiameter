/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open PANA_: Open-source software for the PANA_ and               */
/*                PANA_ related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open PANA_ Project                          */
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
/* $Id: parser_avpvalue.cxx,v 1.21 2006/04/21 19:15:39 vfajardo Exp $ */

#include "pana_parser_avpvalue.h"

template<> void PANA_AvpValueParser::parseRawToApp() {
}

template<> void PANA_AvpValueParser::parseAppToRaw() {
}

/// Parser creators.
static PANA_AvpValueParserCreator<PANA_AnyParser>           anyParserCreator;
static PANA_AvpValueParserCreator<PANA_Integer32Parser>     integer32ParserCreator;
static PANA_AvpValueParserCreator<PANA_Integer64Parser>     integer64ParserCreator;
static PANA_AvpValueParserCreator<PANA_Utf8stringParser>    utf8stringParserCreator;
static PANA_AvpValueParserCreator<PANA_OctetstringParser>   octetstringParserCreator;
static PANA_AvpValueParserCreator<PANA_GroupedParser>       groupedParserCreator;
static PANA_AvpValueParserCreator<PANA_AddressParser>       addressParserCreator;

/// Container entry creators.
static AAAAvpContainerEntryCreator<PANA_StringAvpContainerEntry>
    stringContainerEntryCreator;
static AAAAvpContainerEntryCreator<PANA_Integer32AvpContainerEntry>
    integer32ContainerEntryCreator;
static AAAAvpContainerEntryCreator<PANA_Unsigned32AvpContainerEntry>
    unsigned32ContainerEntryCreator;
static AAAAvpContainerEntryCreator<PANA_Integer64AvpContainerEntry>
    integer64ContainerEntryCreator;
static AAAAvpContainerEntryCreator<PANA_Unsigned64AvpContainerEntry>
    unsigned64ContainerEntryCreator;
static AAAAvpContainerEntryCreator<PANA_GroupedAvpContainerEntry>
    groupedContainerEntryCreator;
static AAAAvpContainerEntryCreator<PANA_AddressAvpContainerEntry>
    addressContainerEntryCreator;
static AAAAvpContainerEntryCreator<PANA_TimeAvpContainerEntry>
    timeContainerEntryCreator;

void PANA_AvpTypeList_S::DefaultTypes()
{
  // Registering default AVP types and AVP value parsers.  This
  // function is called from the constructor.  Since PANA_AvpTypeList is
  // used as a singleton, this function is called just one time when
  // it is used at the first time.
  add(new PANA_AvpType("Any", AAA_AVP_DATA_TYPE, 0,
                    anyParserCreator, stringContainerEntryCreator));

  add(new PANA_AvpType("Integer32", AAA_AVP_INTEGER32_TYPE, 4,
                    integer32ParserCreator, integer32ContainerEntryCreator));

  add(new PANA_AvpType("Integer64", AAA_AVP_INTEGER64_TYPE, 8,
                    integer64ParserCreator, integer64ContainerEntryCreator));

  add(new PANA_AvpType("Unsigned32",  AAA_AVP_UINTEGER32_TYPE, 4,
                    integer32ParserCreator, unsigned32ContainerEntryCreator));

  add(new PANA_AvpType("Unsigned64", AAA_AVP_UINTEGER64_TYPE, 8,
                    integer64ParserCreator, unsigned64ContainerEntryCreator));

  add(new PANA_AvpType("UTF8String", AAA_AVP_UTF8_STRING_TYPE, 0,
                    utf8stringParserCreator, stringContainerEntryCreator));

  add(new PANA_AvpType("Enumerated", AAA_AVP_ENUM_TYPE, 4,
                    integer32ParserCreator, integer32ContainerEntryCreator));

  add(new PANA_AvpType("Time", AAA_AVP_TIME_TYPE, 4,
                    integer32ParserCreator, timeContainerEntryCreator));

  add(new PANA_AvpType("OctetString", AAA_AVP_STRING_TYPE, 0,
                    octetstringParserCreator, stringContainerEntryCreator));

  add(new PANA_AvpType("Grouped", AAA_AVP_GROUPED_TYPE, 0,
                    groupedParserCreator, groupedContainerEntryCreator));

  add(new PANA_AvpType("Address", AAA_AVP_ADDRESS_TYPE, 0,
                    addressParserCreator, addressContainerEntryCreator));
}

PANA_AvpTypeList_S::~PANA_AvpTypeList_S(void)
{
  while (! empty()) {
    PANA_AvpType *p = (PANA_AvpType*)(*(begin()));
    delete p;
    pop_front();
  }
}
