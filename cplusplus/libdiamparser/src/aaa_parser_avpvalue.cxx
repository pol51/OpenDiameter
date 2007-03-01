/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open Diameter Project                          */
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

#include "aaa_parser_avpvalue.h"

// static variables
#ifndef BOOST_SPIRIT_THREADSAFE
ACE_Mutex AvpGrammarMutex_S;
#endif

template<> void DiameterAvpValueParser::parseRawToApp() {
}

template<> void DiameterAvpValueParser::parseAppToRaw() {
}

/// Parser creators.
static DiameterAvpValueParserCreator<AnyParser>           anyParserCreator;
static DiameterAvpValueParserCreator<Integer32Parser>     integer32ParserCreator;
static DiameterAvpValueParserCreator<Integer64Parser>     integer64ParserCreator;
static DiameterAvpValueParserCreator<Utf8stringParser>    utf8stringParserCreator;
static DiameterAvpValueParserCreator<OctetstringParser>   octetstringParserCreator;
static DiameterAvpValueParserCreator<DiamidentParser>     diamidentParserCreator;
static DiameterAvpValueParserCreator<DiamuriParser>       diamuriParserCreator;
static DiameterAvpValueParserCreator<IPFilterRuleParser>  ipfilterRuleParserCreator;
static DiameterAvpValueParserCreator<GroupedParser>       groupedParserCreator;
static DiameterAvpValueParserCreator<AddressParser>       addressParserCreator;

/// Container entry creators.
static AAAAvpContainerEntryCreator<DiameterStringAvpContainerEntry>
    stringContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterInteger32AvpContainerEntry>
    integer32ContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterUnsigned32AvpContainerEntry>
    unsigned32ContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterInteger64AvpContainerEntry>
    integer64ContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterUnsigned64AvpContainerEntry>
    unsigned64ContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterURIAvpContainerEntry>
    diamuriContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterIPFilterRuleAvpContainerEntry>
    ipfilterRuleContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterGroupedAvpContainerEntry>
    groupedContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterAddressAvpContainerEntry>
    addressContainerEntryCreator;
static AAAAvpContainerEntryCreator<DiameterTimeAvpContainerEntry>
    timeContainerEntryCreator;

void
DiameterAvpTypeList_S::registerDefaultTypes()
{
  // Registering default AVP types and AVP value parsers.  This
  // function is called from the constructor.  Since DiameterAvpTypeList is
  // used as a singleton, this function is called just one time when
  // it is used at the first time.
  add(new DiameterAvpType("Any", AAA_AVP_DATA_TYPE, 0,
                    anyParserCreator, stringContainerEntryCreator));

  add(new DiameterAvpType("Integer32", AAA_AVP_INTEGER32_TYPE, 4,
                    integer32ParserCreator, integer32ContainerEntryCreator));

  add(new DiameterAvpType("Integer64", AAA_AVP_INTEGER64_TYPE, 8,
                    integer64ParserCreator, integer64ContainerEntryCreator));

  add(new DiameterAvpType("Unsigned32",  AAA_AVP_UINTEGER32_TYPE, 4,
                    integer32ParserCreator, unsigned32ContainerEntryCreator));

  add(new DiameterAvpType("Unsigned64", AAA_AVP_UINTEGER64_TYPE, 8,
                    integer64ParserCreator, unsigned64ContainerEntryCreator));

  add(new DiameterAvpType("UTF8String", AAA_AVP_UTF8_STRING_TYPE, 0,
                    utf8stringParserCreator, stringContainerEntryCreator));

  add(new DiameterAvpType("Enumerated", AAA_AVP_ENUM_TYPE, 4,
                    integer32ParserCreator, integer32ContainerEntryCreator));

  add(new DiameterAvpType("Time", AAA_AVP_TIME_TYPE, 4,
                    integer32ParserCreator, timeContainerEntryCreator));

  add(new DiameterAvpType("OctetString", AAA_AVP_STRING_TYPE, 0,
                    octetstringParserCreator, stringContainerEntryCreator));

  add(new DiameterAvpType("DiameterIdentity", AAA_AVP_DIAMID_TYPE, 0,
                    diamidentParserCreator, stringContainerEntryCreator));

  add(new DiameterAvpType("DiameterURI", AAA_AVP_DIAMURI_TYPE, 0,
                    diamuriParserCreator, diamuriContainerEntryCreator));

  add(new DiameterAvpType("IPFilterRule", AAA_AVP_IPFILTER_RULE_TYPE, 0,
                    ipfilterRuleParserCreator,
                    ipfilterRuleContainerEntryCreator));

  add(new DiameterAvpType("Grouped", AAA_AVP_GROUPED_TYPE, 0,
                    groupedParserCreator, groupedContainerEntryCreator));

  add(new DiameterAvpType("Address", AAA_AVP_ADDRESS_TYPE, 0,
                    addressParserCreator, addressContainerEntryCreator));
}

DiameterAvpTypeList_S::~DiameterAvpTypeList_S(void) 
{
  while (! empty()) {
    DiameterAvpType *p = (DiameterAvpType*)(*(begin()));
    delete p;
    pop_front();
  }
}
