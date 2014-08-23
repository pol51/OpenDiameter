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

#ifndef __EAP_FAST_DATA_DEFS_H__
#define __EAP_FAST_DATA_DEFS_H__

#ifdef WIN32
#include "StdAfx.h"
#endif
#include "aaa_parser_api.h"
//#include "rbtree.h"
#include "ace/Singleton.h"

//
// Definition of default values
//
#define EAPFAST_CFG_XML_ROOT_SERVER                  "configuration_server"
#define EAPFAST_CFG_XML_ROOT_CLIENT                  "configuration_client"



//
// Configuration Data Structures
//
typedef ACE_UINT32 diameter_unsigned32_t;

typedef struct {
	std::string pass_phrase;
	std::string key_file;
	std::string cert_file;
	std::string random;
	std::string ca_path;
	std::string ca_cert;
	std::string pac_file;
	std::string credential_file;
	std::string pac_opaque_encr;
	std::string dh;
	diameter_unsigned32_t rsa_key;
	diameter_unsigned32_t dh_key;
	diameter_unsigned32_t rsa_key_length;
	diameter_unsigned32_t dh_key_length;
	diameter_unsigned32_t verify_depth;
	diameter_unsigned32_t file_type;
} EAPFAST_CfgDataEncryption;


typedef struct {
	bool include_length;
	diameter_unsigned32_t fragment_size;
} EAPFAST_CfgDataFragmentation;


typedef struct {
	std::string auth_id;
} EAPFAST_CfgDataServer;


typedef struct {
	EAPFAST_CfgDataEncryption encryption;
	EAPFAST_CfgDataFragmentation fragmentation;
	EAPFAST_CfgDataServer server;
} EAPFAST_CfgDataRootServer;


typedef struct {
	EAPFAST_CfgDataEncryption encryption;
	EAPFAST_CfgDataFragmentation fragmentation;
} EAPFAST_CfgDataRootClient;



typedef ACE_Singleton<EAPFAST_CfgDataRootServer, ACE_Null_Mutex> EAPFAST_CfgDataRootServer_S;
typedef ACE_Singleton<EAPFAST_CfgDataRootClient, ACE_Null_Mutex> EAPFAST_CfgDataRootClient_S;



#define EAPFAST_CFG_ROOT_SERVER()			EAPFAST_CfgDataRootServer_S::instance()
#define EAPFAST_CFG_ENCRYPTION_SERVER()		(&(EAPFAST_CfgDataRootServer_S::instance()->encryption))
#define EAPFAST_CFG_FRAGMENTATION_SERVER()	(&(EAPFAST_CfgDataRootServer_S::instance()->fragmentation))
#define EAPFAST_CFG_SERVER_SERVER()			(&(EAPFAST_CfgDataRootServer_S::instance()->server))
#define EAPFAST_CFG_ROOT_CLIENT()				EAPFAST_CfgDataRootClient_S::instance()
#define EAPFAST_CFG_ENCRYPTION_CLIENT()		(&(EAPFAST_CfgDataRootClient_S::instance()->encryption))
#define EAPFAST_CFG_FRAGMENTATION_CLIENT()	(&(EAPFAST_CfgDataRootClient_S::instance()->fragmentation))



#endif // __EAP_FAST_DATA_DEFS_H__


