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

// Common utility for XML/DOM handling

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>

#include "xml.h"

DOMNode*
UtilXML::getNextElementNode(DOMNode *n)
{
  for (n=n->getNextSibling(); n != NULL; n=n->getNextSibling())
    {
      if (n->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          return n;
        }
    }
  return n;
}

bool
UtilXML::matchNode(const DOMNode *n, const char *name)
{
  if (n == NULL)
    return false;

  std::string nodeName(transcodeFromXMLCh(n->getNodeName()));
  return (nodeName == std::string(name));
}

std::string
UtilXML::getProp(const DOMNode *n, const char *name)
{
  XMLCh *tmp = XMLString::transcode(name); // XML transcoder allocates memory
  DOMElement *e = (DOMElement*)n;
  const XMLCh *c_data = e->getAttribute(tmp);
  XMLString::release(&tmp);
  if (c_data == NULL)
    {
      std::cout << e->getTagName() << "requires " << name 
		<< "attribute" << std::endl;
      exit(1);
    }
  
 return (transcodeFromXMLCh(c_data));
}

std::string
UtilXML::getProp(const DOMNode *n, const char* name, char* const dflt)
{
  if (dflt == NULL)
    {
      AAA_LOG(LM_ERROR, "default string cannot be null\n");
      exit(1);
    }
  XMLCh *tmp = XMLString::transcode(name);  // XML transcoder allocates memory
  DOMElement *e = (DOMElement*)n;
  const XMLCh *c_data = e->getAttribute(tmp);
  XMLString::release(&tmp);
  std::string s1(dflt);
  if (c_data == NULL)
    {
      return s1;
    }
  std::string s2(transcodeFromXMLCh(c_data));
  if (s2.size() == 0) // #IMPLIED field contains null string
    {
      return s1;
    }
  return s2;
}

std::string
UtilXML::transcodeFromXMLCh(const XMLCh *xmlCh) 
{
  char *c_str;
  c_str = XMLString::transcode(xmlCh); // XML transcoder allocates memory
  std::string s(c_str);
  XMLString::release(&c_str);
  return s;
}
