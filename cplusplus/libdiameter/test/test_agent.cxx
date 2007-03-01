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

// victor fajardo: sample agent

#include "diameter_api.h"
#include "aaa_session_msg_mux.h"

class AAA_SampleProxy : public AAA_ProxyHandler
{
        // This is a sample proxy handler for messages
        // passing through this AAA entity. Note that
        // for both request and answer messages, routing
        // AVP and other base protocol management AVP's
        // are managed by the library. Note also that
        // the current design will not maintain session
        // state and acts only as a passthrough for
        // messages of a particular session id.
    public:
        AAA_SampleProxy() : 
             AAA_ProxyHandler(10000) {
        }

        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {

            /// This function is called when incomming request 
            /// message is received. This method is given temporary
            /// ownership of 'msg'. If this method returns AAA_ERR_SUCCESS
            /// then the message will be forwarded to the proper 
            /// destination via the router. Note that proxy handlers
            /// are special cases and are session-less in this context
            /// so routing is done as a means of transport.
            std::cout << "**** Proxy request message ****" << std::endl;

            DiameterMsgHeaderDump::Dump(msg);

            DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
            DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
            DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
            DiameterUInt32AvpContainerWidget authAppIdAvp(msg.acl);
            DiameterEnumAvpContainerWidget reAuthAvp(msg.acl);

            diameter_identity_t *host = oHostAvp.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
            diameter_identity_t *realm = oRealmAvp.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
            diameter_utf8string_t *uname = uNameAvp.GetAvp(DIAMETER_AVPNAME_USERNAME);
            diameter_unsigned32_t *authAppId = authAppIdAvp.GetAvp(DIAMETER_AVPNAME_AUTHAPPID);
            diameter_enumerated_t *reAuth = reAuthAvp.GetAvp(DIAMETER_AVPNAME_REAUTHREQTYPE);

            if (host) {
                AAA_LOG((LM_INFO, "(%P|%t) From Host: %s\n", host->c_str()));
            }
            if (realm) {
                AAA_LOG((LM_INFO, "(%P|%t) From Realm: %s\n", realm->c_str()));
            }
            if (uname) {
                AAA_LOG((LM_INFO, "(%P|%t) From User: %s\n", uname->c_str()));
            }
            if (authAppId) {
                AAA_LOG((LM_INFO, "(%P|%t) Auth Application Id: %d\n", *authAppId));
            }
            if (reAuth) {
                AAA_LOG((LM_INFO, "(%P|%t) Re-Auth Request type: %d\n", *reAuth));
	    }
            // If a value other than AAA_ERR_SUCCESS is return
            // then the message will be dropped.
            return (AAA_ERR_SUCCESS);
        }

        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {

            /// This function is called when incomming answer message 
            /// is received. Same behavior as RequestMsg().

            std::cout << "**** Proxy answer message ****" << std::endl;

            DiameterMsgHeaderDump::Dump(msg);

            DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
            DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
            DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
            DiameterUInt32AvpContainerWidget authAppIdAvp(msg.acl);
            DiameterEnumAvpContainerWidget reAuthAvp(msg.acl);

            diameter_identity_t *host = oHostAvp.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
            diameter_identity_t *realm = oRealmAvp.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
            diameter_utf8string_t *uname = uNameAvp.GetAvp(DIAMETER_AVPNAME_USERNAME);
            diameter_unsigned32_t *authAppId = authAppIdAvp.GetAvp(DIAMETER_AVPNAME_AUTHAPPID);
            diameter_enumerated_t *reAuth = reAuthAvp.GetAvp(DIAMETER_AVPNAME_REAUTHREQTYPE);

            if (host) {
                AAA_LOG((LM_INFO, "(%P|%t) From Host: %s\n", host->c_str()));
            }
            if (realm) {
                AAA_LOG((LM_INFO, "(%P|%t) From Realm: %s\n", realm->c_str()));
            }
            if (uname) {
                AAA_LOG((LM_INFO, "(%P|%t) From User: %s\n", uname->c_str()));
            }
            if (authAppId) {
                AAA_LOG((LM_INFO, "(%P|%t) Auth Application Id: %d\n", *authAppId));
            }
            if (reAuth) {
                AAA_LOG((LM_INFO, "(%P|%t) Re-Auth Request type: %d\n", *reAuth));
	    }

            // This answer will be forwarded regardless of 
            // the return value from this function
            return (AAA_ERR_SUCCESS);
        }

        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {

            /// This function is called when error message 
            /// is received. Same behavior as RequestMsg().
            /// You have the option of chaning the dest-host/dest-realm
            /// setting the error-bit and adding error-avp if necessary
            /// and send it back to the router.

            std::cout << "**** Proxy message with error ****" << std::endl;

            DiameterMsgHeaderDump::Dump(msg);

            // This answer will be forwarded regardless of 
            // the return value from this function
            return (AAA_ERR_SUCCESS);
        }
};

