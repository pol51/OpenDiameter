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

#ifndef __DIAMETER_API_H__
#define __DIAMETER_API_H__

/*!
 * Windows specific export declarations
 */
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
#  endif   /* ! DIAMETERBASEPROTOCOL_EXPORTS */
#else
#  define DIAMETERBASEPROTOCOL_EXPORT
#  define DIAMETERBASEPROTOCOL_EXPORT_ONLY
#  define DIAMETERBASEPROTOCOL_SINGLETON_DECLARATION(T)
#  define DIAMETERBASEPROTOCOL_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif     /* WIN32 */

#include "aaa_application.h"
#include "aaa_session_client.h"
#include "aaa_session_server.h"
#include "aaa_session_server_factory.h"
#include "aaa_session_acct_rec_transformer.h"
#include "diameter_parser_api.h"
#include "diameter_compatibility_api.h"

///////////////////////////////////////////////////////////////////////////////////
/////////////      !!!!            WARNING              !!!!       ////////////////
/////////////      THE FOLLOWING ARE OLD COMPATIBILITY API's       ////////////////
/////////////     NEW APPLICATIONS SHOULD NOT USE THESE API's      ////////////////
///////////////////////////////////////////////////////////////////////////////////


typedef DiameterServerSessionFactory AAAServerSessionFactory;

typedef std::auto_ptr<DiameterApplication> AAAApplicationHandle;

typedef DiameterSessionPayload AAASessionPayload;

class DIAMETERBASEPROTOCOL_EXPORT AAAApplicationCore 
{
    public:
        AAAApplicationCore() { 
        }
        AAAApplicationCore(char *configFileName, 
                           AAA_Task &task) {
            Open(configFileName, task);
        }
        virtual ~AAAApplicationCore() {
            Close();
        }
        AAAReturnCode Open(char *configFileName, 
                           AAA_Task &task) {
	    if (! m_Handle.get()) {
	        m_Handle = std::auto_ptr<DiameterApplication>
                              (new DiameterApplication(task));
                return m_Handle->Open(configFileName);
	    }
            return (AAA_ERR_SUCCESS);
	}
        AAAReturnCode Close() {
            if (m_Handle.get()) {
	        m_Handle->Close();
                m_Handle.reset();
	    }
            return (AAA_ERR_SUCCESS);
	}
        AAAReturnCode RegisterServerSessionFactory
	  (AAAServerSessionFactory *factory) {
            if (m_Handle.get()) {
	        return m_Handle->RegisterServerSessionFactory
                             (*factory);
            }
            return (AAA_ERR_SUCCESS);
	}
        AAAReturnCode RemoveServerSessionFactory
	  (AAAServerSessionFactory *factory) {
            if (m_Handle.get()) {
	        return m_Handle->RemoveServerSessionFactory
                             (factory->GetApplicationId());
            }
            return (AAA_ERR_SUCCESS);
	}
        AAAApplicationHandle &GetAppHandle() {
	    return m_Handle;
        }
        diameter_unsigned32_t GetNumActivePeerConnections() {
            if (m_Handle.get()) {
	        return m_Handle->NumActivePeers();
	    }
            return (0);
	}
        AAA_Task& GetTask() {
	    // warning - this will throw if m_Handle is invalid
	    return m_Handle->Task();
	}

    private:
        std::auto_ptr<DiameterApplication> m_Handle;
};

template<class SESSION_SERVER>
class AAAServerSessionClassFactory : 
    public AAAServerSessionFactory
{
    public:
       AAAServerSessionClassFactory(AAAApplicationCore &c,
                                    diameter_unsigned32_t appId) : 
	  AAAServerSessionFactory(c.GetTask(), appId),
          m_Core(c) {
       }
       DiameterSessionIO *CreateInstance() {        
           SESSION_SERVER *s = new SESSION_SERVER
              (m_Core, GetApplicationId());
           return s->IO();
       }

   private:
       AAAApplicationCore &m_Core;
};

