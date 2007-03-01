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


#ifndef __PANA_SESSION_H__
#define __PANA_SESSION_H__

#include <string>
#include "ace/Atomic_Op.h"
#include "pana_exports.h"
#include "pana_exceptions.h"
#include "pana_security_assoc.h"
#include "pana_config_manager.h"
#include "pana_serial_num.h"
#include "pana_pmk_bootstrap.h"

typedef PANA_SimpleQueue<PANA_Message*> PANA_MsgQueue;
typedef PANA_SimpleQueue<AAAMessageBlock*> PANA_BufferQueue;

class PANA_AuxillarySessionVariables {
    public:
        PANA_AuxillarySessionVariables() {
            Reset();
        }
        virtual ~PANA_AuxillarySessionVariables() {
            Reset();
        }
        bool &Authorized() {
            return m_Authorized;
        }
        bool &AlgorithmIsSet() {
            return m_AlgorithmIsSet;
        }
        PANA_MsgQueue &RxMsgQueue() {
            return m_RxMessageQueue;
        }
        PANA_BufferQueue &TxEapMessageQueue() {
            return m_TxEapMessageQueue;
        }
        virtual void Reset();

    private:
        bool m_Authorized; // Set to true if authorize() returns true
        bool m_AlgorithmIsSet; // Set to true if algorithm avp is agreed upon
        PANA_MsgQueue m_RxMessageQueue; // Receive message queue
        PANA_BufferQueue m_TxEapMessageQueue; // EAP Tx message queue
};

class PANA_SessionAttribute {

    public:
        PANA_SessionAttribute() {
           Reset();
       }
       virtual ~PANA_SessionAttribute() {
           Reset();
       }
       ACE_UINT32 &SessionId() {
           return m_SessionId;
       }
       ACE_INET_Addr &PacAddress() {
           return m_PacAddress;
       }
       ACE_INET_Addr &PaaAddress() {
           return m_PaaAddress;
       }
       PANA_SerialNumber &LastTxSeqNum() {
           return m_LastTxSeqNum;
       }
       PANA_SerialNumber &LastRxSeqNum() {
           return m_LastRxSeqNum;
       }
       boost::shared_ptr<PANA_Message> &LastTxReqMsg() {
           return m_LastTxReqMsg;
       }
       boost::shared_ptr<PANA_Message> &CachedAnsMsg() {
           return m_CachedAnsMsg;
       }
       ACE_UINT32 &SessionLifetime() {
           return m_SessionLifetime;
       }
       PANA_MsgHeader &LastRxHeader() {
           return m_LastRxHeader;
       }
       virtual void Reset();

    private:
       ACE_UINT32 m_SessionId;  // Session Id
       ACE_INET_Addr m_PacAddress;     // PaC IP address and port
       ACE_INET_Addr m_PaaAddress;     // PAA IP address and port
       PANA_SerialNumber m_LastTxSeqNum; // last transmitted seq number
       PANA_SerialNumber m_LastRxSeqNum; // last recevied tseq number value
       boost::shared_ptr<PANA_Message> m_LastTxReqMsg; // last transmitted message
       boost::shared_ptr<PANA_Message> m_CachedAnsMsg; // cached message
       ACE_UINT32 m_SessionLifetime; // negotiated session lifetime
       PANA_MsgHeader m_LastRxHeader; // Header of the last received message
};

typedef enum {
  PANA_TID_RETRY,
  PANA_TID_SESSION,
  PANA_TID_EAP_RESP,
} PANA_TID;

template<class ARG>
class PANA_EXPORT PANA_SessionRxInterface
{
   public:
      PANA_SessionRxInterface(ARG &arg) : m_arg(arg) { }
      virtual ~PANA_SessionRxInterface() { }
      virtual void operator()(PANA_Message &msg) = 0;
      virtual PANA_SessionRxInterface<ARG> *clone() = 0;
   protected:
      ARG &m_arg;
};

template<class ARG>
class PANA_EXPORT PANA_SessionRxInterfaceTable
{
   public:
      virtual ~PANA_SessionRxInterfaceTable() { }

      virtual void Register(int mtype, PANA_SessionRxInterface<ARG> &i) {
         m_map.insert(std::pair<int, PANA_SessionRxInterface<ARG>*>
                      (mtype, i.clone()));
      }
      virtual void Remove(int type) { 
         typename std::map<int, PANA_SessionRxInterface<ARG>*>::iterator i =
             m_map.find(type); 
         if (i != m_map.end()) {
            delete static_cast<PANA_SessionRxInterface<ARG>*>
                (i->second);
            m_map.erase(i);
         }
      }
      void Receive(PANA_Message &msg) {
         typename std::map<int, PANA_SessionRxInterface<ARG>*>::iterator i =
             m_map.find(msg.type());
         if (i != m_map.end()) {
            PANA_SessionRxInterface<ARG> *h = i->second;
            try {
               (*h)(msg);
            }
            catch (PANA_Exception &e) {
               delete &msg;
               AAA_LOG((LM_INFO, "(%P|%t) ERROR: code = %d, desc = %s\n",
                        e.code(), e.description().data()));
            }
            catch (...) {
               delete &msg;
               AAA_LOG((LM_INFO, "(%P|%t) ERROR: Unknown exception\n"));
            }
         }
         else {
            AAA_LOG((LM_INFO, "(%P|%t) ERROR: No message handler for msg: %d, discarding\n",
                       msg.type()));
         }
      }