class AAA_SampleProxyWithMux : public AAA_ProxyHandler
{
    public:
         class AAA_ProxyMuxHandler : 
             public DiameterSessionMsgMuxHandler<AAA_SampleProxyWithMux>
         {
             public:

                 virtual AAAReturnCode RequestMsg(AAA_SampleProxyWithMux &arg, 
                                                  DiameterMsg &msg) {
                     // same as without mux
                     std::cout << "**** Proxy request message ****" << std::endl;
                     DiameterMsgHeaderDump::Dump(msg);
                     return (AAA_ERR_SUCCESS);
                 }

                 virtual AAAReturnCode AnswerMsg(AAA_SampleProxyWithMux &arg, 
                                                 DiameterMsg &msg) {
                     // same as without mux
                     std::cout << "**** Proxy answer message ****" << std::endl;
                     DiameterMsgHeaderDump::Dump(msg);
                     return (AAA_ERR_SUCCESS);
                 }

                 virtual AAAReturnCode ErrorMsg(AAA_SampleProxyWithMux &arg, 
                                                DiameterMsg &msg) {
                    // same as without mux
                    std::cout << "**** Proxy message with error ****" << std::endl;
                    DiameterMsgHeaderDump::Dump(msg);
                    return (AAA_ERR_SUCCESS);
                 }
        };

        // This is a sample proxy handler for messages
        // passing through this AAA entity. Note that
        // for both request and answer messages, routing
        // AVP and other base protocol management AVP's
        // are managed by the library. Note also that
        // the current design will not maintain session
        // state and acts only as a passthrough for
        // messages of a particular session id.
    public:
        AAA_SampleProxyWithMux() : 
             AAA_ProxyHandler(20000),
             m_Mux(*this) {
             m_Mux.Register(300, m_Action);
        }
        ~AAA_SampleProxyWithMux() {
             m_Mux.Remove(300);
	}

        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
            // same as without mux
            return m_Mux.Mux(msg);
	}
  
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {
            // same as without mux
            return m_Mux.Mux(msg);
        }
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
            // same as without mux
            return m_Mux.Mux(msg);
        }

   private:
        AAA_ProxyMuxHandler m_Action;
        DiameterSessionMsgMux<AAA_SampleProxyWithMux> m_Mux;
};

class PeerEventHandler : 
   public DiameterPeerEventInterface
{
   public:
      virtual void Connected() {
         AAA_LOG((LM_INFO, "(%P|%t) **** peer is now connected ****\n"));
      }
      virtual void Disconnected(int cause) {
         AAA_LOG((LM_INFO, "(%P|%t) **** peer is now disconnected: %d ****\n",
                 cause));
      }
      virtual void Error(PFSM_EV_ERR err) {
         AAA_LOG((LM_INFO, "(%P|%t) **** peer is now disconnected: %d ****\n",
                 err));
         switch (err) {
            case PEER_EVENT_CONN_NACK:
               break;
            case PEER_EVENT_TIMEOUT_OR_NONCEA:
               break;
         }
      }
};

int main(int argc, char *argv[])
{
   AAA_Task task;
   task.Start(5);

   // Application core is responsible for providing
   // peer connectivity between AAA entities
   DiameterApplication appCore(task, "config/agent.local.xml");
   AAA_SampleProxy myProxy;
   AAA_SampleProxyWithMux myProxyWithMux;

   /// Example of connecting dynamically 
   //  to peers not in the peer table. It
   //  also shows how to register an event
   //  handler to a peer entry. The event
   //  handler can be registered to any peer
   //  wether static or dynamic
   PeerEventHandler peerhandler;
   DiameterPeer *dyncPeer = NULL;
   std::string peername("dynamic.peer.com");
   DiameterPeerManager peerMngr(task);
   if (peerMngr.Add(peername, // hostname of peer to connect to
                    1812,     // port number of host
                    false,    // Use TCP
                    false,    // tls support
                    50,
                    false)) {
      AAA_LOG((LM_INFO, "(%P|%t) **** failed to create dynamic peer: %s***\n",
              peername.c_str()));
   }
   else {       
      // If a dynamic peer is successfully created
      // then register a peer handler and start the
      // peer
      dyncPeer = peerMngr.Lookup(peername);
      dyncPeer->RegisterUserEventHandler(peerhandler);
      dyncPeer->Start();
   }

   // register our proxy handler
   appCore.RegisterProxyHandler(myProxy);
   appCore.RegisterProxyHandler(myProxyWithMux);

   // server loop (this could be done better)
   while (true) {
       ACE_OS::sleep(10);
   }

   appCore.RemoveProxyHandler(10000);
   appCore.RemoveProxyHandler(20000);
   
   if (dyncPeer) {
       dyncPeer->RemoveUserEventHandler();
   }

   appCore.Close();
   task.Stop();
   return (0);
}