class DIAMETERBASEPROTOCOL_EXPORT AAAPeerManager : 
    public DiameterPeerManager
{
    public:
        AAAPeerManager(AAAApplicationCore &core) :
            DiameterPeerManager(core.GetTask()) {
	}
};

class DIAMETERBASEPROTOCOL_EXPORT AAAEventHandler 
{
    public:
        AAAEventHandler(AAAApplicationCore &c) : 
             m_Core(c) { 
        }
        virtual ~AAAEventHandler() { 
        }
        virtual AAAReturnCode HandleMessage(DiameterMsg &msg) {
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode HandleDisconnect() {
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode HandleTimeout() {
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode HandleSessionTimeout() {
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode HandleAuthLifetimeTimeout() {
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode HandleAuthGracePeriodTimeout() {
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode HandleAbort() {
            return (AAA_ERR_SUCCESS);
        }
        const AAAApplicationCore &GetApplicationCore() {
            return m_Core;
        }

    protected:
        AAAApplicationCore &m_Core;
};

class DIAMETERBASEPROTOCOL_EXPORT AAASessionMessageHandler : 
   public AAAEventHandler 
{
   public:
      AAASessionMessageHandler(AAAApplicationCore &appCore,
                               AAACommandCode cmdCode) :
	 AAAEventHandler(appCore),
         m_Code(cmdCode) {
      }
      AAACommandCode GetCommandCode() { 
         return m_Code; 
      }

   private:
      AAACommandCode m_Code;
};

class DIAMETERBASEPROTOCOL_EXPORT AAASession : 
   public AAAEventHandler 
{
   public:
       typedef std::map<AAACommandCode,
                        AAASessionMessageHandler*>
                        AAAMessageMap;
       typedef std::pair<AAACommandCode,
                         AAASessionMessageHandler*>
                         AAAMessageMapPair;
       typedef enum {
           EVENT_AUTH_REQUEST = 0, /**< Application indicates pending authorization request */
           EVENT_AUTH_SUCCESS,     /**< Application indicates authorization successful */
           EVENT_AUTH_CONTINUE,    /**< Application indicates multi-round exchange */
           EVENT_AUTH_FAILED,      /**< Application indicates authorization successful */
           EVENT_ACCT_SUCCESS,     /**< Application indicates accounting request is successful */
           EVENT_NO_SERVICE,       /**< Application indicates service not provided */
           EVENT_PROC_ERROR        /**< Application indicates processing error */
       } EVENT;

    public:
        AAASession(AAAApplicationCore &appCore, 
                   diameter_unsigned32_t appId) :
	   AAAEventHandler(appCore),
           m_Id(appId),
	   m_LastEvent(EVENT_AUTH_SUCCESS),
	   m_IO(NULL) {
	}
        virtual ~AAASession() {
	}
        virtual AAAReturnCode SetTimeout(time_t timeout) {
            return (AAA_ERR_SUCCESS);
	}
        virtual AAAReturnCode RegisterMessageHandler
	     (AAASessionMessageHandler *handler) {
	    AAAMessageMap::iterator i = m_MsgMap.find
                    (handler->GetCommandCode());
	    if (m_MsgMap.end() != i) {
	        return (AAA_ERR_FAILURE);
	    }
            m_MsgMap.insert(AAAMessageMapPair
		           (handler->GetCommandCode(), handler));
            return (AAA_ERR_SUCCESS);
	}
        virtual AAAReturnCode RemoveMessageHandler
  	      (AAASessionMessageHandler *handler) {
	    AAAMessageMap::iterator i = m_MsgMap.find
                    (handler->GetCommandCode());
	    if (m_MsgMap.end() != i) {
	        m_MsgMap.erase(i);
	        return (AAA_ERR_SUCCESS);
	    }
            return (AAA_ERR_FAILURE);
	}
        virtual AAAReturnCode Update(AAASession::EVENT event) {
	    m_LastEvent = event;
            return (AAA_ERR_SUCCESS);           
	}
	diameter_unsigned32_t GetApplicationId() { 
            return m_Id; 
        }
	virtual void GetSessionId(diameter_octetstring_t &id) {
	}
        DiameterSessionIO *IO() {
	    return m_IO;
	}

    protected:   
        virtual AAAReturnCode CallMsgHandler(DiameterMsg &msg) {
	    AAAMessageMap::iterator i = m_MsgMap.find
                    (msg.hdr.code);
	    if (m_MsgMap.end() != i) {
	        return i->second->HandleMessage(msg);
	    }
            return (AAA_ERR_FAILURE);
	}

    protected:
	AAAMessageMap m_MsgMap;
        diameter_unsigned32_t m_Id;
        EVENT m_LastEvent;
        DiameterSessionIO *m_IO;
};

class DIAMETERBASEPROTOCOL_EXPORT AAAClientSession : 
    public AAASession,
    public DiameterClientAuthSession

{
    public:
        AAAClientSession(AAAApplicationCore &appCore,
                         diameter_unsigned32_t id) :
	    AAASession(appCore, id),
	    DiameterClientAuthSession(appCore.GetTask(), id) {
	    m_IO = this;
	}
        virtual ~AAAClientSession() {
	    DiameterClientAuthSession::End();
	}        
        virtual AAAReturnCode SetTimeout(time_t timeout) {
	    DIAMETER_CFG_AUTH_SESSION()->lifetimeTm = (diameter_unsigned32_t)timeout;
            return (AAA_ERR_SUCCESS);
	}
	virtual void GetSessionId(diameter_octetstring_t &id) {
	    Attributes().SessionId().Dump(id);
	}
        AAAReturnCode Start() {
	    return Begin();
	}
        AAAReturnCode SetOptionalSIDValue(std::string optVal) {
	    Attributes().SessionId().OptionalValue() = optVal.data();
            return (AAA_ERR_SUCCESS);
	}
        AAAReturnCode End() {
	    return DiameterClientAuthSession::End();
	}
    private:
        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
	    return CallMsgHandler(msg);
        }
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {
	    return CheckUpdateEvent(msg);
	}
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
	    return CallMsgHandler(msg);
	}
        virtual AAAReturnCode Disconnect() {
            return this->HandleDisconnect();
        }
        virtual AAAReturnCode ReClaimSession() {
            return this->HandleDisconnect();
        }
        virtual AAAReturnCode SessionTimeout() {
	    this->HandleTimeout();
	    return this->HandleSessionTimeout();
        }
        virtual AAAReturnCode AuthorizationTimeout() {
	    this->HandleAuthGracePeriodTimeout();
	    return this->HandleAuthLifetimeTimeout();
        }
        virtual AAAReturnCode AbortSession() {
	    return this->HandleAbort();
        }
        AAAReturnCode CheckUpdateEvent(DiameterMsg &msg) {
	    CallMsgHandler(msg);
	    switch (m_LastEvent) {
	    case EVENT_AUTH_SUCCESS:  return AAA_ERR_SUCCESS;
            case EVENT_AUTH_CONTINUE: return AAA_ERR_INCOMPLETE;
            default: return AAA_ERR_FAILURE;
	    }
	}
};

class DIAMETERBASEPROTOCOL_EXPORT AAAServerSession : 
    public AAASession,
    public DiameterServerAuthSession
{
    public:
        AAAServerSession(AAAApplicationCore &appCore,
                         diameter_unsigned32_t id) :
	    AAASession(appCore, id),
	    DiameterServerAuthSession(appCore.GetTask(), id) {
	    m_IO = this;
	}
        virtual ~AAAServerSession() {
            End();
	}
        AAAReturnCode Abort() {
	    return End();
	}
    private:
        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
	    return CheckUpdateEvent(msg);
        }
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {
	    return CallMsgHandler(msg);
	}
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
	    return CallMsgHandler(msg);
	}
        virtual AAAReturnCode Disconnect() {
            return this->HandleDisconnect();
        }
        virtual AAAReturnCode SessionTimeout() {
	    this->HandleTimeout();
	    return this->HandleSessionTimeout();
        }
        virtual AAAReturnCode AuthorizationTimeout() {
	    this->HandleAuthGracePeriodTimeout();
	    return this->HandleAuthLifetimeTimeout();
        }
        virtual AAAReturnCode AbortSession() {
	    return this->HandleAbort();
        }
        AAAReturnCode CheckUpdateEvent(DiameterMsg &msg) {
	    CallMsgHandler(msg);
	    switch (m_LastEvent) {
	    case EVENT_AUTH_SUCCESS:  return AAA_ERR_SUCCESS;
            case EVENT_AUTH_CONTINUE: return AAA_ERR_INCOMPLETE;
            default: return AAA_ERR_FAILURE;
	    }
	}
};

class DIAMETERBASEPROTOCOL_EXPORT AAAMessageControl 
{
    public:
        AAAMessageControl(AAASession *s) : 
            m_Session(*s) {
        }
        virtual ~AAAMessageControl() {
	}
        AAAReturnCode SetResultCode(DiameterMsg &response, 
                                    DiameterMsg &request, 
                                    DiameterResultCode resultCode) {
           DiameterMsgResultCode rcode(response);
           rcode.ResultCode(resultCode);
           return (AAA_ERR_SUCCESS);
	}
        AAAReturnCode Send(DiameterMsg &msg) {
	   std::auto_ptr<DiameterMsg> newMsg(new DiameterMsg);
           newMsg->hdr = msg.hdr;          
           while (! msg.acl.empty()) {
	       AAAAvpContainer *c = msg.acl.front();
               msg.acl.pop_front();
               newMsg->acl.add(c);
	   }

           DiameterIdentityAvpContainerWidget oHostAvp(newMsg->acl);
           DiameterIdentityAvpContainerWidget oRealmAvp(newMsg->acl);

           // resolve origin host
           diameter_identity_t *oHost = oHostAvp.GetAvp
                 (DIAMETER_AVPNAME_ORIGINHOST);
           if (oHost == NULL) {
               oHostAvp.AddAvp(DIAMETER_AVPNAME_ORIGINHOST) = 
		   DIAMETER_CFG_TRANSPORT()->identity;
           }
           else if (oHost->length() == 0) {
	       *oHost = DIAMETER_CFG_TRANSPORT()->identity;
	   }

           // resolve origin realm
           diameter_identity_t *oRealm = oRealmAvp.GetAvp
                   (DIAMETER_AVPNAME_ORIGINREALM);
           if (oRealm == NULL) {
               oRealmAvp.AddAvp(DIAMETER_AVPNAME_ORIGINREALM) = 
		   DIAMETER_CFG_TRANSPORT()->realm;
           }
           else if (oRealm->length() == 0) {
	       *oRealm = DIAMETER_CFG_TRANSPORT()->realm;
	   }

	   return m_Session.IO()->Send(newMsg);
	}

    protected:
        AAASession &m_Session;
};

class DIAMETERBASEPROTOCOL_EXPORT AAAAccountingSession : 
    public AAASession
{
    public:
        AAAAccountingSession(AAAApplicationCore &appCore,
                             diameter_unsigned32_t id) :
	    AAASession(appCore, id) {
	}
        virtual ~AAAAccountingSession() {
	}
        AAAReturnCode Send(DiameterMsg &reqMsg) {
	    AAAMessageControl msgCntrl(this);
            return msgCntrl.Send(reqMsg);
	}
};

class DIAMETERBASEPROTOCOL_EXPORT AAAAccountingClientSession : 
    public AAAAccountingSession
{
    private:
        class LocalCollector : public DiameterClientAcctRecCollector {
            public:
	        LocalCollector() : 
                   m_Session(0) {
	        }
                virtual void GenerateRecord(AAAAvpContainerList &avpList,
                                            int recordType,
                                            int recordNum) {
		    if (m_Session) {
		       m_Session->HandleInterimRecordEvent
                          ((AAAAccountingClientSession::RECTYPE)recordType,
			   m_Session->m_LastPayload);
		    }
		}
                virtual bool IsLastRecordInStorage() {
		    return (false);
		}
                virtual bool IsStorageSpaceAvailable() {
		    return (true);
		}
                virtual AAAReturnCode StoreLastRecord(int recordType) {
		    return (AAA_ERR_SUCCESS);
		}
                virtual AAAReturnCode DeleteLastRecord(int recordType) {
		    return (AAA_ERR_SUCCESS);
		}
                AAAAccountingClientSession *m_Session;
                friend class AAAAccountingClientSession;
        };

    public:
        typedef enum {
            RECTYPE_EVENT = 1,
            RECTYPE_START = 2,
            RECTYPE_INTERIM = 3,
            RECTYPE_STOP = 4
        } RECTYPE;

    public:
        AAAAccountingClientSession(AAAApplicationCore &appCore,
                                   diameter_unsigned32_t id) :
	   AAAAccountingSession(appCore, id),
	   m_ParentSession(appCore.GetTask(), id),
           m_LastPayload(NULL) {
           m_SubSession = std::auto_ptr<DiameterClientAcctSubSession<LocalCollector> >
                (new DiameterClientAcctSubSession<LocalCollector>(m_ParentSession));
           m_SubSession->RecCollector().m_Session = this;
           m_IO = m_SubSession.get();
           m_SubSession->Attributes().BackwardCompatibility() = true;
	}
        virtual ~AAAAccountingClientSession() {
	}
        AAAReturnCode SetInterimRecordInterval(RECTYPE type, 
                                               int interval = 0,
                                               AAASessionPayload payload = NULL) {
	   DIAMETER_CFG_ACCT_SESSION()->recIntervalTm = interval;
           m_LastPayload = payload;
           switch (type) {
	      case RECTYPE_EVENT: m_SubSession->Begin(true); break;
              case RECTYPE_START: m_SubSession->Begin(false); break;
              case RECTYPE_INTERIM: break;
              case RECTYPE_STOP: m_SubSession->End(); break;
	   }
           return (AAA_ERR_SUCCESS);
	}
        virtual AAAReturnCode HandleInterimRecordEvent(RECTYPE type, 
                                                       AAASessionPayload payload) {
	   return (AAA_ERR_SUCCESS);
	}
     private:
        DiameterClientAcctSession m_ParentSession;
        std::auto_ptr<DiameterClientAcctSubSession<LocalCollector> >
               m_SubSession;
        AAASessionPayload m_LastPayload;
};

typedef DiameterAccountingRecTransformer AAAAccountingRecTransformer;

typedef DiameterAccountingXMLRecTransformer AAAAccountingXMLRecTransformer;

class DIAMETERBASEPROTOCOL_EXPORT AAAAccountingServerSession : 
    public AAAAccountingSession
{
    private:
        class LocalStorage : public DiameterServerAcctRecStorage
        {
            public:
	       LocalStorage() :
                  m_Session(0) {
	       }
	       virtual bool IsSpaceAvailableOnDevice() {
		  return (true);
	       }
               virtual void StoreRecord(AAAAvpContainerList &avpList,
                                        int recordType,
                                        int recordNum) {
		  if (m_Session) {
		      DiameterMsg msg;
                      ACE_OS::memset(&msg.hdr, 0, sizeof(msg.hdr));
                      msg.hdr.ver = DIAMETER_PROTOCOL_VERSION;
                      msg.hdr.length = 0;
                      msg.hdr.flags.r = DIAMETER_FLAG_SET;
                      msg.hdr.flags.p = DIAMETER_FLAG_CLR;
                      msg.hdr.flags.e = DIAMETER_FLAG_CLR;
                      msg.hdr.code = DIAMETER_MSGCODE_ACCOUNTING;
                      msg.hdr.appId = m_Session->GetApplicationId();

                      while (! avpList.empty()) {
	                  AAAAvpContainer *c = avpList.front();
                          avpList.pop_front();
                          msg.acl.add(c);
	              }

                      m_Session->GetTransformer()->Convert(&msg);
                      m_Session->GetTransformer()->OutputRecord(&msg);
		  }
	       }           
               virtual void UpdateAcctResponse(DiameterMsg &aca) {
	       }
               AAAAccountingServerSession *m_Session;
        };

    public:
        AAAAccountingServerSession(AAAApplicationCore &appCore,
                                   diameter_unsigned32_t id) :
	    AAAAccountingSession(appCore, id) {
            m_Session = std::auto_ptr<DiameterServerAcctSession<LocalStorage> >
                (new DiameterServerAcctSession<LocalStorage>(appCore.GetTask(), id));
            m_Session->RecStorage().m_Session = this;
            m_Session->Attributes().BackwardCompatibility() = true;
            m_IO = m_Session.get();
	}
        virtual ~AAAAccountingServerSession() {
	}
        void SetTransformer(DiameterAccountingRecTransformer *trns) { 
            m_Transformer = trns; 
        }
	DiameterAccountingRecTransformer *GetTransformer() { 
            return m_Transformer; 
        }

    protected:
        DiameterAccountingRecTransformer *m_Transformer;
        std::auto_ptr<DiameterServerAcctSession<LocalStorage> >
	    m_Session;
};

class DIAMETERBASEPROTOCOL_EXPORT AAAProxyServices
{    
    private:
        class LocalHandler : public AAA_ProxyHandler
        {
           public:
               LocalHandler(AAAEventHandler *h,
                            diameter_unsigned32_t appId) :                  
		  AAA_ProxyHandler(appId),
		  m_ApplicationId(appId),
                  m_Handler(h) {
               }
               virtual ~LocalHandler() {
               }
               virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
		  return m_Handler->HandleMessage(msg);
               }
               virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {
		  return m_Handler->HandleMessage(msg);
               }
               virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
		  return m_Handler->HandleMessage(msg);
               }
               diameter_unsigned32_t &ApplicationId() {
                  return m_ApplicationId;
               }
           protected:
               diameter_unsigned32_t m_ApplicationId;
               AAAEventHandler *m_Handler;
        };
        typedef std::map<diameter_unsigned32_t,
                         LocalHandler*>
                         AAAHandlerMap;
        typedef std::pair<diameter_unsigned32_t,
                          LocalHandler*>
                          AAAHandlerMapPair;

