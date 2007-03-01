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

// victor fajardo: sample server using message mux and multiple session

#include "diameter_api.h"
#include "aaa_session_msg_mux.h"

static int gReqMsgCount = 0;
static int gSessionCount = 0;

class AAA_SampleServer : public DiameterServerAuthSession,
                         public DiameterSessionMsgMux<AAA_SampleServer>
{
        // AAA server session derived from DiameterServerAuthSession
        // and an DiameterSessionMsgMux<>. The message mux allows 
        // message delegation to registered message handlers.
        // The server session is the same as in sample_server2.cxx.
    public:
        AAA_SampleServer(AAA_Task &task,
                         diameter_unsigned32_t id) :
           DiameterServerAuthSession(task, id),
           DiameterSessionMsgMux<AAA_SampleServer>(*this) {
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
        virtual void SetSessionTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this server can dictate the session timeout 
            // to the client. If not overridden, the value 
            // in the config file is used
            timeout = 30;
        }
        virtual void SetAuthLifetimeTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this server can dictate the auth lifetime
            // to the client. If not overridden, the value 
            // in the config file is used
            timeout = 2;
        }
        virtual void SetAuthGracePeriodTimeout
        (DiameterScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this server can dictate the auth grace period
            // to the client. If not overridden, the value 
            // in the config file is used
            timeout = 2;
        }
        virtual AAAReturnCode ReAuthenticate(diameter_unsigned32_t rcode) {
            // optional override, called by the library so 
            // this server is informed that the client has
            // responded to the server initiated re-auth
            // request. The result code from the client
            // is passed as a parameter to this funciton.
            AAA_LOG((LM_INFO, "(%P|%t) **** client responded to re-auth ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode RequestMsg(DiameterMsg &msg) {
            // all request messages are handled by this function.
            // Note that you can call the Mux() method
            // here to deligate handling of messages. See
            // sample_server2.cxx and comments below in the
            // RequestMsg() of the mux message handler
            AAA_LOG((LM_INFO, "(%P|%t) **** client responded to re-auth ****\n"));
            return Mux(msg);
        }
        virtual AAAReturnCode AnswerMsg(DiameterMsg &msg) {
            // all answer messages are handled by this function.
            // AAA servers normally will receive answer messages
            // in the open state

            // all answer messages are handled by this function. 
            // This function can retrun the following values:
            // a. AAA_ERR_SUCCESS - client has successfully responded
            //                      to server request
            // b. AAA_ERR_FAILURE - client failed. 
            AAA_LOG((LM_INFO, "(%P|%t) **** Answer message message received in server ****\n"));
            DiameterMsgHeaderDump::Dump(msg);
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode ErrorMsg(DiameterMsg &msg) {
            // all error messages are handled by this function.
            AAA_LOG((LM_INFO, "(%P|%t) **** Received message with error bit set ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Success() {
            // notification of successful auth
            AAA_LOG((LM_INFO, "(%P|%t) **** user authorized ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Disconnect() {
            // notification of completed STR/STA exchange
            AAA_LOG((LM_INFO, "(%P|%t) **** session disconnecting ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode ReClaimSession() {
            // notification of a stateless session being reclaimed
            // return AAA_ERR_SUCCESS if you wish to delete this server
            // session otherwise return AAA_ERR_FAILURE
            AAA_LOG((LM_INFO, "(%P|%t) **** session re-claim ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode SessionTimeout() {
            // notification of session timeout
            AAA_LOG((LM_INFO, "(%P|%t) **** session timeout ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AuthorizationTimeout() {
            // notification of auth lifetime timeout
            AAA_LOG((LM_INFO, "(%P|%t) **** auth timeout ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AbortSession() {
            // notification of completed ASR/ASA exchange
            AAA_LOG((LM_INFO, "(%P|%t) **** session aborted by server ****\n"));
            return (AAA_ERR_SUCCESS);
        }
};

class AAA_SampleServerAction : 
    public DiameterSessionMsgMuxHandler<AAA_SampleServer>
{
        // AAA message multiplex handler. This is a
        // handler class for a specific AAA message.
        // and instance of this class is registered
        // with an DiameterSessionMsgMux<> object
    public:
        virtual AAAReturnCode AnswerMsg(AAA_SampleServer &server, DiameterMsg &msg) {
            // all answer messages are handled by this function.
            // AAA servers normally should not receive
            // answer messags. In this sample, this will not
            // be called since the mux is not called in the
            // AnswerMsg() in the server session
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode RequestMsg(AAA_SampleServer &server, DiameterMsg &msg) {

            // all request messages with code 300 are handled by
            // this function. This function can retrun the following
            // values:
            // a. AAA_ERR_SUCCESS - client is successfully authenticated
            // b. AAA_ERR_INCOMPLETE - auth not yet completed, muti-round 
            //                         message trip exchange
            // c. AAA_ERR_FAILURE - client authentication failed

            AAA_LOG((LM_INFO, "(%P|%t) Request message received\n"));
            DiameterMsgHeaderDump::Dump(msg);

            DiameterIdentityAvpContainerWidget oHostAvp(msg.acl);
            DiameterIdentityAvpContainerWidget oRealmAvp(msg.acl);
            DiameterUtf8AvpContainerWidget uNameAvp(msg.acl);
            DiameterUInt32AvpContainerWidget authAppIdAvp(msg.acl);
            DiameterEnumAvpContainerWidget reAuthAvp(msg.acl);
            DiameterGroupedAvpContainerWidget tunneling(msg.acl);

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

            AAA_LOG((LM_INFO, "(%P|%t) Request Message Count: %d\n", ++gReqMsgCount));
            return TxAuthenticationAnswer(server);
        }
        virtual AAAReturnCode ErrorMsg(AAA_SampleServer &server, DiameterMsg &msg) {
            // same as ErrorMsg of server session
            AAA_LOG((LM_INFO, "(%P|%t) **** Received message with error bit set ****\n"));
            return (AAA_ERR_SUCCESS);
        }
        AAAReturnCode TxAuthenticationAnswer(AAA_SampleServer &server) {

            // sample of how to compose a message using parser widgets

            std::cout << "Sending answer message" << std::endl;

            DiameterMsgWidget msg(300, false, 10000);

            DiameterUInt32AvpWidget authIdAvp(DIAMETER_AVPNAME_AUTHAPPID);
            DiameterUtf8AvpWidget unameAvp(DIAMETER_AVPNAME_USERNAME);
            DiameterGroupedAvpWidget tunneling("Tunneling");
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

            authIdAvp.Get() = 10000; // my application id
            unameAvp.Get() = "username@domain.com";

            msg()->acl.add(authIdAvp());
            msg()->acl.add(unameAvp());
            msg()->acl.add(tunneling());

            DiameterMsgResultCode rcode(*msg());
            rcode.ResultCode(AAA_SUCCESS);

            server.Send(msg());
            return (AAA_ERR_SUCCESS);
        }
};

class AAA_SampleServerSessionAllocator : 
    public DiameterServerSessionFactory
{
	// Server session factory. Unlike AAA clients, server
        // sessions need to be created on demand. This factory
        // is responsible for creating new server sessions
        // based on incomming new request. This is an extension
        // the server factory in sample_server2.cxx where the
        // actual CreateInstance() method is implemented by the
        // application.
    public:
       AAA_SampleServerSessionAllocator(AAA_Task &task,
                                        diameter_unsigned32_t appId) : 
            DiameterServerSessionFactory(task, appId) { 
       }
       AAA_SampleServer *CreateInstance() {
            // This method is called by the library to ask
            // the application specific to create a server 
            // session
            AAA_SampleServer *session = new AAA_SampleServer(m_Task, m_ApplicationId);
            if (session) {
                AAA_LOG((LM_INFO, "(%P|%t) Session Count: %d\n", ++gSessionCount));
                session->Register(300, m_Action);
            }
            return session;
       }
    private:
       AAA_SampleServerAction m_Action;
};

int main(int argc, char *argv[])
{
   AAA_Task task;
   task.Start(5);

   // Application core is responsible for providing
   // peer connectivity between AAA entities
   DiameterApplication appCore(task, "config/isp.local.xml");
   AAA_SampleServerSessionAllocator allocator(task, 10000);
   appCore.RegisterServerSessionFactory(allocator);

   while (true) {
      std::cout << "Just wait here and let factory take care of new sessions" << std::endl;
      ACE_OS::sleep(10);
   }

   appCore.Close();
   task.Stop();
   return (0);
}






