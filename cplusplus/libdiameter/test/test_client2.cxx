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

// victor fajardo: sample client using message mux and multiple session

#include "diameter_api.h"
#include "aaa_session_msg_mux.h"

class AAA_SampleClient : public DiameterClientAuthSession,
                         public DiameterSessionMsgMux<AAA_SampleClient>
{
        // AAA client session derived from DiameterClientAuthSession
        // and an DiameterSessionMsgMux<>. The message mux allows 
        // message delegation to registered message handlers.
        // The client session is the same as in sample_client2.cxx.
    public:
        AAA_SampleClient(AAA_Task &task,
                         diameter_unsigned32_t id,
                         int number) :
            DiameterClientAuthSession(task, id),
            DiameterSessionMsgMux<AAA_SampleClient>(*this),
            m_SessionNum(number),
            m_Success(false),
            m_Disconnected(false) {
        }
        virtual void SetAuthSessionState
        (DiameterScholarAttribute<diameter_unsigned32_t> &authState)
        {
            // optional override, called by the library to set 
            // the auth state. Note that this overrides the 
            // settings in the configuration file or applications 
            // sending an auth session state AVP
            authState = DIAMETER_SESSION_STATE_MAINTAINED;
        }
        virtual void SetDestinationHost
        (DiameterScholarAttribute<diameter_identity_t> &dHost)
        {
            // optional override, called by the library to 
            // set the destination host. Note that this 
            // overrides applications sending a destination
            // host AVP
            dHost = "server.isp.net";
        }
        virtual void SetDestinationRealm
        (DiameterScholarAttribute<diameter_identity_t> &dRealm)
        {
            // optional override, called by the library 
            // to set the destination realm. Note that 
            // this overrides applications sending a 
            // destination realm AVP
            dRealm = "isp.net";
        }
        virtual void SetSessionTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this client can send a hint to the server 
            // about the session timeout it prefers. If not
            // overridden, the value in the config file
            // is used
            timeout = 60;
        }
        virtual AAAReturnCode ReAuthenticate(diameter_unsigned32_t type) {
            // optional override, called by the library so 
            // this client is informed about a re-auth request
            // initiated by the server. Note that the client
            // must return a valid result-code when exiting
            // this function
            AAA_LOG((LM_INFO, "(%P|%t) **** server re-authentication ****\n"));
            return (AAAReturnCode)(AAA_SUCCESS);
        }
        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
            // all request messages are handled by this function.
            // AAA clients normally will receive request message
            // in the open state

            // all request messages are handled by this function. 
            // This function can retrun the following values:
            // a. AAA_ERR_SUCCESS - client is successfully responded
            //                      to server request
            // b. AAA_ERR_FAILURE - client failed. 
            AAA_LOG((LM_INFO, "(%P|%t) **** Request message message received in client ****\n"));
            DiameterMsgHeaderDump::Dump(msg);
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {
            // all answer messages are handled by this function.
            // Note that you can call the Mux() method
            // here to deligate handling of messages. See
            // sample_client2.cxx and comments below in the
            // AnswerMsg() of the mux message handler
            return Mux(msg);
        }
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
            // all error messages are handled by this function.
            AAA_LOG((LM_INFO, "(%P|%t) **** [Num:%d] Received message with error bit set ****\n",
                    m_SessionNum));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Success() {
            // notification of successful auth
            AAA_LOG((LM_INFO, "(%P|%t) **** [Num:%d] user authorized ****\n",
                    m_SessionNum));
            m_Success = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Disconnect() {
            // notification of completed STR/STA exchange
            AAA_LOG((LM_INFO, "(%P|%t) **** [Num:%d] session disconnecting ****\n",
                    m_SessionNum));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode SessionTimeout() {
            // notification of session timeout
            AAA_LOG((LM_INFO, "(%P|%t) **** [Num:%d] session timeout ****\n",
                    m_SessionNum));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AuthorizationTimeout() {
            // notification of auth lifetime timeout
            AAA_LOG((LM_INFO, "(%P|%t) **** [Num:%d] auth timeout ****\n",
                    m_SessionNum));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AbortSession() {
            // notification of completed ASR/ASA exchange
            AAA_LOG((LM_INFO, "(%P|%t) **** [Num:%d] session aborted by server ****\n",
                    m_SessionNum));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        bool UserAuthorized() {
            return m_Success;
        }
        bool SessionDisconnected() {
            return m_Disconnected;
        }
        int SessionNum() {
            return m_SessionNum;
        }
    private:
        int m_SessionNum;
        bool m_Success;
        bool m_Disconnected;
};

class AAA_SampleClientAction : 
    public DiameterSessionMsgMuxHandler<AAA_SampleClient>
{
        // AAA message multiplex handler. This is a
        // handler class for a specific AAA message.
        // and instance of this class is registered
        // with an DiameterSessionMsgMux<> object
    public:
        AAA_SampleClientAction(int howManyMsg = 1) :
            m_HowManyMsg(howManyMsg) {
        }
        virtual AAAReturnCode AnswerMsg(AAA_SampleClient &client, DiameterMsg &msg) {

            // all answer messages with code 300 are handled by
            // this function. This function can retrun the following
            // values:
            // a. AAA_ERR_SUCCESS - client is successfully authenticated
            // b. AAA_ERR_INCOMPLETE - auth not yet completed, muti-round 
            //                         message trip exchange
            // c. AAA_ERR_FAILURE - client authentication failed

            AAA_LOG((LM_INFO, "(%P|%t) Answer message received\n"));

            DiameterMsgHeaderDump::Dump(msg);

            DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
            DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
            DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
            DiameterUInt32AvpContainerWidget authAppIdAvp(msg.acl);
            DiameterGroupedAvpContainerWidget tunneling(msg.acl);

            diameter_identity_t *host = oHostAvp.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
            diameter_identity_t *realm = oRealmAvp.GetAvp(DIAMETER_AVPNAME_ORIGINREALM);
            diameter_utf8string_t *uname = uNameAvp.GetAvp(DIAMETER_AVPNAME_USERNAME);
            diameter_unsigned32_t *authAppId = authAppIdAvp.GetAvp(DIAMETER_AVPNAME_AUTHAPPID);

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

            diameter_grouped_t *grouped = tunneling.GetAvp("Tunneling");
            DiameterEnumAvpContainerWidget ttypeAvp(*grouped);
            DiameterEnumAvpContainerWidget tmediumAvp(*grouped);
            DiameterUtf8AvpContainerWidget cepAvp(*grouped);
            DiameterUtf8AvpContainerWidget sepAvp(*grouped);

            diameter_enumerated_t *ttype = ttypeAvp.GetAvp("Tunnel-Type");
            diameter_enumerated_t *tmedium = tmediumAvp.GetAvp("Tunnel-Medium-Type");
            diameter_utf8string_t *cep = cepAvp.GetAvp("Tunnel-Client-Endpoint");
            diameter_utf8string_t *sep = sepAvp.GetAvp("Tunnel-Server-Endpoint");

            if (ttype) {
                AAA_LOG((LM_INFO, "(%P|%t) Tunnel-Type: %d\n", *ttype));
            }
            if (tmedium) {
                AAA_LOG((LM_INFO, "(%P|%t) Medium: %d\n", *tmedium));
            }
            if (sep) {
                AAA_LOG((LM_INFO, "(%P|%t) Server EP: %s\n", sep->c_str()));
            }
            if (cep) {
                AAA_LOG((LM_INFO, "(%P|%t) Client EP: %s\n", cep->c_str()));
            }

            if ((-- m_HowManyMsg) > 0) {
                TxAuthenticationRequest(client);
                return (AAA_ERR_INCOMPLETE);
            }
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode RequestMsg(AAA_SampleClient &client, DiameterMsg &msg) {
            // all request messages are handled by this function.
            // AAA clients normally should not receive
            // request messags. In this sample, this will not
            // be called since the mux is not called in the
            // RequestMsg() in the client session
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode ErrorMsg(AAA_SampleClient &client, DiameterMsg &msg) {
            // same as ErrorMsg of client session
            return (AAA_ERR_SUCCESS);
        }
        AAAReturnCode TxAuthenticationRequest(AAA_SampleClient &client) {
            
            // sample of how to compose a message using parser widgets

            std::cout << "Sending request message" << std::endl;

            DiameterMsgWidget msg(300, true, 10000);

            DiameterUInt32AvpWidget authIdAvp(DIAMETER_AVPNAME_AUTHAPPID);
            DiameterUtf8AvpWidget unameAvp(DIAMETER_AVPNAME_USERNAME);
            DiameterEnumAvpWidget reAuthAvp(DIAMETER_AVPNAME_REAUTHREQTYPE);
            DiameterGroupedAvpWidget tunneling("Tunneling");

            authIdAvp.Get() = 10000; // my application id
            unameAvp.Get() = "username@domain.com";
            reAuthAvp.Get() = 1;

            DiameterEnumAvpWidget ttype("Tunnel-Type");
            DiameterEnumAvpWidget tmedium("Tunnel-Medium-Type");
            DiameterUtf8AvpWidget cep("Tunnel-Client-Endpoint");
            DiameterUtf8AvpWidget sep("Tunnel-Server-Endpoint");

            ttype.Get() = 100;
            tmedium.Get() = 200;
            cep.Get() = "ClientEnd";
            sep.Get() = "ServerEnd";
       
            diameter_grouped_t &grp = tunneling.Get();
            grp.add(ttype());
            grp.add(tmedium());
            grp.add(cep());
            grp.add(sep());

            msg()->acl.add(authIdAvp());
            msg()->acl.add(unameAvp());
            msg()->acl.add(reAuthAvp());
            msg()->acl.add(tunneling());

            client.Send(msg());
            return (AAA_ERR_SUCCESS);
        }
    private:
        int m_HowManyMsg;
};

class Client
{
    public:
        Client(AAA_Task &task,
               diameter_unsigned32_t id,
               int msgCount,
               int number) :
           m_Client(task, id, number),
           m_Action(msgCount) {
        }
        void Start() {
           m_Client.Register(300, m_Action);
           m_Client.Begin("my_client");
           m_Action.TxAuthenticationRequest(m_Client);
        }
        void Stop() {
           do {              
               std::cout << "Waiting till user is authorized #" 
                         << m_Client.SessionNum()
                         << std::endl;
               ACE_OS::sleep(1);
           } while (! m_Client.UserAuthorized());

           m_Client.End();

           do {              
               std::cout << "Waiting till user is disconnected, #" 
                         << m_Client.SessionNum()
                         << std::endl;
               ACE_OS::sleep(1);
           } while (! m_Client.SessionDisconnected());
        }

    private:
        AAA_SampleClient m_Client;
        AAA_SampleClientAction m_Action;
};

// An example of a peer event
// handler. We can register this
// to a static peer entry to notify
// us of peer fsm events.
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

   time_t start, end;
   start = time(0);

   int msgCountPerSession = 10;
   int sessionCount = 100;
   char *cfgFile = "config/nas2.local.xml";

   if (argc == 2) {
       cfgFile = argv[1];
   }
   else if (argc == 3) {
       cfgFile = argv[1];
       sessionCount = (atoi(argv[2]) > 0) ? 
	      atoi(argv[2]) : sessionCount;
   }
   else if (argc > 1) {
       std::cout << "Usage: aaa_sample_client2 [config file] [num session]\n" 
                 << std::endl;
       exit(0);
   }

   // Application core is responsible for providing
   // peer connectivity between AAA entities
   DiameterApplication appCore(task);
   if (appCore.Open(cfgFile) == AAA_ERR_SUCCESS) {

       /// We can get notified of events for "server.isp.net" peer
       PeerEventHandler handler;
       DiameterPeerManager peerMngr(task);
       std::string peerName("server.isp.net");
       DiameterPeer *dyncPeer = peerMngr.Lookup(peerName);
       dyncPeer->RegisterUserEventHandler(handler);
          
       /// Wait for connectivity
       do {
           std::cout << "Waiting till this AAA has connectivity" << std::endl;
           ACE_OS::sleep(1);
       } while (appCore.NumActivePeers() == 0);
  
       /// start all session
       Client **c = new Client*[sessionCount];
       for (int i=0; i < sessionCount; i++) {
           c[i] = new Client(task, 300, msgCountPerSession, i);
           c[i]->Start();
       }

       /// wait for all sessions to stop
       for (int i=0; i < sessionCount; i++) {
           c[i]->Stop();
           delete c[i];
       }
       delete c;
       
       if (dyncPeer) {
           dyncPeer->RemoveUserEventHandler();
       }
   }

   appCore.Close();
   task.Stop();

   end = time(0);

   printf("*********************** TOTAL [%ld] ****************\n", end - start);
   return (0);
}



