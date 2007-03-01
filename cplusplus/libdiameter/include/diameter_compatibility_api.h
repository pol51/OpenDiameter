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

#ifndef __DIAMETER_COMPATIBILITY_API_H__
#define __DIAMETER_COMPATIBILITY_API_H__

///////////////////////////////////////////////////////////////////////////////////
// Backward compatibility Section
//   * Includes typedef's to previous class names in version 1.0.7-g and below
//   * Misc backward compatibility definition
///////////////////////////////////////////////////////////////////////////////////

//
// Old AVP name macros mapped to current macros
//
#define AAA_AVPNAME_SESSIONID             DIAMETER_AVPNAME_SESSIONID
#define AAA_AVPNAME_RESULTCODE            DIAMETER_AVPNAME_RESULTCODE
#define AAA_AVPNAME_ORIGINHOST            DIAMETER_AVPNAME_ORIGINHOST
#define AAA_AVPNAME_ORIGINREALM           DIAMETER_AVPNAME_ORIGINREALM
#define AAA_AVPNAME_ORIGINSTATEID         DIAMETER_AVPNAME_ORIGINSTATEID
#define AAA_AVPNAME_DESTHOST              DIAMETER_AVPNAME_DESTHOST
#define AAA_AVPNAME_DESTREALM             DIAMETER_AVPNAME_DESTREALM
#define AAA_AVPNAME_AUTHSESSIONSTATE      DIAMETER_AVPNAME_AUTHSESSIONSTATE
#define AAA_AVPNAME_AUTHAPPID             DIAMETER_AVPNAME_AUTHAPPID
#define AAA_AVPNAME_ACCTAPPID             DIAMETER_AVPNAME_ACCTAPPID
#define AAA_AVPNAME_VENDORAPPID           DIAMETER_AVPNAME_VENDORAPPID
#define AAA_AVPNAME_REAUTHREQTYPE         DIAMETER_AVPNAME_REAUTHREQTYPE
#define AAA_AVPNAME_TERMINATION           DIAMETER_AVPNAME_TERMINATION
#define AAA_AVPNAME_AUTHSESSIONSTATE      DIAMETER_AVPNAME_AUTHSESSIONSTATE
#define AAA_AVPNAME_ERRRORREPORTINGHOST   DIAMETER_AVPNAME_ERRRORREPORTINGHOST
#define AAA_AVPNAME_AUTHLIFETIME          DIAMETER_AVPNAME_AUTHLIFETIME
#define AAA_AVPNAME_AUTHGRACE             DIAMETER_AVPNAME_AUTHGRACE
#define AAA_AVPNAME_SESSIONTIMEOUT        DIAMETER_AVPNAME_SESSIONTIMEOUT
#define AAA_AVPNAME_HOSTIP                DIAMETER_AVPNAME_HOSTIP
#define AAA_AVPNAME_VENDORID              DIAMETER_AVPNAME_VENDORID
#define AAA_AVPNAME_PRODUCTNAME           DIAMETER_AVPNAME_PRODUCTNAME
#define AAA_AVPNAME_ROUTERECORD           DIAMETER_AVPNAME_ROUTERECORD
#define AAA_AVPNAME_REDIRECTHOST          DIAMETER_AVPNAME_REDIRECTHOST
#define AAA_AVPNAME_REDIRECTHOSTUSAGE     DIAMETER_AVPNAME_REDIRECTHOSTUSAGE
#define AAA_AVPNAME_USERNAME              DIAMETER_AVPNAME_USERNAME
#define AAA_AVPNAME_FIRMWAREREV           DIAMETER_AVPNAME_FIRMWAREREV
#define AAA_AVPNAME_INBANDSECID           DIAMETER_AVPNAME_INBANDSECID
#define AAA_AVPNAME_SUPPORTEDVENDORID     DIAMETER_AVPNAME_SUPPORTEDVENDORID
#define AAA_AVPNAME_ERRORMESSAGE          DIAMETER_AVPNAME_ERRORMESSAGE
#define AAA_AVPNAME_ERRORREPORTINGHOST    DIAMETER_AVPNAME_ERRORREPORTINGHOST
#define AAA_AVPNAME_DISCONNECT_CAUSE      DIAMETER_AVPNAME_DISCONNECT_CAUSE
#define AAA_AVPNAME_ACCTREC_TYPE          DIAMETER_AVPNAME_ACCTREC_TYPE
#define AAA_AVPNAME_ACCTREC_NUM           DIAMETER_AVPNAME_ACCTREC_NUM
#define AAA_AVPNAME_ACCTSUBSID            DIAMETER_AVPNAME_ACCTSUBSID
#define AAA_AVPNAME_ACCTREALTIME          DIAMETER_AVPNAME_ACCTREALTIME
#define AAA_AVPNAME_ACCTSID               DIAMETER_AVPNAME_ACCTSID
#define AAA_AVPNAME_ACCTMULTISID          DIAMETER_AVPNAME_ACCTMULTISID
#define AAA_AVPNAME_ACCTINTERVAL          DIAMETER_AVPNAME_ACCTINTERVAL
#define AAA_AVPNAME_CLASS                 DIAMETER_AVPNAME_CLASS
#define AAA_AVPNAME_WILDCARD              DIAMETER_AVPNAME_WILDCARD

