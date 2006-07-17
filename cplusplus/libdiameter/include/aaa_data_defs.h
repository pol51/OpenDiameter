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
#include "diameter_parser_api.h"
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
// Definition of default values
//
#define AAA_CFG_FILENAME                  "diameter.conf"
#define AAA_CFG_XML_ROOT                  "configuration"

//
// AVP names used internally
//
#define AAA_AVPNAME_SESSIONID             "Session-Id"
#define AAA_AVPNAME_RESULTCODE            "Result-Code"
#define AAA_AVPNAME_ORIGINHOST            "Origin-Host"
#define AAA_AVPNAME_ORIGINREALM           "Origin-Realm"
#define AAA_AVPNAME_ORIGINSTATEID         "Origin-State-Id"
#define AAA_AVPNAME_DESTHOST              "Destination-Host"
#define AAA_AVPNAME_DESTREALM             "Destination-Realm"
#define AAA_AVPNAME_AUTHSESSIONSTATE      "Auth-Session-State"
#define AAA_AVPNAME_AUTHAPPID             "Auth-Application-Id"
#define AAA_AVPNAME_ACCTAPPID             "Acct-Application-Id"
#define AAA_AVPNAME_VENDORAPPID           "Vendor-Specific-Application-Id"
#define AAA_AVPNAME_REAUTHREQTYPE         "Re-Auth-Request-Type"
#define AAA_AVPNAME_TERMINATION           "Termination-Cause"
#define AAA_AVPNAME_AUTHSESSIONSTATE      "Auth-Session-State"
#define AAA_AVPNAME_ERRRORREPORTINGHOST   "Error-Reporting-Host"
#define AAA_AVPNAME_AUTHLIFETIME          "Authorization-Lifetime"
#define AAA_AVPNAME_AUTHGRACE             "Auth-Grace-Period"
#define AAA_AVPNAME_SESSIONTIMEOUT        "Session-Timeout"
#define AAA_AVPNAME_HOSTIP                "Host-IP-Address"
#define AAA_AVPNAME_VENDORID              "Vendor-Id"
#define AAA_AVPNAME_PRODUCTNAME           "Product-Name"
#define AAA_AVPNAME_ROUTERECORD           "Route-Record"
#define AAA_AVPNAME_REDIRECTHOST          "Redirect-Host"
#define AAA_AVPNAME_REDIRECTHOSTUSAGE     "Redirect-Host-Usage"
#define AAA_AVPNAME_USERNAME              "User-Name"
#define AAA_AVPNAME_FIRMWAREREV           "Firmware-Revision"
#define AAA_AVPNAME_INBANDSECID           "Inband-Security-Id"
#define AAA_AVPNAME_SUPPORTEDVENDORID     "Supported-Vendor-Id"
#define AAA_AVPNAME_ERRORMESSAGE          "Error-Message"
#define AAA_AVPNAME_ERRORREPORTINGHOST    "Error-Reporting-Host"
#define AAA_AVPNAME_DISCONNECT_CAUSE      "Disconnect-Cause"
#define AAA_AVPNAME_ACCTREC_TYPE          "Accounting-Record-Type"
#define AAA_AVPNAME_ACCTREC_NUM           "Accounting-Record-Number"
#define AAA_AVPNAME_ACCTSUBSID            "Accounting-Sub-Session-Id"
#define AAA_AVPNAME_ACCTREALTIME          "Accounting-Realtime-Required"
#define AAA_AVPNAME_ACCTSID               "Acct-Session-Id"
#define AAA_AVPNAME_ACCTMULTISID          "Acct-Multi-Session-Id"
#define AAA_AVPNAME_ACCTINTERVAL          "Acct-Interim-Interval"
#define AAA_AVPNAME_CLASS                 "Class"
#define AAA_AVPNAME_WILDCARD              "AVP"

//
// Command Codes used internally
//
#define AAA_MSGCODE_ABORTSESSION          274
#define AAA_MSGCODE_SESSIONTERMINATION    275
#define AAA_MSGCODE_CAPABILITIES_EXCHG    257
#define AAA_MSGCODE_WATCHDOG              280
#define AAA_MSGCODE_DISCONNECT_PEER       282
#define AAA_MSGCODE_ACCOUNTING            271
#define AAA_MSGCODE_REAUTH                258

//
// Transport module definitions
//
#define AAA_MAX_INTERFACES                10
#define AAA_MAX_MSGLEN                    2048
#define AAA_WATCHDOG_VERBOSE              0
#define AAA_WATCHDOG_INTERVAL             30 // 30 sec defined in 3.4.1 of draft
#define AAA_WATCHDOG_COUNT_ONSTART        2
#define AAA_TRANSPORT_STATE_TIMEOUT       50
#define AAA_DEFAULT_ROUTE_ENTRY           "***"

//
// Application Identifiers
//
#define AAA_RELAY_APPLICATION_ID          0xffffffff

//
// Session state values
//
#define AAA_SESSION_STATE_MAINTAINED        0
#define AAA_SESSION_NO_STATE_MAINTAINED     1

