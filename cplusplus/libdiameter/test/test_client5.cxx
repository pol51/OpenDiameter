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

// victor fajardo: sample client 

#include "diameter_api.h"

class AAA_SampleClient : public DiameterClientAuthSession {
        // AAA client session derived from DiameterClientAuthSession.
        // It provides for all the functionality of a diameter 
        // client session. Note that the application is responsible
        // for instantiating this object
    protected:
        typedef enum {
            MSG_COUNT = 5
        };
        
    public:
        AAA_SampleClient(AAA_Task &task,
                         diameter_unsigned32_t id) :
            DiameterClientAuthSession(task, id),
            m_MsgCount(0),
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
            
            // NOTE: We are not maintaining session here
            authState = DIAMETER_SESSION_NO_STATE_MAINTAINED;
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
            timeout = 30;
        }
        virtual void SetAuthLifetimeTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this client can send a hint to the server 
            // about the auth lifetime it prefers. If not
            // overridden, the value in the config file
            // is used
            timeout = 29;
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
            // AAA clients normally should not receive
            // request messags. 
            AAA_LOG((LM_INFO, "(%P|%t) **** Request message message received in client ??? ****\n"));
            DiameterMsgHeaderDump::Dump(msg);
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {

            // all answer messages are handled by this function. 
            // This function can retrun the following values:
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
            if (m_MsgCount ++ < MSG_COUNT) {
                TxAuthenticationRequest();
                return AAA_ERR_INCOMPLETE;
            }
            return AAA_ERR_SUCCESS;
        }
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
            // all error messages are handled by this function.
            AAA_LOG((LM_INFO, "(%P|%t) **** Received message with error bit set ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Success() {
            // notification of successful auth
            AAA_LOG((LM_INFO, "(%P|%t) **** user authorized ****\n"));
            m_Success = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Disconnect() {
            // notification of completed STR/STA exchange
            AAA_LOG((LM_INFO, "(%P|%t) **** session disconnecting ****\n"));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode SessionTimeout() {
            // notification of session timeout
            AAA_LOG((LM_INFO, "(%P|%t) **** session timeout ****\n"));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AuthorizationTimeout() {
            // notification of auth lifetime timeout
            AAA_LOG((LM_INFO, "(%P|%t) **** auth timeout ****\n"));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AbortSession() {
            // notification of completed ASR/ASA exchange
            AAA_LOG((LM_INFO, "(%P|%t) **** session aborted by server ****\n"));
            m_Disconnected = true;
            return (AAA_ERR_SUCCESS);
        }
        AAAReturnCode TxAuthenticationRequest() {
            std::cout << "Sending request message" << std::endl;

            // sample of how to compose a message using parser widgets

            DiameterMsgWidget msg(300, true, 10000);

            DiameterUInt32AvpWidget authIdAvp(DIAMETER_AVPNAME_AUTHAPPID);
            DiameterUtf8AvpWidget unameAvp(DIAMETER_AVPNAME_USERNAME);
            DiameterEnumAvpWidget reAuthAvp(DIAMETER_AVPNAME_REAUTHREQTYPE);

            authIdAvp.Get() = 10000; // my application id
            unameAvp.Get() = "username@domain.com";
            reAuthAvp.Get() = 1;

            msg()->acl.add(authIdAvp());
            msg()->acl.add(unameAvp());
            msg()->acl.add(reAuthAvp());

            Send(msg());
            return (AAA_ERR_SUCCESS);
        }
        bool UserAuthorized() {
            return m_Success;
        }
        bool SessionDisconnected() {
            return m_Disconnected;
	}
    private:
        int m_MsgCount;
        bool m_Success;
        bool m_Disconnected;
};

//
// NOTE: You can use this sample with sample_server2.cxx
// This sample has the same basic setup as sample_client2.cxx
// except that the sessions are stateless and only one SSAR/SSAA
// exchange occurs.
//
int main(int argc, char *argv[])
{
   AAA_Task task;
   task.Start(5);

   char *cfgFile = "config/nas1.local.xml";

   if (argc == 2) {
       cfgFile = argv[1];
   }
   else if (argc > 2) {
       std::cout << "Usage: aaa_sample_client6 [config file]\n" 
                 << std::endl;
       exit(0);
   }

   // Application core is responsible for providing
   // peer connectivity between AAA entities
   DiameterApplication appCore(task);
   if (appCore.Open(cfgFile) == AAA_ERR_SUCCESS) {

       /// Wait for connectivity
       do {
           std::cout << "Waiting till this AAA has connectivity" << std::endl;
           ACE_OS::sleep(1);
       } while (appCore.NumActivePeers() == 0);

       /// send the client request
       AAA_SampleClient client(task, 1);
       client.Begin("my_client");
       client.TxAuthenticationRequest();

       /// wait till user is authorized
       do {
           std::cout << "Waiting till user is authorized" << std::endl;
           ACE_OS::sleep(1);
       } while (! client.UserAuthorized());

       client.End();

       /// wait till user is disconnected, this is to make sure
       /// 'client' object is not deleted before STR/STA exchange
       //  completes
       do {
           std::cout << "Waiting till user is disconnected" << std::endl;
           ACE_OS::sleep(1);
       } while (! client.SessionDisconnected());
   }

   appCore.Close();
   task.Stop();
   return (0);
}



