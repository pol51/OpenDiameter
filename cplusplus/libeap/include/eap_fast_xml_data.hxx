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


#ifndef __EAP_FAST_XML_DATA_H__
#define __EAP_FAST_XML_DATA_H__

#include "eap_fast_data_defs.hxx"
#include "eap_fast_xml_parser.hxx"



class EAPFAST_XMLDataEncryption : public EAPFAST_XMLElementParser
{
	public:
		EAPFAST_XMLDataEncryption (std::string &name, EAPFAST_CfgDataEncryption &data) :
			EAPFAST_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPFAST_CfgDataEncryption &data)
		{ }	// TODO

	protected:
		EAPFAST_CfgDataEncryption &payload;
};


class EAPFAST_XMLDataFragmentation : public EAPFAST_XMLElementParser
{
	public:
		EAPFAST_XMLDataFragmentation (std::string &name, EAPFAST_CfgDataFragmentation &data) :
			EAPFAST_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPFAST_CfgDataFragmentation &data)
		{ }	// TODO

	protected:
		EAPFAST_CfgDataFragmentation &payload;
};


class EAPFAST_XMLDataServer : public EAPFAST_XMLElementParser
{
	public:
		EAPFAST_XMLDataServer (std::string &name, EAPFAST_CfgDataServer &data) :
			EAPFAST_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPFAST_CfgDataServer &data)
		{ }	// TODO

	protected:
		EAPFAST_CfgDataServer &payload;
};


class EAPFAST_XMLDataRootServer : public EAPFAST_XMLElementParser
{
	public:
		EAPFAST_XMLDataRootServer (std::string &name, EAPFAST_CfgDataRootServer &data) :
			EAPFAST_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPFAST_CfgDataRootServer &data)
		{ }	// TODO

//	protected:
		EAPFAST_CfgDataRootServer &payload;
};


class EAPFAST_XMLDataRootClient : public EAPFAST_XMLElementParser
{
	public:
		EAPFAST_XMLDataRootClient (std::string &name, EAPFAST_CfgDataRootClient &data) :
			EAPFAST_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPFAST_CfgDataRootClient &data)
		{ }	// TODO

//	protected:
		EAPFAST_CfgDataRootClient &payload;
};



#endif // __EAP_FAST_XML_DATA_H__


