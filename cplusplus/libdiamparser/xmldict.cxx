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
/* $Id: xmldict.cxx,v 1.30 2004/10/21 23:40:12 yohba Exp $ */
#include <iostream>
#include <cstdlib>
#include <ace/OS.h>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/validators/DTD/DTDValidator.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/util/XMLString.hpp>
#include <list>
#include <string>
#include "comlist.h"
#include "avplist.h"
#include "q_avplist.h"
#include "g_avplist.h"
#include "parser.h"
#include "diameter_parser_api.h"
#include "xmldict.h"
#include "xml.h"
#include "xml_errorreporter.h"

/// The application identifier of the currently parsing dictionary.
/// If it is currently parsing base protocol dictionary,
/// currentApplicationId is set to zero.
static AAAApplicationId currentApplicationId;

typedef struct {
  DOMNode *node;
  AAAGroupedAVP *gavp;
} AAAGroupedAVPNode;

void
parseVendor(DOMNode *n)
{
  std::string id(UtilXML::getProp(n, "id"));
  std::string name(UtilXML::getProp(n, "name"));
#ifdef DEBUG
  std::cout << "id = " << id;
  std::cout << ", name = " << name << std::endl;
#endif
}

void
parseOneAvpRule(DOMNode *n, AAAQualifiedAvpList* qavp_l,
                int protocol)
{
  AAAQualifiedAVP *qavp;
  AAADictionaryEntry *avp;
  
  std::string name(UtilXML::getProp(n, "name"));
  std::string vendorStr(UtilXML::getProp(n, "vendor-id", "0"));
  std::string minStr(UtilXML::getProp(n, "minimum", "0"));
  std::string maxStr(UtilXML::getProp(n, "maximum", "none"));
  ACE_UINT32 min = ACE_OS::atoi(minStr.c_str()); 
  ACE_UINT32 max = (maxStr == std::string("none") ? 
		   QUAL_INFINITY : ACE_OS::atoi(maxStr.c_str()));
  
#ifdef DEBUG
	 std::cout << "name = " << name;
	 std::cout << ", vendor-id = " << vendorStr << std::endl;
#endif
  qavp = new AAAQualifiedAVP;
  if ((avp = AAAAvpList::instance()->search(name.c_str(),
                                            protocol)) == NULL)
    {
      AAA_LOG(LM_ERROR, "cannot find AVP named %s\n", name.c_str());
      throw;
    }
  qavp->avp = avp; 
  qavp->qual.min = min; 
  qavp->qual.max = max; 
  qavp_l->add(qavp);
}

enum {
  AVP_RULE_TYPE_COMMAND=0,
  AVP_RULE_TYPE_GROUPED
} AVPRuleType;

void
parseAvpRule(DOMNode *n, AAAQualifiedAvpList* qavp_l,
             int protocol)
{
  n = UtilXML::getNextElementNode(n);
  if (!UtilXML::matchNode(n, "avprule"))
    {
      AAA_LOG(LM_ERROR, "Grouped AVP requires one or more AVPs.\n" );
      throw;
    }
  do 
    {
      parseOneAvpRule(n, qavp_l, protocol);
      n = UtilXML::getNextElementNode(n);
    } while (UtilXML::matchNode(n, "avprule"));
}

