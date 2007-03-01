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

#ifndef __AAA_DATA_DEFS_H__
#define __AAA_DATA_DEFS_H__

#ifdef WIN32
#include "StdAfx.h"
#endif
#include "diameter_parser.h"
#include "od_utl_rbtree.h"
#include "ace/Singleton.h"

//
// Windows specific export declarations
//
#if defined (WIN32)
#  if defined (DIAMETERBASEPROTOCOL_EXPORTS)
#    define DIAMETERBASEPROTOCOL_EXPORT ACE_Proper_Export_Flag
#    define DIAMETERBASEPROTOCOL_EXPORT_ONLY ACE_Proper_Export_Flag
#    define DIAMETERBASEPROTOCOL_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define DIAMETERBASEPROTOCOL_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else
#    define DIAMETERBASEPROTOCOL_EXPORT ACE_Proper_Import_Flag
#    define DIAMETERBASEPROTOCOL_EXPORT_ONLY
#    define DIAMETERBASEPROTOCOL_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define DIAMETERBASEPROTOCOL_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif   // DIAMETERBASEPROTOCOL_EXPORTS
#else
#  define DIAMETERBASEPROTOCOL_EXPORT
#  define DIAMETERBASEPROTOCOL_EXPORT_ONLY
#  define DIAMETERBASEPROTOCOL_SINGLETON_DECLARATION(T)
#  define DIAMETERBASEPROTOCOL_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif     // WIN32

//
// AVP names used internally
//
#define DIAMETER_AVPNAME_SESSIONID             "Session-Id"
#define DIAMETER_AVPNAME_RESULTCODE            "Result-Code"
#define DIAMETER_AVPNAME_ORIGINHOST            "Origin-Host"
#define DIAMETER_AVPNAME_ORIGINREALM           "Origin-Realm"
#define DIAMETER_AVPNAME_ORIGINSTATEID         "Origin-State-Id"
#define DIAMETER_AVPNAME_DESTHOST              "Destination-Host"
#define DIAMETER_AVPNAME_DESTREALM             "Destination-Realm"
#define DIAMETER_AVPNAME_AUTHSESSIONSTATE      "Auth-Session-State"
#define DIAMETER_AVPNAME_AUTHAPPID             "Auth-Application-Id"
#define DIAMETER_AVPNAME_ACCTAPPID             "Acct-Application-Id"
#define DIAMETER_AVPNAME_VENDORAPPID           "Vendor-Specific-Application-Id"
#define DIAMETER_AVPNAME_REAUTHREQTYPE         "Re-Auth-Request-Type"
#define DIAMETER_AVPNAME_TERMINATION           "Termination-Cause"
#define DIAMETER_AVPNAME_AUTHSESSIONSTATE      "Auth-Session-State"
#define DIAMETER_AVPNAME_ERRRORREPORTINGHOST   "Error-Reporting-Host"
#define DIAMETER_AVPNAME_AUTHLIFETIME          "Authorization-Lifetime"
#define DIAMETER_AVPNAME_AUTHGRACE             "Auth-Grace-Period"
#define DIAMETER_AVPNAME_SESSIONTIMEOUT        "Session-Timeout"
#define DIAMETER_AVPNAME_HOSTIP                "Host-IP-Address"
#define DIAMETER_AVPNAME_VENDORID              "Vendor-Id"
#define DIAMETER_AVPNAME_PRODUCTNAME           "Product-Name"
#define DIAMETER_AVPNAME_ROUTERECORD           "Route-Record"
#define DIAMETER_AVPNAME_REDIRECTHOST          "Redirect-Host"
#define DIAMETER_AVPNAME_REDIRECTHOSTUSAGE     "Redirect-Host-Usage"
#define DIAMETER_AVPNAME_USERNAME              "User-Name"
#define DIAMETER_AVPNAME_FIRMWAREREV           "Firmware-Revision"
#define DIAMETER_AVPNAME_INBANDSECID           "Inband-Security-Id"
#define DIAMETER_AVPNAME_SUPPORTEDVENDORID     "Supported-Vendor-Id"
#define DIAMETER_AVPNAME_ERRORMESSAGE          "Error-Message"
#define DIAMETER_AVPNAME_ERRORREPORTINGHOST    "Error-Reporting-Host"
#define DIAMETER_AVPNAME_DISCONNECT_CAUSE      "Disconnect-Cause"
#define DIAMETER_AVPNAME_ACCTREC_TYPE          "Accounting-Record-Type"
#define DIAMETER_AVPNAME_ACCTREC_NUM           "Accounting-Record-Number"
#define DIAMETER_AVPNAME_ACCTSUBSID            "Accounting-Sub-Session-Id"
#define DIAMETER_AVPNAME_ACCTREALTIME          "Accounting-Realtime-Required"
#define DIAMETER_AVPNAME_ACCTSID               "Acct-Session-Id"
#define DIAMETER_AVPNAME_ACCTMULTISID          "Acct-Multi-Session-Id"
#define DIAMETER_AVPNAME_ACCTINTERVAL          "Acct-Interim-Interval"
#define DIAMETER_AVPNAME_CLASS                 "Class"
#define DIAMETER_AVPNAME_PROXYINFO             "Proxy-Info"
#define DIAMETER_AVPNAME_PROXYHOST             "Proxy-Host"
#define DIAMETER_AVPNAME_PROXYSTATE            "Proxy-State"
#define DIAMETER_AVPNAME_WILDCARD              "AVP"

