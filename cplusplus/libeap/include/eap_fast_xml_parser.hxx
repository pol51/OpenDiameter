/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open Diameter Project                          */
/*                                                                        */
/* This program is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation; either version 2 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
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

#ifndef __EAP_FAST_XML_PARSER_H__
#define __EAP_FAST_XML_PARSER_H__

#include <string>
#include <xercesc/dom/DOM.hpp>
#include "aaa_parser_api.h"
#include "eap_log.hxx"

#if defined(XERCES_HAS_CPP_NAMESPACE)
using namespace xercesc;
#endif

typedef ACE_UINT32	diameter_unsigned32_t;

class EAPFAST_XMLElementParser {
   public:
      EAPFAST_XMLElementParser(std::string &tagName);
      virtual ~EAPFAST_XMLElementParser() { }

      virtual int svc(DOMNode *n) = 0;

      std::string &name() { return this->name_; }

      int populate(DOMNode *n, DOMNode **found = NULL);

      char *getTextContent(DOMNode *n);

   protected:
      std::string name_;
};

class EAPFAST_XMLTreeParser {
   public:
      int open(std::string &filename, EAPFAST_XMLElementParser &root);
      void close();
};

class EAPFAST_XMLDataString : public EAPFAST_XMLElementParser {
   public:
      EAPFAST_XMLDataString(std::string &name, std::string &data) : 
	EAPFAST_XMLElementParser(name),
	payload(data) { }

      virtual int svc(DOMNode *n);

   protected:
      std::string &payload;
};

class EAPFAST_XMLDataUInt32 : public EAPFAST_XMLElementParser {
   public:
      EAPFAST_XMLDataUInt32(std::string &name, diameter_unsigned32_t &data) : 
	EAPFAST_XMLElementParser(name),
	payload(data) { }

      virtual int svc(DOMNode *n);

   protected:
      diameter_unsigned32_t &payload;
};

class EAPFAST_XMLDataBoolean : public EAPFAST_XMLElementParser
{
	public:
		EAPFAST_XMLDataBoolean (std::string &name, bool &data) :
			EAPFAST_XMLElementParser (name),
			payload (data)
		{ }

		virtual int svc (DOMNode *n);

	protected:
		bool &payload;
};



#endif // __EAP_FAST_XML_PARSER_H__



