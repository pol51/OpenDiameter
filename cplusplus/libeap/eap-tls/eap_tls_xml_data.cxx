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



#include "eap_tls_xml_data.hxx"



int EAPTLS_XMLDataEncryption::svc (DOMNode *n)
{
	std::string tagName;

	tagName = "pass_phrase";
	EAPTLS_XMLDataString pass_phrase (tagName, payload.pass_phrase);
	pass_phrase.populate (n->getFirstChild ());

	tagName = "key_file";
	EAPTLS_XMLDataString key_file (tagName, payload.key_file);
	key_file.populate (n->getFirstChild ());

	tagName = "cert_file";
	EAPTLS_XMLDataString cert_file (tagName, payload.cert_file);
	cert_file.populate (n->getFirstChild ());

	tagName = "random";
	EAPTLS_XMLDataString random (tagName, payload.random);
	random.populate (n->getFirstChild ());

	tagName = "ca_path";
	EAPTLS_XMLDataString ca_path (tagName, payload.ca_path);
	ca_path.populate (n->getFirstChild ());

	tagName = "ca_cert";
	EAPTLS_XMLDataString ca_cert (tagName, payload.ca_cert);
	ca_cert.populate (n->getFirstChild ());

	tagName = "dh";
	EAPTLS_XMLDataString dh (tagName, payload.dh);
	dh.populate (n->getFirstChild ());

	tagName = "rsa_key";
	EAPTLS_XMLDataUInt32 rsa_key (tagName, payload.rsa_key);
	rsa_key.populate (n->getFirstChild ());

	tagName = "dh_key";
	EAPTLS_XMLDataUInt32 dh_key (tagName, payload.dh_key);
	dh_key.populate (n->getFirstChild ());

	tagName = "rsa_key_length";
	EAPTLS_XMLDataUInt32 rsa_key_length (tagName, payload.rsa_key_length);
	rsa_key_length.populate (n->getFirstChild ());

	tagName = "dh_key_length";
	EAPTLS_XMLDataUInt32 dh_key_length (tagName, payload.dh_key_length);
	dh_key_length.populate (n->getFirstChild ());

	tagName = "verify_depth";
	EAPTLS_XMLDataUInt32 verify_depth (tagName, payload.verify_depth);
	verify_depth.populate (n->getFirstChild ());

	tagName = "file_type";
	EAPTLS_XMLDataUInt32 file_type (tagName, payload.file_type);
	file_type.populate (n->getFirstChild ());

	return (0);
}


int EAPTLS_XMLDataFragmentation::svc (DOMNode *n)
{
	std::string tagName;

	tagName = "include_length";
	EAPTLS_XMLDataBoolean include_length (tagName, payload.include_length);
	include_length.populate (n->getFirstChild ());

	tagName = "fragment_size";
	EAPTLS_XMLDataUInt32 fragment_size (tagName, payload.fragment_size);
	fragment_size.populate (n->getFirstChild ());

	return (0);
}


int EAPTLS_XMLDataServer::svc (DOMNode *n)
{
	std::string tagName;

	tagName = "id_context";
	EAPTLS_XMLDataUInt32 id_context (tagName, payload.id_context);
	id_context.populate (n->getFirstChild ());

	return (0);
}


int EAPTLS_XMLDataRootServer::svc (DOMNode *n)
{
	std::string tagName;

	tagName = "encryption";
	EAPTLS_XMLDataEncryption encryption (tagName, payload.encryption);
	encryption.populate (n->getFirstChild ());

	tagName = "fragmentation";
	EAPTLS_XMLDataFragmentation fragmentation (tagName, payload.fragmentation);
	fragmentation.populate (n->getFirstChild ());

	tagName = "server";
	EAPTLS_XMLDataServer server (tagName, payload.server);
	server.populate (n->getFirstChild ());

	return (0);
}


int EAPTLS_XMLDataRootClient::svc (DOMNode *n)
{
	std::string tagName;

	tagName = "encryption";
	EAPTLS_XMLDataEncryption encryption (tagName, payload.encryption);
	encryption.populate (n->getFirstChild ());

	tagName = "fragmentation";
	EAPTLS_XMLDataFragmentation fragmentation (tagName, payload.fragmentation);
	fragmentation.populate (n->getFirstChild ());

	return (0);
}
