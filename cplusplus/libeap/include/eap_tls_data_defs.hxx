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

#ifndef __EAP_TLS_DATA_DEFS_H__
#define __EAP_TLS_DATA_DEFS_H__

#ifdef WIN32
#include "StdAfx.h"
#endif
#include "aaa_parser_api.h"
//#include "rbtree.h"
#include "ace/Singleton.h"

//
// Definition of default values
//
#define EAPTLS_CFG_XML_ROOT_SERVER                  "configuration_server"
#define EAPTLS_CFG_XML_ROOT_CLIENT                  "configuration_client"



//
// Configuration Data Structures
//

typedef struct {
	std::string pass_phrase;
	std::string key_file;
	std::string cert_file;
	std::string random;
	std::string ca_path;
	std::string ca_cert;
	std::string dh;
	diameter_unsigned32_t rsa_key;
	diameter_unsigned32_t dh_key;
	diameter_unsigned32_t rsa_key_length;
	diameter_unsigned32_t dh_key_length;
	diameter_unsigned32_t verify_depth;
	diameter_unsigned32_t file_type;
} EAPTLS_CfgDataEncryption;


typedef struct {
	bool include_length;
	diameter_unsigned32_t fragment_size;
} EAPTLS_CfgDataFragmentation;


typedef struct {
	diameter_unsigned32_t id_context;
} EAPTLS_CfgDataServer;


typedef struct {
	EAPTLS_CfgDataEncryption encryption;
	EAPTLS_CfgDataFragmentation fragmentation;
	EAPTLS_CfgDataServer server;
} EAPTLS_CfgDataRootServer;


typedef struct {
	EAPTLS_CfgDataEncryption encryption;
	EAPTLS_CfgDataFragmentation fragmentation;
} EAPTLS_CfgDataRootClient;



typedef ACE_Singleton<EAPTLS_CfgDataRootServer, ACE_Null_Mutex> EAPTLS_CfgDataRootServer_S;
typedef ACE_Singleton<EAPTLS_CfgDataRootClient, ACE_Null_Mutex> EAPTLS_CfgDataRootClient_S;



#define EAPTLS_CFG_ROOT_SERVER()			EAPTLS_CfgDataRootServer_S::instance()
#define EAPTLS_CFG_ENCRYPTION_SERVER()		(&(EAPTLS_CfgDataRootServer_S::instance()->encryption))
#define EAPTLS_CFG_FRAGMENTATION_SERVER()	(&(EAPTLS_CfgDataRootServer_S::instance()->fragmentation))
#define EAPTLS_CFG_SERVER_SERVER()			(&(EAPTLS_CfgDataRootServer_S::instance()->server))
#define EAPTLS_CFG_ROOT_CLIENT()				EAPTLS_CfgDataRootClient_S::instance()
#define EAPTLS_CFG_ENCRYPTION_CLIENT()		(&(EAPTLS_CfgDataRootClient_S::instance()->encryption))
#define EAPTLS_CFG_FRAGMENTATION_CLIENT()	(&(EAPTLS_CfgDataRootClient_S::instance()->fragmentation))



#endif // __EAP_TLS_DATA_DEFS_H__