//
// Old session state values mapped to current macros
//
#define AAA_SESSION_STATE_MAINTAINED      DIAMETER_SESSION_STATE_MAINTAINED
#define AAA_SESSION_NO_STATE_MAINTAINED   DIAMETER_SESSION_NO_STATE_MAINTAINED

//
// Old config macros mapped to current macros
//
#define AAA_CFG_ROOT                      DIAMETER_CFG_ROOT
#define AAA_CFG_GENERAL                   DIAMETER_CFG_GENERAL
#define AAA_CFG_PARSER                    DIAMETER_CFG_PARSER
#define AAA_CFG_TRANSPORT                 DIAMETER_CFG_TRANSPORT
#define AAA_CFG_SESSION                   DIAMETER_CFG_SESSION
#define AAA_CFG_AUTH_SESSION              DIAMETER_CFG_AUTH_SESSION
#define AAA_CFG_ACCT_SESSION              DIAMETER_CFG_ACCT_SESSION
#define AAA_CFG_LOG                       DIAMETER_CFG_LOG

//
// Old Accounting record types mapped to current types
//
typedef enum {
   AAA_ACCT_RECTYPE_EVENT        =        DIAMETER_ACCT_RECTYPE_EVENT,
   AAA_ACCT_RECTYPE_START        =        DIAMETER_ACCT_RECTYPE_START,
   AAA_ACCT_RECTYPE_INTERIM      =        DIAMETER_ACCT_RECTYPE_INTERIM,
   AAA_ACCT_RECTYPE_STOP         =        DIAMETER_ACCT_RECTYPE_STOP
} AAA_ACCT_RECTYPE;

//
// Old Accounting realtime required values mapped to current values
//
typedef enum {
   AAA_ACCT_REALTIME_DELIVER_AND_GRANT =  DIAMETER_ACCT_REALTIME_DELIVER_AND_GRANT,
   AAA_ACCT_REALTIME_GRANT_AND_STORE   =  DIAMETER_ACCT_REALTIME_GRANT_AND_STORE,
   AAA_ACCT_REALTIME_GRANT_AND_LOSE    =  DIAMETER_ACCT_REALTIME_GRANT_AND_LOSE
} AAA_ACCT_REALTIME;

//
// The following macro definitions are mapped to template classes.
// This is the simpliest approach that will allow older apps to
// keep the class names from previous versions withour worrying
// about template namespacing.
//

// Generic session level API
#define AAA_SessionMsgMuxHandler          DiameterSessionMsgMuxHandler
#define AAA_SessionMsgMux                 DiameterSessionMsgMux
#define AAA_ServerSessionAllocator        DiameterServerSessionAllocator

// Accounting session level API
#define AAA_ServerAcctSession                  DiameterServerAcctSession
#define AAA_ClientAcctSubSession               DiameterClientAcctSubSession
#define AAA_ServerAcctRecStorageWithConverter  DiameterServerAcctRecStorageWithConverter

//
// The following are typedefs are mappings between old classnames
// and new classnames that are used by application programs.
//

// Application API
typedef DiameterApplication                     AAA_Application;

// Peer management API
typedef DiameterPeer                            AAA_Peer;
typedef DiameterPeerEntry                       AAA_PeerEntry;
typedef DiameterPeerManager                     AAA_PeerManager;
typedef DiameterPeerEventInterface              AAA_PeerFsmUserEventInterface;

// Accounting record collection API
typedef DiameterClientAcctRecCollector          AAA_ClientAcctRecCollector;
typedef DiameterServerAcctRecStorage            AAA_ServerAcctRecStorage;
typedef DiameterAccountingRecTransformer        AAAAccountingRecTransformer;
typedef DiameterAccountingXMLRecTransformer     AAAAccountingXMLRecTransformer;

// Generic session level API
typedef DiameterSessionId                       AAA_SessionId;
typedef DiameterServerSessionFactory            AAA_ServerSessionFactory;

// Accounting session API
typedef DiameterAcctSession                     AAA_AcctSession;
typedef DiameterClientAcctSession               AAA_ClientAcctSession;

// Auth session API
typedef DiameterAuthSession                     AAA_AuthSession;
typedef DiameterClientAuthSession               AAA_ClientAuthSession;
typedef DiameterServerAuthSession               AAA_ServerAuthSession;

#endif   // __DIAMETER_COMPATIBILITY_API_H__


