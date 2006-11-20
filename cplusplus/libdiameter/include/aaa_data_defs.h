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
#define AAA_AUTH_SESSION_GRACE_PERIOD        5

//
// Configuration Data Structures
//
typedef std::list<diameter_unsigned32_t> DiameterApplicationIdLst;

typedef struct {
   DiameterApplicationIdLst vendorIdLst;     // vendor application id 
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

template <class ARG>
class DiameterProtectedQueue :
   private std::list<ARG>
{
   public:
      virtual ~DiameterProtectedQueue() {
      }
      virtual void Enqueue(ARG arg) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         std::list<ARG>::push_back(arg);  
      }
      virtual ARG Dequeue() {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         ARG a = std::list<ARG>::front();
         std::list<ARG>::pop_front();
         return a;
      }
      virtual bool IsEmpty() {
         ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
         return std::list<ARG>::empty() ? true : false;
      }

   private:
      ACE_RW_Mutex m_Lock;
};

template <class ARG> 
class DiameterIterAction 
{
   public:
      // return TRUE to delete entry in iteration
      // return FALSE to sustain entry
      virtual bool operator()(ARG&)=0;
      
   protected:
      virtual ~DiameterIterAction() {
      }
      DiameterIterAction() {
      }
};

template <class INDEX, 
          class DATA>
class DiameterProtectedMap : 
   private std::map<INDEX, DATA>
{
   public:
      virtual ~DiameterProtectedMap() {
      }
      virtual void Add(INDEX ndx, DATA data) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         std::map<INDEX, DATA>::insert(std::pair
             <INDEX, DATA>(ndx, data));
      }
      virtual bool Lookup(INDEX ndx, DATA &data) {
         ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
         typename std::map<INDEX, DATA>::iterator i;
         i = std::map<INDEX, DATA>::find(ndx);
         if (i != std::map<INDEX, DATA>::end()) {
             data = i->second;
             return (true);
         }
         return (false);
       }
      virtual bool Remove(INDEX ndx,
                          DiameterIterAction<DATA> &e) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         typename std::map<INDEX, DATA>::iterator i;
         i = std::map<INDEX, DATA>::find(ndx);
         if (i != std::map<INDEX, DATA>::end()) {
             e(i->second);
             std::map<INDEX, DATA>::erase(i);
             return (true);
         }
         return (false);
      }
      virtual bool IsEmpty() {
         ACE_Read_Guard<ACE_RW_Mutex> guard(m_Lock);
         return std::map<INDEX, DATA>::empty() ? true : false;
      }
      virtual void Iterate(DiameterIterAction<DATA> &e) {
         ACE_Write_Guard<ACE_RW_Mutex> guard(m_Lock);
         typename std::map<INDEX, DATA>::iterator i =
             std::map<INDEX, DATA>::begin();
         while (i != std::map<INDEX, DATA>::end()) {
             if (e(i->second)) {
                 typename std::map<INDEX, DATA>::iterator h = i;
                 i ++;
                 std::map<INDEX, DATA>::erase(h);
                 continue;
             }
             i ++;
         }
      }

   private:
      ACE_RW_Mutex m_Lock;
};

template <class ARG>
class DiameterProtectedPtrQueue 
{
   public:
      void Enqueue(std::auto_ptr<ARG> a) {
         m_Queue.Enqueue(a.release());
      }
      std::auto_ptr<ARG> Dequeue() {
         std::auto_ptr<ARG> arg(m_Queue.Dequeue());
         return arg;
      }

   private:
      DiameterProtectedQueue<ARG*> m_Queue;
};

template <class ARG>
class DiameterProtectedPtrMap
{
   public:
      void Enqueue(std::auto_ptr<ARG> a) {
         m_Queue.Enqueue(a.release());
      }
      std::auto_ptr<ARG> Dequeue() {
         std::auto_ptr<ARG> arg(m_Queue.Dequeue());
         return arg;
      }

   private:
      DiameterProtectedQueue<ARG*> m_Queue;
};

class DiameterRangedValue
{
   public:
      typedef enum {
          DEFAULT_LOW  = 0,
          DEFAULT_HIGH = 3,
      };

   public:
      DiameterRangedValue(int level = DEFAULT_LOW,
                      int low = DEFAULT_LOW, 
                      int high = DEFAULT_HIGH) {
          Reset(level, low, high);
      }      
      virtual ~DiameterRangedValue() {
      }
      virtual int operator++() {
          m_CurrentLevel += 1;
          return (m_CurrentLevel > m_HighThreshold) ? true : false;
      }
      virtual int operator--() {
          m_CurrentLevel -= 1;
          return (m_CurrentLevel < m_LowThreshold) ? true : false;
      }
      virtual int operator()() {
          return m_CurrentLevel;
      }
      virtual bool InRange() {
          return ((m_CurrentLevel > m_LowThreshold) &&
                  (m_CurrentLevel < m_HighThreshold));
      }
      void Reset(int level = DEFAULT_LOW,
                 int low = DEFAULT_LOW, 
                 int high = DEFAULT_HIGH) {
          m_CurrentLevel = level;
          m_LowThreshold = low;
          m_HighThreshold = high;
      } 

   private:
      int m_CurrentLevel;
      int m_LowThreshold;
      int m_HighThreshold;
};

#endif // __AAA_DEFS_H__