void
parseCommandRule(DOMNode *n, AAACommand *com, int protocol)
{
  n = UtilXML::getNextElementNode(n);
  com->avp_f = new AAAQualifiedAvpList(PARSE_TYPE_FIXED_HEAD);
  com->avp_r = new AAAQualifiedAvpList(PARSE_TYPE_REQUIRED);
  com->avp_o = new AAAQualifiedAvpList(PARSE_TYPE_OPTIONAL);
  com->avp_f2 = new AAAQualifiedAvpList(PARSE_TYPE_FIXED_TAIL);
  if (UtilXML::matchNode(n, "fixed"))
    {
      parseAvpRule(n->getFirstChild(), com->avp_f, protocol);
      n = UtilXML::getNextElementNode(n);
    }
  if (UtilXML::matchNode(n, "required"))
    {
      parseAvpRule(n->getFirstChild(), com->avp_r, protocol);
      n = UtilXML::getNextElementNode(n);
    }
  if (UtilXML::matchNode(n, "optional"))
    {
      parseAvpRule(n->getFirstChild(), com->avp_o, protocol);
      n = UtilXML::getNextElementNode(n);
    }
  if (UtilXML::matchNode(n, "fixed"))
    {
      parseAvpRule(n->getFirstChild(), com->avp_f2, protocol);
      com->avp_f2->reverse();
      n = UtilXML::getNextElementNode(n);
    }
}

void
parseCommand(DOMNode *n, int protocol)
{
  std::string name(UtilXML::getProp(n, "name"));
  std::string code(UtilXML::getProp(n, "code"));
  char defaultAppId[11]; // max value in decimal is "4294967296".
  ACE_OS::sprintf(defaultAppId, "%d", currentApplicationId);
  std::string appId(UtilXML::getProp(n, "application-id", defaultAppId));
  std::string pbit(UtilXML::getProp(n, "pbit", "0"));
  std::string ebit(UtilXML::getProp(n, "ebit", "0"));

  AAA_LOG(LM_DEBUG, "name = %s, code = %s, applid = %s\n", 
	       name.c_str(), code.c_str(), appId.c_str());

  n = UtilXML::getNextElementNode(n->getFirstChild());
  if (UtilXML::matchNode(n, "requestrules"))
    { 
      AAACommand *com = new AAACommand;
      std::string rname(name);
      rname += std::string("-Request");
      com->name = rname.c_str();
      com->code = ACE_OS::atoi(code.c_str()); 
      com->appId = ACE_OS::atoi(appId.c_str()); 
      com->flags.p = ACE_OS::atoi(pbit.c_str()); 
      com->flags.r = 1;
      com->flags.e = ACE_OS::atoi(ebit.c_str());
      com->protocol = protocol;
      parseCommandRule(n->getFirstChild(), com, protocol);
      AAACommandList::instance()->add(com);
      n = UtilXML::getNextElementNode(n);
    }
  if (UtilXML::matchNode(n, "answerrules"))
    {
      AAACommand *com = new AAACommand;
      std::string aname(name);
      aname += std::string("-Answer");
      com->name = aname.c_str();
      com->code = ACE_OS::atoi(code.c_str()); 
      com->appId = ACE_OS::atoi(appId.c_str()); 
      com->flags.p = ACE_OS::atoi(pbit.c_str()); 
      com->flags.r = 0;
      com->flags.e = ACE_OS::atoi(ebit.c_str()); 
      com->protocol = protocol;
      parseCommandRule(n->getFirstChild(), com, protocol);
      AAACommandList::instance()->add(com);
      n = UtilXML::getNextElementNode(n);
    }
}

void
parseTypedefn(DOMNode *n)
{
  std::string typeName(UtilXML::getProp(n, "type-name"));
  std::string typeParent(UtilXML::getProp(n, "type-parent", "none"));
  std::string desc(UtilXML::getProp(n, "description", "none"));
#ifdef DEBUG
  std::cout << "type-name = " << typeName;
  std::cout << ", type-parent = " << typeParent;
  std::cout << "desc = " << desc << std::endl;
#endif
}

void
parseAvpType(DOMNode *n, AAA_AVPDataType& type)
{
  std::string typeName(UtilXML::getProp(n, "type-name"));
#ifdef DEBUG
  std::cout << "type-name = " << typeName << std::endl;
#endif
  do {
    AvpType *avpt;
    avpt = AvpTypeList::instance()->search(typeName.c_str());
    if (avpt == NULL)
      {
	AAA_LOG(LM_ERROR, "Unknown AVP type %s.\n", typeName.c_str());
	throw;
      }
    type = avpt->getType();
  } while(0);
}