   protected:
      std::map<int, PANA_SessionRxInterface<ARG>*> m_map;
};

class PANA_EXPORT PANA_SessionTxInterface
{
   public:
      PANA_SessionTxInterface() { }
      virtual ~PANA_SessionTxInterface() { }
      virtual void Send(boost::shared_ptr<PANA_Message> msg) = 0;
};

class PANA_SessionTimerInterface
{
    public:
        PANA_SessionTimerInterface() :
            m_Timeout(0),
            m_Duration(0),
            m_Count(0) {
        }
        virtual ~PANA_SessionTimerInterface() { }

        // for general timer use
        virtual void Schedule(PANA_TID id, ACE_UINT32 sec) = 0;
        virtual void Cancel(PANA_TID id) = 0;

        // for session timeout
        bool ScheduleSession(ACE_UINT32 sec) {
            Schedule(PANA_TID_SESSION, sec);
            return (true);
        }
        void CancelSession() {
            Cancel(PANA_TID_SESSION);
        }

        // for eap response timer only
        bool ScheduleEapResponse() {
            Schedule(PANA_TID_EAP_RESP,
                     PANA_CFG_PAC().m_EapResponseTimeout);
            return true;
        }
        void CancelEapResponse() {
            Cancel(PANA_TID_EAP_RESP);
        }

        // for message re-transmission only
        bool ScheduleTxRetry();
        bool ReScheduleTxRetry();
        void CancelTxRetry() {
            Cancel(PANA_TID_RETRY);
        }

    private:
        inline float RAND() {
            return float(-0.1 + 0.2*float(ACE_OS::rand()/(RAND_MAX+1.0)));
        }
        unsigned int m_Timeout;
        unsigned int m_Duration;
        unsigned int m_Count;
};

class PANA_EXPORT PANA_SessionEventInterface
{
   public:
      typedef struct {
         AAAScholarAttribute<ACE_INET_Addr> m_PacAddress;
         AAAScholarAttribute<ACE_INET_Addr> m_PaaAddress;
         AAAScholarAttribute<pana_octetstring_t> m_Key;
         AAAScholarAttribute<ACE_UINT32> m_KeyId;
         AAAScholarAttribute<ACE_UINT32> m_Lifetime;
      } PANA_AuthorizationArgs;

   public:
      virtual void EapStart() = 0;
      virtual void Authorize(PANA_AuthorizationArgs &args) = 0;
      virtual bool IsKeyAvailable(pana_octetstring_t &key) = 0;
      virtual void Disconnect(ACE_UINT32 cause = 0) = 0;
      virtual void EapAltReject() = 0;
      virtual void Error(ACE_UINT32 resultCode) = 0;
      virtual ~PANA_SessionEventInterface() { }
};

class PANA_EXPORT PANA_Session :
   public PANA_SessionAttribute
{
   public:
      virtual void NotifyScheduleLifetime();

      virtual bool IsFatalError();

      virtual void TxPPR();
      virtual void TxPPA();
      virtual void TxPUR();
      virtual void TxPUA();
      virtual void TxPTR(ACE_UINT32 cause);
      virtual void TxPTA();
      virtual void TxPER(pana_unsigned32_t rcode);
      virtual void TxPEA();

      virtual void RxPPR();
      virtual void RxPPA();
      virtual void RxPUR();
      virtual void RxPUA();
      virtual void RxPTR();
      virtual void RxPTA();
      virtual void RxPER();
      virtual void RxPEA();

      virtual void SendReqMsg(boost::shared_ptr<PANA_Message> msg,
                              bool allowRetry = true);
      virtual void SendAnsMsg(boost::shared_ptr<PANA_Message> msg);

      virtual void TxPrepareMessage(PANA_Message &msg);
      virtual void RxValidateMsg(PANA_Message &msg,
                                 bool skipAuth = false);

      virtual bool TxLastReqMsg();
      virtual bool TxLastAnsMsg();

      virtual void Disconnect(ACE_UINT32 cause = 0);
      virtual void Error(ACE_UINT32 resultCode = 0);
      virtual void Reset();

      PANA_AuxillarySessionVariables &AuxVariables() {
          return m_AuxVariables;
      }
      PANA_SecurityAssociation &SecurityAssociation() {
          return m_SA;
      }

   protected:
      PANA_Session(PANA_SessionTxInterface &tp,
                   PANA_SessionTimerInterface &tm,
                   PANA_SessionEventInterface &ev) :
         m_TxChannel(tp),
         m_Timer(tm),
         m_Event(ev) {
      }
      virtual ~PANA_Session() {
      }

      // session variables
      PANA_SecurityAssociation m_SA;
      PANA_AuxillarySessionVariables m_AuxVariables;

      // interface reference
      PANA_SessionTxInterface &m_TxChannel;
      PANA_SessionTimerInterface &m_Timer;
      PANA_SessionEventInterface &m_Event;
};

#endif /* __PANA_SESSION_H__ */
