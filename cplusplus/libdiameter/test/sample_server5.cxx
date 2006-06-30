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

// victor fajardo: simple sample server 

#include "diameter_api.h"

class AAA_SampleAcctRecStorage : 
    public AAA_ServerAcctRecStorageWithConverter<diameter_utf8string_t>
{
        // see definition of AAA_ServerAcctRecStorageWithConverter
        // in aaa_session_acct_server_fsm.h
    public:
        virtual bool IsSpaceAvailableOnDevice() {
           /// Checks the server application if there is 
           /// enough storage space to hold the next
           /// record. Application must check thier specific
           /// storage schemes.
           return (true);
        }
        virtual diameter_utf8string_t *Convert
                          (AAAAvpContainerList &avpList,
                           int recordType,
                           int recordNum) {
           /// Asks the server application to convert
           /// the newly arrived record. Note that 
           /// the server application must be aware of
           /// the application specific AVP's it needs to
           /// process. Also, avpList contains all the
           /// relevant avp's in the ACR message such
           /// as Accounting-Sub-Session-Id, Acct-Session
           /// -Id ... etc which the application may wish
           /// to track
           AAA_Utf8AvpContainerWidget recAvp(avpList);
           diameter_utf8string_t *rec = recAvp.GetAvp("Example-Accounting-Record");
           return rec;
        }
        virtual void UpdateAcctResponse(AAAMessage &aca) {
           /// If you wish to add AVP's to the ACA
           /// before it is sent, you need to override
           /// this method and insert your AVP's here
           
           // as an example, add a timestamp to your aca
           time_t currentTime = time(0);
           if (currentTime > 0) {
              AAA_TimeAvpWidget tstampAvp("Event-Timestamp");
              tstampAvp.Get() = currentTime;
              aca.acl.add(tstampAvp());
           }
        }
        virtual void Output(diameter_utf8string_t &record) {
           std::cout << "Record: " << record << std::endl;             
	}
};