void
parseGrouped(AAAGroupedAVPNode& gNode, int protocol)
{
  DOMNode *n = gNode.node;
  AAAGroupedAVP* gavp = gNode.gavp;
  n = UtilXML::getNextElementNode(n->getFirstChild());
  gavp->avp_f = new AAAQualifiedAvpList(PARSE_TYPE_FIXED_HEAD);
  gavp->avp_r = new AAAQualifiedAvpList(PARSE_TYPE_REQUIRED);
  gavp->avp_o = new AAAQualifiedAvpList(PARSE_TYPE_OPTIONAL);
  gavp->avp_f2 = new AAAQualifiedAvpList(PARSE_TYPE_FIXED_TAIL);
  if (UtilXML::matchNode(n, "fixed"))
    {
      parseAvpRule(n->getFirstChild(), gavp->avp_f, protocol);
      n = UtilXML::getNextElementNode(n);
    }
  if (UtilXML::matchNode(n, "required"))
    {
      parseAvpRule(n->getFirstChild(), gavp->avp_r, protocol);
      n = UtilXML::getNextElementNode(n);
    }
  if (UtilXML::matchNode(n, "optional"))
    {
      parseAvpRule(n->getFirstChild(), gavp->avp_o, protocol);
      n = UtilXML::getNextElementNode(n);
    }
  if (UtilXML::matchNode(n, "fixed"))
    {
      parseAvpRule(n->getFirstChild(), gavp->avp_f2, protocol);
      gavp->avp_f2->reverse();
      n = UtilXML::getNextElementNode(n);
    }
}

void
parseEnum(DOMNode *n)
{
  std::string name(UtilXML::getProp(n, "name"));
  std::string code(UtilXML::getProp(n, "code"));
#ifdef DEBUG
  std::cout << "name = " << name;
  std::cout << ", code = " << code << std::endl;
#endif
}

void
parseAvp(DOMNode *n, std::list<AAAGroupedAVPNode>& gavpNodeList,
         int protocol)
{
  std::string name(UtilXML::getProp(n, "name"));
  std::string code(UtilXML::getProp(n, "code"));
  std::string mayEncrypt(UtilXML::getProp(n, "may-encript", "yes"));
  std::string mandatory(UtilXML::getProp(n, "mandatory", "must"));
  std::string protect(UtilXML::getProp(n, "protected", "may"));
  std::string vendorid(UtilXML::getProp(n, "vendor-id", "0"));
  AAA_AVPDataType type;
  AAAGroupedAVP* gavp;

#ifdef DEBUG
  std::cout << "name = " << name;
  std::cout << ", code = " << code;
  std::cout << ", may-encrypt = " << mayEncrypt;
  std::cout << ", mandatory = " << mandatory;
  std::cout << ", protected = " << protect;
  std::cout << ", vendor-id = " << vendorid << std::endl;
  std::cout << ", protocol  = " << protocol << std::endl;
#endif
  n = UtilXML::getNextElementNode(n->getFirstChild());
  if (UtilXML::matchNode(n, "type"))
    {
      parseAvpType(n, type);
      n = UtilXML::getNextElementNode(n);
    }
  else if (UtilXML::matchNode(n, "grouped"))
    {
      AAAGroupedAVPNode gavpNode;
      type = AAA_AVP_GROUPED_TYPE;
      gavp = new AAAGroupedAVP;
      gavp->code = ACE_OS::atoi(code.c_str());
      gavp->vendorId = ACE_OS::atoi(vendorid.c_str()); 
      AAAGroupedAvpList::instance()->add(gavp);  // to be revisited
						   // after parsing
						   // all avps.
      gavpNode.gavp = gavp;
      gavpNode.node = n;
      gavpNodeList.push_back(gavpNode); // parse lator.
      n = UtilXML::getNextElementNode(n);
    }
  while (UtilXML::matchNode(n, "enum"))
    { 
      parseEnum(n);
      n = UtilXML::getNextElementNode(n);
    }
  // create runtime avp dictionary 
  do {
    AAADictionaryEntry* avp = 
      new AAADictionaryEntry(ACE_OS::atoi(code.c_str()),
			     name.c_str(),
			     type,
			     ACE_OS::atoi(vendorid.c_str()),
			     AAA_AVP_FLAG_NONE,
                             protocol);
    avp->flags = AAA_AVP_FLAG_NONE;
    avp->flags |= 
      ((mayEncrypt == std::string("yes")) ? AAA_AVP_FLAG_ENCRYPT : 0);
    avp->flags |= 
      (((mandatory == std::string("may")) || 
	(mandatory == std::string("must"))) ? 
       AAA_AVP_FLAG_MANDATORY : 0);
    avp->flags |= 
      (((protect == std::string("may")) || 
	(protect == std::string("must"))) ?
       AAA_AVP_FLAG_END_TO_END_ENCRYPT : 0);
    avp->flags |= 
      ((vendorid == std::string("0")) ? 0 : AAA_AVP_FLAG_VENDOR_SPECIFIC);
    if (avp->avpCode != 0)  // Do not add "AVP" AVP
      {
	AAAAvpList::instance()->add(avp);
      }
    else {
        delete avp;
      }
  } while (0);
}