    public:
        AAAProxyServices(AAAApplicationCore &appCore) :
            m_Core(appCore) {
        }
        virtual ~AAAProxyServices() { 
	    while (! m_HandlerMap.empty()) {
	        AAAHandlerMap::iterator i = m_HandlerMap.begin();
                delete i->second;
                m_HandlerMap.erase(i);
	    }
        }
        AAAReturnCode RegisterMessageHandler(diameter_unsigned32_t appId,
                                             AAAEventHandler *handler) {
	    AAAHandlerMap::iterator i = m_HandlerMap.find(appId);
	    if (m_HandlerMap.end() != i) {
	        return (AAA_ERR_FAILURE);
	    }
            LocalHandler *h = new LocalHandler(handler, appId);
            if (h) {
                m_HandlerMap.insert(AAAHandlerMapPair(appId, h));
                return (AAA_ERR_SUCCESS);            
	    }
            return (AAA_ERR_FAILURE);
	}
        AAAReturnCode RemoveMessageHandler(diameter_unsigned32_t appId) {
	    AAAHandlerMap::iterator i = m_HandlerMap.find(appId);
	    if (m_HandlerMap.end() != i) {
	        delete i->second;
                m_HandlerMap.erase(i);
	    }
            return (AAA_ERR_SUCCESS);            
	}

    protected:
        AAAApplicationCore &m_Core;
        AAAHandlerMap m_HandlerMap;
};

#endif   // __DIAMETER_API_H__ 