class AAA_SampleAcctServer : 
    public AAA_ServerAcctSession<AAA_SampleAcctRecStorage>
{
        // AAA serve session derived from AAA_ServerAcctSession.
        // It provides for all the functionality of a diameter 
        // accounting server session. Note that the server 
        // session factory is responsible for instantiating 
        // this object. AAA_ServerAcctSession is also a template
        // class that requires an AAA_ServerAcctRecStorage derived
        // class as a parameter.
    public:
        AAA_SampleAcctServer(AAA_Task &task,
                         diameter_unsigned32_t id) :
            AAA_ServerAcctSession<AAA_SampleAcctRecStorage>
                   (task, 
                    id, 
                    true) // dictates whether this session is stateful 
        {             
        }
        /// This function is used for setting realtime required 
        /// AVP as a hint to the server
        virtual void SetRealTimeRequired
        (AAA_ScholarAttribute<diameter_enumerated_t> &rt)
        {
            /// The following are possible values:
            ///   AAA_ACCT_REALTIME_DELIVER_AND_GRANT
            ///   ACCT_REALTIME_GRANT_AND_STORE
            ///   ACCT_REALTIME_GRANT_AND_LOSE
            rt = AAA_ACCT_REALTIME_DELIVER_AND_GRANT;
        }
        /// This function is used for setting acct interim 
        /// interval AVP dictated by the server to client
        virtual void SetInterimInterval
        (AAA_ScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            timeout = 2; // tell client to generate record every 2 sec
        }
        virtual AAAReturnCode Success() {
            // notification of successful ACR exchange for all record type
            AAA_LOG(LM_INFO, "(%P|%t) **** record exchange completed ****\n");
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Failed(int recNum) {
            // notification that recNum record was not processed properly
            AAA_LOG(LM_INFO, "(%P|%t) **** record #%d not processed ****\n", recNum);
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode SessionTimeout() {
            // notification of session timeout if this
            // session was stateful
            AAA_LOG(LM_INFO, "(%P|%t) **** session timeout ****\n");
            return (AAA_ERR_SUCCESS);
        }
};

class AAA_SampleAuthServer : public AAA_ServerAuthSession {
        // AAA serve session derived from AAA_ServerAuthSession.
        // It provides for all the functionality of a diameter 
        // server session. Note that the server session factory
        // is responsible for instantiating this object
    public:
        AAA_SampleAuthServer(AAA_Task &task,
                         diameter_unsigned32_t id,
                         bool endOnSuccess = false) :
           AAA_ServerAuthSession(task, id),
           m_EndOnSuccess(endOnSuccess) {
        }
        virtual void SetAuthSessionState
        (AAA_ScholarAttribute<diameter_unsigned32_t> &authState)
        {
            // optional override, called by the library to set 
            // the auth state. Note that this overrides the 
            // settings in the configuration file or applications 
            // sending an auth session state AVP
            authState = AAA_SESSION_STATE_MAINTAINED;
        }
        virtual void SetSessionTimeout
        (AAA_ScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this server can dictate the session timeout 
            // to the client. If not overridden, the value 
            // in the config file is used
            timeout = 30;
        }
        virtual void SetAuthLifetimeTimeout
        (AAA_ScholarAttribute<diameter_unsigned32_t> &timeout)
        {
            // optional override, called by the library so 
            // this server can dictate the session timeout 
            // to the client. If not overridden, the value 
            // in the config file is used
            timeout = 2;
        }
        virtual void SetAuthGracePeriodTimeout
        (AAA_ScholarAttribute<diameter_unsigned32_t> &timeout)
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
            AAA_LOG(LM_INFO, "(%P|%t) **** client responded to re-auth ****\n");
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode RequestMsg(AAAMessage &msg) {

            // all request messages are handled by this function. 
            // This function can retrun the following values:
            // a. AAA_ERR_SUCCESS - client is successfully authenticated
            // b. AAA_ERR_INCOMPLETE - auth not yet completed, muti-round 
            //                         message trip exchange
            // c. AAA_ERR_FAILURE - client authentication failed

            AAA_LOG(LM_INFO, "(%P|%t) Request message received\n");
            AAA_MsgDump::Dump(msg);

            AAA_IdentityAvpContainerWidget oHostAvp(msg.acl);
            AAA_IdentityAvpContainerWidget oRealmAvp(msg.acl);
            AAA_Utf8AvpContainerWidget uNameAvp(msg.acl);
            AAA_UInt32AvpContainerWidget authAppIdAvp(msg.acl);
            AAA_EnumAvpContainerWidget reAuthAvp(msg.acl);

            diameter_identity_t *host = oHostAvp.GetAvp(AAA_AVPNAME_ORIGINHOST);
            diameter_identity_t *realm = oRealmAvp.GetAvp(AAA_AVPNAME_ORIGINREALM);
            diameter_utf8string_t *uname = uNameAvp.GetAvp(AAA_AVPNAME_USERNAME);
            diameter_unsigned32_t *authAppId = authAppIdAvp.GetAvp(AAA_AVPNAME_AUTHAPPID);
            diameter_enumerated_t *reAuth = reAuthAvp.GetAvp(AAA_AVPNAME_REAUTHREQTYPE);

            if (host) {
                AAA_LOG(LM_INFO, "(%P|%t) From Host: %s\n", host->data());
            }
            if (realm) {
                AAA_LOG(LM_INFO, "(%P|%t) From Realm: %s\n", realm->data());
            }
            if (uname) {
                AAA_LOG(LM_INFO, "(%P|%t) From User: %s\n", uname->data());
            }
            if (authAppId) {
                AAA_LOG(LM_INFO, "(%P|%t) Auth Application Id: %d\n", *authAppId);
            }
            if (reAuth) {
                AAA_LOG(LM_INFO, "(%P|%t) Re-Auth Request type: %d\n", *reAuth);
	    }

            // Send answer back
            return TxAuthenticationAnswer();
        }
        virtual AAAReturnCode AnswerMsg(AAAMessage &msg) {
            // all answer messages are handled by this function.
            // AAA servers normally will receive answer messages
            // in the open state

            // all answer messages are handled by this function. 
            // This function can retrun the following values:
            // a. AAA_ERR_SUCCESS - client has successfully responded
            //                      to server request
            // b. AAA_ERR_FAILURE - client failed. 
            AAA_LOG(LM_INFO, "(%P|%t) **** Answer message message received in server ****\n");
            AAA_MsgDump::Dump(msg);
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode ErrorMsg(AAAMessage &msg) {
            // all error messages are handled by this function.
            AAA_LOG(LM_INFO, "(%P|%t) **** Received message with error bit set ****\n");
            return (AAA_ERR_SUCCESS);
	}
        virtual AAAReturnCode Success() {
            // notification of successful auth
            AAA_LOG(LM_INFO, "(%P|%t) **** user authorized ****\n");
            if (m_EndOnSuccess) {
                End();
	    }
            else {
                // test server re-auth
                // ReAuth(AAA_SESSION_AUTHORIZE_AUTHENTICATE);
	    }
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode Disconnect() {
            // notification of completed STR/STA exchange
            AAA_LOG(LM_INFO, "(%P|%t) **** session disconnecting ****\n");
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode SessionTimeout() {
            // notification of session timeout
            AAA_LOG(LM_INFO, "(%P|%t) **** session timeout ****\n");
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AuthorizationTimeout() {
            // notification of auth lifetime timeout
            AAA_LOG(LM_INFO, "(%P|%t) **** auth timeout ****\n");
            return (AAA_ERR_SUCCESS);
        }
        virtual AAAReturnCode AbortSession() {
            // notification of completed ASR/ASA exchange
            AAA_LOG(LM_INFO, "(%P|%t) **** session aborted by server ****\n");
            return (AAA_ERR_SUCCESS);
        }
        AAAReturnCode TxAuthenticationAnswer() {
            std::cout << "Sending answer message" << std::endl;

            AAA_MsgWidget msg(300, false, 10000);

            AAA_UInt32AvpWidget authIdAvp(AAA_AVPNAME_AUTHAPPID);
            AAA_Utf8AvpWidget unameAvp(AAA_AVPNAME_USERNAME);

            authIdAvp.Get() = 10000; // my application id
            unameAvp.Get() = "username@domain.com";

            msg()->acl.add(authIdAvp());
            msg()->acl.add(unameAvp());

            AAA_MsgResultCode rcode(*msg());
            rcode.ResultCode(AAA_SUCCESS);

            Send(msg());
            return (AAA_ERR_SUCCESS);
        }
    private:
        bool m_EndOnSuccess;
};

// Server session factory. Unlike AAA clients, server
// sessions need to be created on demand. This factory
// is responsible for creating new server sessions
// based on incomming new request.
typedef AAA_ServerSessionAllocator<AAA_SampleAuthServer> 
        SampleAuthServerAllocator;
typedef AAA_ServerSessionAllocator<AAA_SampleAcctServer> 
        SampleAcctServerAllocator;

int main(int argc, char *argv[])
{
   AAA_Task task;
   task.Start(5);

   // Application core is responsible for providing
   // peer connectivity between AAA entities
   AAA_Application appCore(task, "config/isp.local.xml");

   SampleAuthServerAllocator auth(task, 10000);
   SampleAcctServerAllocator acct(task, 20000);

   appCore.RegisterServerSessionFactory(auth);
   appCore.RegisterServerSessionFactory(acct);

   while (true) {
      std::cout << "Just wait here and let factory take care of new sessions" << std::endl;
      ACE_OS::sleep(10);
   }

   appCore.Close();
   task.Stop();
   return (0);
}