void
parseBase(DOMNode *n, int protocol)
{
  std::string uri(UtilXML::getProp(n, "uri", "default uri"));
  currentApplicationId = 0;
  DOMNode *comNode = NULL;  // Node that contains command definitions.
  std::list<AAAGroupedAVPNode> gavpNodeList;
#ifdef DEBUG
  std::cout << "uri = " << uri << std::endl;
#endif
  n = UtilXML::getNextElementNode(n->getFirstChild());
  if (UtilXML::matchNode(n, "command"))
    {
      comNode = n;
      while (UtilXML::matchNode(n, "command"))
	{
	  n = UtilXML::getNextElementNode(n);  // parse later
	}
    }
  while (UtilXML::matchNode(n, "typedefn"))
    {
      parseTypedefn(n);
      n = UtilXML::getNextElementNode(n);
    }
  while (UtilXML::matchNode(n, "avp"))
    {
      parseAvp(n, gavpNodeList, protocol);
      n = UtilXML::getNextElementNode(n);
    }
  do { // parse grouped avp definition now.
    std::list<AAAGroupedAVPNode>::iterator i;
    for (i=gavpNodeList.begin(); i!=gavpNodeList.end(); i++)
      {
	parseGrouped(*i, protocol);
      }
  } while (0);
  n = comNode;
  while (UtilXML::matchNode(n, "command"))
    {
      parseCommand(n, protocol);
      n = UtilXML::getNextElementNode(n);
    }
}

void
parseApplication(DOMNode *n, int protocol)
{
  std::string applicationId(UtilXML::getProp(n, "id"));
  currentApplicationId = atol(applicationId.c_str()); // Set the current applicationId.
  std::string applicationName(UtilXML::getProp(n, "name", "default application"));
  std::string uri(UtilXML::getProp(n, "uri", "default uri"));
  DOMNode *comNode = NULL;  // Node that contains command definitions.
  std::list<AAAGroupedAVPNode> gavpNodeList;
#ifdef DEBUG
  std::cout << "Application Identifier = " << applicationId << std::endl;
  std::cout << "Application Name = " << applicationName << std::endl;
  std::cout << "URI = " << uri << std::endl;
#endif
  n = UtilXML::getNextElementNode(n->getFirstChild());
  if (UtilXML::matchNode(n, "command"))
    {
      comNode = n;
      while (UtilXML::matchNode(n, "command"))
	{
	  n = UtilXML::getNextElementNode(n);  // parse later
	}
    }
  while (UtilXML::matchNode(n, "typedefn"))
    {
      parseTypedefn(n);
      n = UtilXML::getNextElementNode(n);
    }
  while (UtilXML::matchNode(n, "avp"))
    {
      parseAvp(n, gavpNodeList, protocol);
      n = UtilXML::getNextElementNode(n);
    }
  do { // parse grouped avp definition now.
    std::list<AAAGroupedAVPNode>::iterator i;
    for (i=gavpNodeList.begin(); i!=gavpNodeList.end(); i++)
      {
	parseGrouped(*i, protocol);
      }
  } while (0);
  n = comNode;
  while (UtilXML::matchNode(n, "command"))
    {
      parseCommand(n, protocol);
      n = UtilXML::getNextElementNode(n);
    }
}