//
// Command Codes used internally
//
#define DIAMETER_MSGCODE_ABORTSESSION          274
#define DIAMETER_MSGCODE_SESSIONTERMINATION    275
#define DIAMETER_MSGCODE_CAPABILITIES_EXCHG    257
#define DIAMETER_MSGCODE_WATCHDOG              280
#define DIAMETER_MSGCODE_DISCONNECT_PEER       282
#define DIAMETER_MSGCODE_ACCOUNTING            271
#define DIAMETER_MSGCODE_REAUTH                258

//
// Application Identifiers
//
#define DIAMETER_RELAY_APPLICATION_ID          0xffffffff

//
// Session state values
//
#define DIAMETER_SESSION_STATE_MAINTAINED        0
#define DIAMETER_SESSION_NO_STATE_MAINTAINED     1

//
// Re-auth-type values
//
#define DIAMETER_SESSION_AUTHORIZE_ONLY          0
#define DIAMETER_SESSION_AUTHORIZE_AUTHENTICATE  1

//
// Accounting record types
//
typedef enum {
   DIAMETER_ACCT_RECTYPE_EVENT =                 1,
   DIAMETER_ACCT_RECTYPE_START =                 2,
   DIAMETER_ACCT_RECTYPE_INTERIM =               3,
   DIAMETER_ACCT_RECTYPE_STOP =                  4
} DIAMETER_ACCT_RECTYPE;

//
// Accouting realtime required values
//
typedef enum {
   DIAMETER_ACCT_REALTIME_DELIVER_AND_GRANT =     1,
   DIAMETER_ACCT_REALTIME_GRANT_AND_STORE =       2,
   DIAMETER_ACCT_REALTIME_GRANT_AND_LOSE =        3
} DIAMETER_ACCT_REALTIME;

//
// General timer type's
typedef enum {
   DIAMETER_TIMER_TYPE_ASR =                      1,
   DIAMETER_TIMER_TYPE_SESSION =                  2,
   DIAMETER_TIMER_TYPE_AUTH =                     3,
   DIAMETER_TIMER_TYPE_INTERVAL =                 4
} DIAMETER_TIMER_TYPE;

//
// Misc definitions
//
#define AAA_AUTH_SESSION_GRACE_PERIOD             5
#define AAA_AUTH_SESSION_RECLAMATION_PERIOD       60

//
// Configuration Data Structures
//
typedef std::list<diameter_unsigned32_t> DiameterApplicationIdLst;

typedef struct {
   diameter_unsigned32_t vendorId;     // vendor application id 
   diameter_unsigned32_t authAppId;      // auth application id
   diameter_unsigned32_t acctAppId;      // acct application id
} DiameterDataVendorSpecificApplicationId;

typedef std::list<DiameterDataVendorSpecificApplicationId> DiameterVendorSpecificIdLst;

typedef struct {
   std::string product;                  // readable string product name
   diameter_unsigned32_t version;        // current version
   diameter_unsigned32_t vendor;         // local vendor id
   diameter_unsigned32_t threadCount;    // number of threads to use 
   DiameterApplicationIdLst supportedVendorIdLst; // supported vendor application id
   DiameterApplicationIdLst authAppIdLst;    // auth application id
   DiameterApplicationIdLst acctAppIdLst;    // acct application id
   DiameterVendorSpecificIdLst vendorSpecificId; // vendor specific app id 
} DiameterDataGeneral;

typedef struct {
   std::string dictionary;               // filename of parser dictionary
} DiameterDataParser;