//
// Re-auth-type values
//
#define AAA_SESSION_AUTHORIZE_ONLY          0
#define AAA_SESSION_AUTHORIZE_AUTHENTICATE  1

//
// Accounting record types
//
typedef enum {
   AAA_ACCT_RECTYPE_EVENT =                 1,
   AAA_ACCT_RECTYPE_START =                 2,
   AAA_ACCT_RECTYPE_INTERIM =               3,
   AAA_ACCT_RECTYPE_STOP =                  4
} AAA_ACCT_RECTYPE;

//
// Accouting realtime required values
//
typedef enum {
   AAA_ACCT_REALTIME_DELIVER_AND_GRANT =     1,
   AAA_ACCT_REALTIME_GRANT_AND_STORE =       2,
   AAA_ACCT_REALTIME_GRANT_AND_LOSE =        3
} AAA_ACCT_REALTIME;

//
// General timer type's
typedef enum { 
   AAA_TIMER_TYPE_ASR =                      1,
   AAA_TIMER_TYPE_SESSION =                  2,
   AAA_TIMER_TYPE_AUTH =                     3,
   AAA_TIMER_TYPE_INTERVAL =                 4
} AAA_TIMER_TYPE;       

//
// Misc definitions
//
#define AAA_AUTH_SESSION_GRACE_PERIOD        5

//
// Configuration Data Structures
//
typedef std::list<diameter_unsigned32_t> AAA_ApplicationIdLst;

typedef struct {
   AAA_ApplicationIdLst vendorIdLst;     // vendor application id 
   diameter_unsigned32_t authAppId;      // auth application id
   diameter_unsigned32_t acctAppId;      // acct application id
} AAA_DataVendorSpecificApplicationId;

typedef std::list<AAA_DataVendorSpecificApplicationId> AAA_VendorSpecificIdLst;

typedef struct {
   std::string product;                  // readable string product name
   diameter_unsigned32_t version;        // current version
   diameter_unsigned32_t vendor;         // local vendor id
   diameter_unsigned32_t threadCount;    // number of threads to use 
   AAA_ApplicationIdLst supportedVendorIdLst; // supported vendor application id
   AAA_ApplicationIdLst authAppIdLst;    // auth application id
   AAA_ApplicationIdLst acctAppIdLst;    // acct application id
   AAA_VendorSpecificIdLst vendorSpecificId; // vendor specific app id 
} AAA_DataGeneral;

typedef struct {
   std::string dictionary;               // filename of parser dictionary
} AAA_DataParser;

typedef struct {
   std::string hostname;                 // hostname of peer
   diameter_unsigned32_t port;           // port number
   diameter_unsigned32_t tls_enabled;    // TLS support
} AAA_DataPeer;

typedef struct {
   std::string identity;                 // local hostname
   std::string realm;                    // local realm
   diameter_unsigned32_t tcp_port;       // TCP listening port
   diameter_unsigned32_t tls_port;       // TLS listening port
   diameter_unsigned32_t watchdog_timeout; // Watchdog timeout
   diameter_unsigned32_t retry_interval; // Retry interval
   diameter_unsigned32_t retx_interval;  // Req ReTx interval
   diameter_unsigned32_t retx_max_count; // Req ReTx max count
   std::list<std::string> advertised_host_ip; // List of host ip
                                         // addresses provided by this peer
} AAA_DataTransportMngt;

typedef struct {
   diameter_unsigned32_t stateful;       // stateful/stateless session flags
   diameter_unsigned32_t sessionTm;      // session timer
   diameter_unsigned32_t lifetimeTm;     // lifetime timer
   diameter_unsigned32_t graceTm;        // grace period timer
   diameter_unsigned32_t abortRetryTm;   // abort retry timer
} AAA_DataAuthSession;

typedef struct {
   diameter_unsigned32_t sessionTm;      // lifetime timer
   diameter_unsigned32_t recIntervalTm;  // record interval
   diameter_unsigned32_t realtime;       // realtime required value
} AAA_DataAcctSession;

typedef struct {
   diameter_unsigned32_t maxSessions;    // maximum number of sessions allowed
   AAA_DataAuthSession authSessions;     // auth session config
   AAA_DataAcctSession acctSessions;     // acct session config
} AAA_DataSessionMngt;

typedef struct {
   diameter_integer32_t debug:1;         // debug flag
   diameter_integer32_t trace:1;         // trace flag
   diameter_integer32_t info:1;          // info flag
   diameter_integer32_t reserved:29;     // un-used
} AAA_DataLogFlags;

typedef struct {
   diameter_integer32_t console:1;         // enable|disable stdout target
   diameter_integer32_t syslog:1;        // enable|disable syslog target
   diameter_integer32_t reserved:30;     // un-used
} AAA_DataLogTragetFlags;

typedef struct {
   AAA_DataLogFlags flags;             // log flags
   AAA_DataLogTragetFlags targets;     // targets
} AAA_DataLog;