void
parseDictionary(DOMNode *n, int protocol)
{
  n = UtilXML::getNextElementNode(n);
  while (UtilXML::matchNode(n, "vendor"))
    {
      parseVendor(n);
      n = UtilXML::getNextElementNode(n);
    }
  parseBase(n, protocol);
  n = UtilXML::getNextElementNode(n);
  // Opening each Diameter application dictionary.
  while (UtilXML::matchNode(n, "application"))
    {
      parseApplication(n, protocol);
      n = UtilXML::getNextElementNode(n);
    }
}

void
parseTree(DOMNode *n)
{
  for (n = UtilXML::getNextElementNode(n); 
       n != NULL; n = UtilXML::getNextElementNode(n))
    {
      if (!UtilXML::matchNode(n, "dictionary"))
      	{
	  AAA_LOG(LM_ERROR, "Bad dictionary format.\n");
	  throw;
	}
      std::string proto(UtilXML::getProp(n, "protocol"));
      int currentProtocol = atol(proto.c_str()); // Set the current protocol.
      parseDictionary(n->getFirstChild(), currentProtocol);
    }
}

void
parseXMLDictionary(char* xmlFile)
{
  try
    {
      XMLPlatformUtils::Initialize();
    }
  catch (const XMLException& toCatch)
    {
      ACE_UNUSED_ARG(toCatch);
      AAA_LOG(LM_ERROR, "Error during initialization! Message:\n");
      XMLPlatformUtils::Terminate();
      throw;
    }

  int errorCount = 0;
  XercesDOMParser::ValSchemes    valScheme = XercesDOMParser::Val_Auto;
  //  XercesDOMParser* parser = new XercesDOMParser;
  //  myDOMTreeErrorReporter *errReporter = new myDOMTreeErrorReporter();
  std::auto_ptr<XercesDOMParser> parser(new XercesDOMParser);
  std::auto_ptr<myDOMTreeErrorReporter> errReporter(new myDOMTreeErrorReporter());
  parser->setErrorHandler(errReporter.get());
  bool errorsOccured = false;

  parser->setValidationScheme(valScheme);
  try
    {
      parser->parse(xmlFile);
      errorCount = parser->getErrorCount();
      if (errorCount > 0)
      {
	AAA_LOG(LM_ERROR, "Error while parsing Dictionary XML file.\n");
	errorsOccured = true;
      }
    }
  catch (const DOMException& e)
    {
      ACE_UNUSED_ARG(e);
      AAA_LOG(LM_ERROR, "DOM error.\n");
      errorCount = parser->getErrorCount(); 
      errorsOccured = true;
    }
  catch (const XMLException& e)
    {
      ACE_UNUSED_ARG(e);
      AAA_LOG(LM_ERROR, "XML error.\n");
      errorsOccured = true;
    }

  if(errorsOccured)
  {
    //	delete errReporter;
    //	delete parser;
	XMLPlatformUtils::Terminate();
	throw (1);
  }
  
  DOMNode *doc = parser->getDocument();
  parseTree(doc->getFirstChild());
}