typedef struct {
   std::string hostname;                 // hostname of peer
   diameter_unsigned32_t port;           // port number
   diameter_unsigned32_t use_sctp;       // Use SCTP for this connection
   diameter_unsigned32_t tls_enabled;    // TLS support
} DiameterDataPeer;

typedef struct {
   std::string identity;                 // local hostname
   std::string realm;                    // local realm
   diameter_unsigned32_t tcp_listen_port; // TCP listening port
   diameter_unsigned32_t sctp_listen_port; // SCTP listening port
   diameter_unsigned32_t use_ipv6;       // TLS listening port
   diameter_unsigned32_t watchdog_timeout; // Watchdog timeout
   diameter_unsigned32_t reconnect_interval; // ReConnect interval
   diameter_unsigned32_t reconnect_max; // ReConnect max
   diameter_unsigned32_t retx_interval;  // Req ReTx interval
   diameter_unsigned32_t retx_max_count; // Req ReTx max count
   diameter_unsigned32_t rx_buffer_size; // Receive Buffer Size
   std::list<std::string> advertised_hostname; // List of hostnames used by this node
                                         // addresses provided by this peer
} DiameterDataTransportMngt;

typedef struct {
   diameter_unsigned32_t stateful;       // stateful/stateless session flags
   diameter_unsigned32_t sessionTm;      // session timer
   diameter_unsigned32_t lifetimeTm;     // lifetime timer
   diameter_unsigned32_t graceTm;        // grace period timer
   diameter_unsigned32_t abortRetryTm;   // abort retry timer
} DiameterDataAuthSession;

typedef struct {
   diameter_unsigned32_t sessionTm;      // lifetime timer
   diameter_unsigned32_t recIntervalTm;  // record interval
   diameter_unsigned32_t realtime;       // realtime required value
} DiameterDataAcctSession;

typedef struct {
   diameter_unsigned32_t maxSessions;    // maximum number of sessions allowed
   DiameterDataAuthSession authSessions;     // auth session config
   DiameterDataAcctSession acctSessions;     // acct session config
} DiameterDataSessionMngt;

typedef struct {
   diameter_integer32_t debug:1;         // debug flag
   diameter_integer32_t trace:1;         // trace flag
   diameter_integer32_t info:1;          // info flag
   diameter_integer32_t reserved:29;     // un-used
} DiameterDataLogFlags;

typedef struct {
   diameter_integer32_t console:1;         // enable|disable stdout target
   diameter_integer32_t syslog:1;        // enable|disable syslog target
   diameter_integer32_t reserved:30;     // un-used
} DiameterDataLogTragetFlags;

typedef struct {
   DiameterDataLogFlags flags;             // log flags
   DiameterDataLogTragetFlags targets;     // targets
} DiameterDataLog;

typedef struct {
   diameter_integer32_t originStateId; // runtime origin state
} DiameterRunTime;

typedef struct {
   DiameterRunTime runtime;                // runtime configuration
   DiameterDataGeneral general;            // general configuration
   DiameterDataParser parser;              // parser configuration
   DiameterDataTransportMngt transport;    // transport configuration
   DiameterDataSessionMngt session;        // session configuration
   DiameterDataLog log;                    // logging configuration
} DiameterDataRoot;

typedef struct {
   diameter_unsigned32_t reAuthType;   // re-authentication type
   diameter_unsigned32_t resultCode;   // result code
} DiameterReAuthValue;

typedef ACE_Singleton<DiameterDataRoot, ACE_Thread_Mutex> DiameterDataRoot_S;

#define DIAMETER_CFG_ROOT()            (DiameterDataRoot_S::instance())
#define DIAMETER_CFG_RUNTIME()         (&(DiameterDataRoot_S::instance()->runtime))
#define DIAMETER_CFG_GENERAL()         (&(DiameterDataRoot_S::instance()->general))
#define DIAMETER_CFG_PARSER()          (&(DiameterDataRoot_S::instance()->parser))
#define DIAMETER_CFG_TRANSPORT()       (&(DiameterDataRoot_S::instance()->transport))
#define DIAMETER_CFG_SESSION()         (&(DiameterDataRoot_S::instance()->session))
#define DIAMETER_CFG_AUTH_SESSION()    (&(DiameterDataRoot_S::instance()->session.authSessions))
#define DIAMETER_CFG_ACCT_SESSION()    (&(DiameterDataRoot_S::instance()->session.acctSessions))
#define DIAMETER_CFG_LOG()             (&(DiameterDataRoot_S::instance()->log))

