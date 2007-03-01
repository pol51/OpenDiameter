/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
/*                                                                        */
/* Copyright (C) 2002-2007 Toshiba America Research, Inc.                 */
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
/* diameter-devel@toshibaamericaresearch.com so that we can reflect your  */
/* changes to one unified version of this software.                       */
/*                                                                        */
/* END_COPYRIGHT                                                          */

#include "xml.h"
#include "xml_errorreporter.h"
#include "eap_tls_xml_parser.hxx"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

EAPTLS_XMLElementParser::EAPTLS_XMLElementParser(std::string &tagName)
{
   this->name_ = tagName;
}

int EAPTLS_XMLElementParser::populate(DOMNode *n, DOMNode **found)
{
   DOMNode *sibling = n;
   while (sibling != NULL) {

      if (sibling->getNodeType() == DOMNode::ELEMENT_NODE) {

         char *c_str = XMLString::transcode(sibling->getNodeName());

         if (XMLString::compareString(c_str, this->name_.c_str()) == 0) {

             XMLString::release(&c_str);
          
            if (found) {
	       *found = sibling;
	    }

 	    return this->svc(sibling);
         }

         XMLString::release(&c_str);
      }
      sibling = sibling->getNextSibling();
   }
   return (-1);
}

char *EAPTLS_XMLElementParser::getTextContent(DOMNode *n)
{
   DOMNode *child = n->getFirstChild();
   if (child && child->getNodeType() == DOMNode::TEXT_NODE) {
    
      const XMLCh *xmlCh = child->getNodeValue();
      if (xmlCh) {
         return XMLString::transcode(xmlCh);
      }
   }
   return (NULL);
}

int EAPTLS_XMLTreeParser::open(std::string &filename, EAPTLS_XMLElementParser &root)
{
   int rcode = (-1);

   try {
      XMLPlatformUtils::Initialize();

      XercesDOMParser parser;
      myDOMTreeErrorReporter errReporter;

      parser.setValidationScheme(XercesDOMParser::Val_Always);
      parser.setDoNamespaces(true);
      parser.setDoSchema(true);
      parser.setErrorHandler(&errReporter);

      try {
         parser.parse(filename.data());

         if (parser.getErrorCount() == 0) {
            DOMNode *doc = parser.getDocument();
            rcode = root.populate(doc->getFirstChild());
         }
      }
      catch (const DOMException& e) {
          ACE_UNUSED_ARG(e);
      }
      catch (const XMLException& e) {
          ACE_UNUSED_ARG(e);
      }
   }
   catch (const XMLException& toCatch) {
      ACE_UNUSED_ARG(toCatch);
   }

   this->close();
   return (rcode);
}

void EAPTLS_XMLTreeParser::close()
{
   XMLPlatformUtils::Terminate();
}

int EAPTLS_XMLDataString::svc(DOMNode *n)
{
   char *c_str = this->getTextContent(n);
   if (c_str) {
      payload.assign(c_str);
      XMLString::release(&c_str);
      return (0);
   }
   return (-1);
}

int EAPTLS_XMLDataUInt32::svc(DOMNode *n)
{
   char *c_str = this->getTextContent(n);
   if (c_str) {
      payload = ACE_OS::atoi(c_str);
      XMLString::release(&c_str);
      return (0);
   }
   return (-1);
}

int EAPTLS_XMLDataBoolean::svc (DOMNode *n)
{
	char *c_str = this->getTextContent(n);
	if (c_str)
	{
		std::string value;
		value.assign(c_str);
		if (value == std::string("true"))
		{
			payload = true;
		}
		else if (value == std::string("false"))
		{
			payload = false;
		}
		else
		{
			XMLString::release(&c_str);
			return (-1);
		}

		XMLString::release(&c_str);
		return (0);
	}

	return (-1);
}