typedef struct {
   diameter_integer32_t originStateId; // runtime origin state
} AAA_RunTime;

typedef struct {
   AAA_RunTime runtime;                // runtime configuration
   AAA_DataGeneral general;            // general configuration
   AAA_DataParser parser;              // parser configuration
   AAA_DataTransportMngt transport;    // transport configuration
   AAA_DataSessionMngt session;        // session configuration
   AAA_DataLog log;                    // logging configuration
} AAA_DataRoot;

typedef struct {
   diameter_unsigned32_t reAuthType;   // re-authentication type
   diameter_unsigned32_t resultCode;   // result code
} AAA_ReAuthValue;

typedef ACE_Singleton<AAA_DataRoot, ACE_Thread_Mutex> AAA_DataRoot_S;
#define AAA_CFG_ROOT()            (AAA_DataRoot_S::instance())
#define AAA_CFG_RUNTIME()         (&(AAA_DataRoot_S::instance()->runtime))
#define AAA_CFG_GENERAL()         (&(AAA_DataRoot_S::instance()->general))
#define AAA_CFG_PARSER()          (&(AAA_DataRoot_S::instance()->parser))
#define AAA_CFG_TRANSPORT()       (&(AAA_DataRoot_S::instance()->transport))
#define AAA_CFG_SESSION()         (&(AAA_DataRoot_S::instance()->session))
#define AAA_CFG_AUTH_SESSION()    (&(AAA_DataRoot_S::instance()->session.authSessions))
#define AAA_CFG_ACCT_SESSION()    (&(AAA_DataRoot_S::instance()->session.acctSessions))
#define AAA_CFG_LOG()             (&(AAA_DataRoot_S::instance()->log))

class AAA_BaseException
{
   public:
      typedef enum {
          ALLOC_FAILURE = 0,
          INVALID_ID_TYPE,
          MISSING_SESSION_ID,
      } ERROR_CODE;
    
   public:
      AAA_BaseException(int code, std::string &desc) :
        m_Code(code), m_Description(desc) {
      }
      AAA_BaseException(int code, const char* desc) :
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

class AAA_MsgQuery
{
    public:
       AAA_MsgQuery(AAAMessage &msg) :
           m_Msg(msg) {
       }
       bool IsRequest() {
           return m_Msg.hdr.flags.r;
       }
       bool IsAnswer() {
           return ! IsRequest();
       }
       bool IsCapabilities() {
           return (m_Msg.hdr.code == AAA_MSGCODE_CAPABILITIES_EXCHG);
       }
       bool IsReAuth() {
           return (m_Msg.hdr.code == AAA_MSGCODE_REAUTH);
       }
       bool IsWatchDog() {
           return (m_Msg.hdr.code == AAA_MSGCODE_WATCHDOG);
       }
       bool IsPeerDisconnect() {
           return (m_Msg.hdr.code == AAA_MSGCODE_DISCONNECT_PEER);
       }
       diameter_unsigned32_t Code() {
           return m_Msg.hdr.code;
       }
       bool IsBaseProtocol() {
           static diameter_unsigned32_t msgBaseProto[] = {
               AAA_MSGCODE_ABORTSESSION,
               AAA_MSGCODE_SESSIONTERMINATION,
               AAA_MSGCODE_CAPABILITIES_EXCHG,
               AAA_MSGCODE_WATCHDOG,
               AAA_MSGCODE_DISCONNECT_PEER,
               AAA_MSGCODE_ACCOUNTING,
               AAA_MSGCODE_REAUTH
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
       AAAMessage &m_Msg;
};

template <class ARG>
class AAA_ProtectedQueue : 
   private std::list<ARG>
{
   public:
      virtual ~AAA_ProtectedQueue() {
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
class AAA_IterAction 
{
   public:
      // return TRUE to delete entry in iteration
      // return FALSE to sustain entry
      virtual bool operator()(ARG&)=0;
      
   protected:
      virtual ~AAA_IterAction() {
      }
      AAA_IterAction() {
      }
};

template <class INDEX, 
          class DATA>
class AAA_ProtectedMap : 
   private std::map<INDEX, DATA>
{
   public:
      virtual ~AAA_ProtectedMap() {
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
                          AAA_IterAction<DATA> &e) {
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
      virtual void Iterate(AAA_IterAction<DATA> &e) {
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
class AAA_ProtectedPtrQueue 
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
      AAA_ProtectedQueue<ARG*> m_Queue;
};

template <class ARG>
class AAA_ProtectedPtrMap
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
      AAA_ProtectedQueue<ARG*> m_Queue;
};

class AAA_RangedValue
{
   public:
      typedef enum {
          DEFAULT_LOW  = 0,
          DEFAULT_HIGH = 3,
      };

   public:
      AAA_RangedValue(int level = DEFAULT_LOW,
                      int low = DEFAULT_LOW, 
                      int high = DEFAULT_HIGH) {
          Reset(level, low, high);
      }      
      virtual ~AAA_RangedValue() {
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