class DiameterBaseException
{
   public:
      typedef enum {
          ALLOC_FAILURE = 0,
          INVALID_ID_TYPE,
          MISSING_SESSION_ID,
          MISSING_ORIGIN_HOST,
          MISSING_ORIGIN_REALM,
          IO_FAILURE
      } ERROR_CODE;

   public:
      DiameterBaseException(int code, std::string &desc) :
        m_Code(code), m_Description(desc) {
      }
      DiameterBaseException(int code, const char* desc) :
        m_Code(code), m_Description(desc) {
      }
      int &Code() {
          return m_Code;
      }
      std::string &Description() {
          return m_Description;
      }

   private:
      int m_Code;
      std::string m_Description;
};

class DiameterMsgQuery
{
    public:
       DiameterMsgQuery(DiameterMsg &msg) :
           m_Msg(msg) {
       }
       bool IsRequest() {
           return m_Msg.hdr.flags.r;
       }
       bool IsAnswer() {
           return ! IsRequest();
       }
       bool IsCapabilities() {
           return (m_Msg.hdr.code == DIAMETER_MSGCODE_CAPABILITIES_EXCHG);
       }
       bool IsReAuth() {
           return (m_Msg.hdr.code == DIAMETER_MSGCODE_REAUTH);
       }
       bool IsWatchDog() {
           return (m_Msg.hdr.code == DIAMETER_MSGCODE_WATCHDOG);
       }
       bool IsPeerDisconnect() {
           return (m_Msg.hdr.code == DIAMETER_MSGCODE_DISCONNECT_PEER);
       }
       diameter_unsigned32_t Code() {
           return m_Msg.hdr.code;
       }
       bool IsBaseProtocol() {
           static diameter_unsigned32_t msgBaseProto[] = {
               DIAMETER_MSGCODE_ABORTSESSION,
               DIAMETER_MSGCODE_SESSIONTERMINATION,
               DIAMETER_MSGCODE_CAPABILITIES_EXCHG,
               DIAMETER_MSGCODE_WATCHDOG,
               DIAMETER_MSGCODE_DISCONNECT_PEER,
               DIAMETER_MSGCODE_ACCOUNTING,
               DIAMETER_MSGCODE_REAUTH
           };
           for (unsigned int i=0;
                i<sizeof(msgBaseProto)/sizeof(diameter_unsigned32_t);
                i++) {
               if (msgBaseProto[i] == m_Msg.hdr.code) {
                   return true;
               }
           }
           return false;
       }

    private:
       DiameterMsg &m_Msg;
};

class DiameterErrorMsg
{
    /*

    7.2.  Error Bit

       The 'E' (Error Bit) in the Diameter header is set when the request
       caused a protocol-related error (see Section 7.1.3).  A message with
       the 'E' bit MUST NOT be sent as a response to an answer message.
       Note that a message with the 'E' bit set is still subjected to the
       processing rules defined in Section 6.2.  When set, the answer
       message will not conform to the ABNF specification for the command,
       and will instead conform to the following ABNF:

       Message Format

       <answer-message> ::= < Diameter Header: code, ERR [PXY] >
                         0*1< Session-Id >
                            { Origin-Host }
                            { Origin-Realm }
                            { Result-Code }
                            [ Origin-State-Id ]
                            [ Error-Reporting-Host ]
                            [ Proxy-Info ]
                          * [ AVP ]

       Note that the code used in the header is the same than the one found
       in the request message, but with the 'R' bit cleared and the 'E' bit
       set.  The 'P' bit in the header is set to the same value as the one
       found in the request message.
    */

