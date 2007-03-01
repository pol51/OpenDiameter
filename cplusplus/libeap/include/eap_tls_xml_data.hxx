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


#ifndef __EAP_TLS_XML_DATA_H__
#define __EAP_TLS_XML_DATA_H__

#include "eap_tls_data_defs.hxx"
#include "eap_tls_xml_parser.hxx"



class EAPTLS_XMLDataEncryption : public EAPTLS_XMLElementParser
{
	public:
		EAPTLS_XMLDataEncryption (std::string &name, EAPTLS_CfgDataEncryption &data) :
			EAPTLS_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPTLS_CfgDataEncryption &data)
		{ }	// TODO

	protected:
		EAPTLS_CfgDataEncryption &payload;
};


class EAPTLS_XMLDataFragmentation : public EAPTLS_XMLElementParser
{
	public:
		EAPTLS_XMLDataFragmentation (std::string &name, EAPTLS_CfgDataFragmentation &data) :
			EAPTLS_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPTLS_CfgDataFragmentation &data)
		{ }	// TODO

	protected:
		EAPTLS_CfgDataFragmentation &payload;
};


class EAPTLS_XMLDataServer : public EAPTLS_XMLElementParser
{
	public:
		EAPTLS_XMLDataServer (std::string &name, EAPTLS_CfgDataServer &data) :
			EAPTLS_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPTLS_CfgDataServer &data)
		{ }	// TODO

	protected:
		EAPTLS_CfgDataServer &payload;
};


class EAPTLS_XMLDataRootServer : public EAPTLS_XMLElementParser
{
	public:
		EAPTLS_XMLDataRootServer (std::string &name, EAPTLS_CfgDataRootServer &data) :
			EAPTLS_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPTLS_CfgDataRootServer &data)
		{ }	// TODO

//	protected:
		EAPTLS_CfgDataRootServer &payload;
};


class EAPTLS_XMLDataRootClient : public EAPTLS_XMLElementParser
{
	public:
		EAPTLS_XMLDataRootClient (std::string &name, EAPTLS_CfgDataRootClient &data) :
			EAPTLS_XMLElementParser (name),
			payload (data)
		{ }

		int svc (DOMNode *n);

		static void print (EAPTLS_CfgDataRootClient &data)
		{ }	// TODO

//	protected:
		EAPTLS_CfgDataRootClient &payload;
};



#endif // __EAP_TLS_XML_DATA_H__