    public:
       static std::auto_ptr<DiameterMsg> Generate(DiameterMsg &request,
                                                  diameter_unsigned32_t rcode) {

          DiameterMsgWidget errAnswer(request.hdr.code, false,
                                      request.hdr.appId);
          errAnswer()->hdr = request.hdr;
          errAnswer()->hdr.flags.e = DIAMETER_FLAG_SET;
          errAnswer()->hdr.flags.r = DIAMETER_FLAG_CLR;

          DiameterUtf8AvpContainerWidget sidReqAvp(request.acl);
          DiameterUInt32AvpContainerWidget originStateReqAvp(request.acl);
          DiameterGroupedAvpContainerWidget proxyInfoReqAvp(request.acl);

          DiameterIdentityAvpWidget originHostAvp(DIAMETER_AVPNAME_ORIGINHOST);
          DiameterIdentityAvpWidget originRealmAvp(DIAMETER_AVPNAME_ORIGINREALM);
          DiameterUInt32AvpWidget resultCodeAvp(DIAMETER_AVPNAME_RESULTCODE);
          DiameterUInt32AvpWidget originStateAvp(DIAMETER_AVPNAME_ORIGINSTATEID);
          DiameterIdentityAvpWidget errorHostAvp(DIAMETER_AVPNAME_ERRORREPORTINGHOST);
          DiameterGroupedAvpWidget proxyInfoAvp(DIAMETER_AVPNAME_PROXYINFO);

          DiameterUtf8AvpContainerWidget sidAvp(errAnswer()->acl);
          diameter_utf8string_t *sidReq = sidReqAvp.GetAvp(DIAMETER_AVPNAME_SESSIONID);
          if (sidReq) {
             diameter_utf8string_t &sid = sidAvp.AddAvp(DIAMETER_AVPNAME_SESSIONID);
             sid = *sidReq;
          }

          diameter_unsigned32_t *oStateId = originStateReqAvp.GetAvp(DIAMETER_AVPNAME_ORIGINSTATEID);
          if (oStateId) {
              originStateAvp.Get() = *oStateId;
              errAnswer()->acl.add(originStateAvp());
          }

          diameter_grouped_t *proxyInfo = proxyInfoReqAvp.GetAvp(DIAMETER_AVPNAME_PROXYINFO);
          if (proxyInfo) {
              DiameterGroupedAvpContainerWidget pInfoAvp(errAnswer()->acl);
              diameter_grouped_t &pInfoAns = pInfoAvp.AddAvp(DIAMETER_AVPNAME_PROXYINFO);

              DiameterIdentityAvpContainerWidget pHostReq(*proxyInfo);
              DiameterStringAvpContainerWidget pStateReq(*proxyInfo);

              DiameterIdentityAvpContainerWidget pHostAns(pInfoAns);
              DiameterStringAvpContainerWidget pStateAns(pInfoAns);

              diameter_identity_t *hostReq = pHostReq.GetAvp(DIAMETER_AVPNAME_PROXYHOST);
              diameter_identity_t &hostAns = pHostAns.AddAvp(DIAMETER_AVPNAME_PROXYHOST);
              hostAns = *hostReq;

              diameter_identity_t *stateReq = pStateReq.GetAvp(DIAMETER_AVPNAME_PROXYSTATE);
              diameter_identity_t &stateAns = pStateAns.AddAvp(DIAMETER_AVPNAME_PROXYSTATE);
              stateAns = *stateReq;
          }

          originHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
          originRealmAvp.Get() = DIAMETER_CFG_TRANSPORT()->realm;
          errorHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
          resultCodeAvp.Get() = rcode;

          errAnswer()->acl.add(originHostAvp());
          errAnswer()->acl.add(originRealmAvp());
          errAnswer()->acl.add(resultCodeAvp());
          errAnswer()->acl.add(errorHostAvp());

          return errAnswer();
       }

       static std::auto_ptr<DiameterMsg> Generate(DiameterMsgHeader hdr,
                                                  diameter_unsigned32_t rcode) {

          DiameterMsgWidget errAnswer(hdr.code, false, hdr.appId);
          errAnswer()->hdr = hdr;
          errAnswer()->hdr.flags.e = DIAMETER_FLAG_SET;
          errAnswer()->hdr.flags.r = DIAMETER_FLAG_CLR;

          DiameterIdentityAvpWidget originHostAvp(DIAMETER_AVPNAME_ORIGINHOST);
          DiameterIdentityAvpWidget originRealmAvp(DIAMETER_AVPNAME_ORIGINREALM);
          DiameterUInt32AvpWidget resultCodeAvp(DIAMETER_AVPNAME_RESULTCODE);
          DiameterIdentityAvpWidget errorHostAvp(DIAMETER_AVPNAME_ERRORREPORTINGHOST);

          originHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
          originRealmAvp.Get() = DIAMETER_CFG_TRANSPORT()->realm;
          errorHostAvp.Get() = DIAMETER_CFG_TRANSPORT()->identity;
          resultCodeAvp.Get() = rcode;

          errAnswer()->acl.add(originHostAvp());
          errAnswer()->acl.add(originRealmAvp());
          errAnswer()->acl.add(resultCodeAvp());
          errAnswer()->acl.add(errorHostAvp());

          return errAnswer();
       }
};

#endif // __AAA_DEFS_H__


