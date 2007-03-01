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

// $Id: client_pana_eap-tls.cxx,v 1.6 2006/04/26 11:06:50 canelaman Exp $ 
// A test program for EAP API.
// Written by Victor Fajardo

#include <iostream>
#include <ace/OS.h>
#include <ace/Signal.h>
#include <ace/Event_Handler.h>
#include <ace/Get_Opt.h>
#include <boost/shared_ptr.hpp>
#include "eap.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_identity.hxx"
#include "eap_policy.hxx"
#include "eap_method_registrar.hxx"
#include "eap_fsm.hxx"
#include "eap_log.hxx"
// Modified by Santiago Zapata Hernández for UMU
// Old
//#include "eap_md5.hxx"
// New
#include "eap_tls_fsm.hxx"
// End New
#include "pana_client_fsm.h"
#include "pana_paa_factory.h"
#include "user_db.h"

// Modified by Santiago Zapata Hernandez for UMU
// New
void print_cadena(ACE_Byte *cad, ACE_INT32 length)
{
        EAP_LOG(LM_DEBUG,"LENGTH cad %d\n",length);
        for (int i=0; i < length ; i++)
        {
          EAP_LOG(LM_DEBUG," %02x ",(ACE_Byte)(cad[i]));
        }
          EAP_LOG(LM_DEBUG,"\n");
}
// End New


class MyPeerSwitchStateMachine;
class PeerData;

/// Task class used in this sample program.
class EapTask : public AAA_Task
{
 public:
  /// Constructor.
  EapTask() : AAA_Task(AAA_SCHED_FIFO, "EAP") {}

  /// Destructor.
  ~EapTask() {}
};

// Class definition for peer EAP-TLS method for my application
class MyEapPeerTlsStateMachine
	: public EapPeerTlsStateMachine
{
	friend class EapMethodStateMachineCreator<MyEapPeerTlsStateMachine>;
public:
	MyEapPeerTlsStateMachine(EapSwitchStateMachine &s)
		: EapPeerTlsStateMachine(s) {}

  /// This pure virtual function is a callback used when an AuthID
  /// needs to be obtained.
  std::string& InputIdentity()
  {
    std::cout << "Received an Tls-Request "<< std::endl;
      
    static std::string identity;
    std::cout << "Input username (within 10sec.): " << std::endl;
    std::cin >> identity;
    std::cout << "username = " << identity << std::endl;
    return identity;
  }
  
protected:
  std::string& InputConfigFile()
  {
    std::cout << "Received an Tls-Request "<< std::endl;
      
//    static std::string configFile;
//    std::cout << "Input config filename (within 10sec.): " << std::endl;
//    std::cin >> configFile;
//    std::cout << "Config file name = " << configFile << std::endl;
static std::string configFile("./config/client.eap-tls.xml");
    return configFile;
  }

private:
	~MyEapPeerTlsStateMachine() {}
};



class MyPeerSwitchStateMachine: public EapPeerSwitchStateMachine
{
 public:

  MyPeerSwitchStateMachine(ACE_Reactor &r, EapJobHandle& h) 
      : EapPeerSwitchStateMachine(r, h) {}

  void Send(AAAMessageBlock *b);

  void Success();

  void Failure();

  void Notification(std::string &str);

  void Abort();

  std::string& InputIdentity();

 private:
  std::string identity;
};

class PeerApplication : public AAA_JobData,
                        public PANA_ClientEventInterface
{
 public:
  PeerApplication(PANA_Node &n, ACE_Semaphore &sem) : 
    pacSession(n, *this),
    handle(EapJobHandle(AAA_GroupedJob::Create(n.Task().Job(), this, "peer"))),
    eap(boost::shared_ptr<MyPeerSwitchStateMachine>
	(new MyPeerSwitchStateMachine(*n.Task().reactor(), handle))),
    semaphore(sem),
// Modified by Santiago Zapata Hernández for UMU
// Old
//    md5Method(EapContinuedPolicyElement(EapType(4)))
// New
methodTls(EapContinuedPolicyElement(EapType(TLS_METHOD_TYPE)))
// End New
  {
// Modified by Santiago Zapata Hernández for UMU
// Old
//    eap->Policy().InitialPolicyElement(&md5Method);
// New
    eap->Policy().InitialPolicyElement(&methodTls);
// End New
  }
  virtual ~PeerApplication() { }

  MyPeerSwitchStateMachine& Eap() { return *eap; }

  ACE_Semaphore& Semaphore() { return semaphore; }

  void EapStart(bool &nap) {
     eap->Stop();
     eap->Start();
  }
  void ChooseISP(const PANA_CfgProviderList &list,
                 PANA_CfgProviderInfo *&choice) {
  }
  virtual bool ResumeSession() {
     return false;
  }
  void EapRequest(AAAMessageBlock *request,
                  bool nap) {
     eap->Receive(request);
  }
  void EapRequest(AAAMessageBlock *request,
                  ACE_UINT32 resultCode,
                  ACE_UINT32 pcap,
                  PANA_DeviceIdContainer &epId) {
     eap->Receive(request);

     PANA_DeviceIdIterator i = epId.begin();
     for (;i != epId.end(); i++) {
          PANA_DeviceId *id = (*i);
          
          char buf[256];
          ACE_INET_Addr addr;
          PANA_DeviceIdConverter::FormatToAddr(*id, addr);
          addr.addr_to_string(buf, sizeof(buf));
          
          std::cout << "EP device id: " << id->type;
          std::cout << " value: ";
          std::cout << buf << std::endl;
     }
  }
    void EapAltReject(void) {
    }
    void Notification(diameter_octetstring_t &) {
    }
  bool IsKeyAvailable(diameter_octetstring_t &key) {
     if (eap->KeyAvailable()) {
         std::cout << "Assigning key" << std::endl;
         key.assign(eap->KeyData().data(), eap->KeyData().size());
         return true;
     }
     return false;
  }
  void Authorize(PANA_AuthorizationArgs &args) {
     char buf[256];
     ACE_INET_Addr addr;
     PANA_DeviceId *id;

     if (args.m_Pac.IsSet()) {        
        PANA_DeviceIdConverter::FormatToAddr(args.m_Pac(), addr);
        addr.addr_to_string(buf, sizeof(buf));
        std::cout << "PaC device id: " << args.m_Pac().type;
        std::cout << " value: ";
        std::cout << buf << std::endl;
     }

     if (args.m_Paa.IsSet()) {        
        PANA_DeviceIdConverter::FormatToAddr(args.m_Paa(), addr);
        addr.addr_to_string(buf, sizeof(buf));
        std::cout << "PAA device id: " << args.m_Paa().type;
        std::cout << " value: ";
        std::cout << buf << std::endl;
     }

     if (args.m_Key.IsSet()) {
        std::cout << "Key: ";
        for (int i=0; i<args.m_Key().size();i++) {
            printf("0x%02X ", (unsigned char)args.m_Key().data()[i]);
            if ((i+1) % 16 == 0) std::cout << std::endl;
	}
        std::cout << std::endl;
     }

     if (args.m_KeyId.IsSet()) {
        std::cout << "Key Id: " << args.m_KeyId();
        std::cout << std::endl;
     }

     if (args.m_Lifetime.IsSet()) {
        std::cout << "Lifetime: " << args.m_Lifetime();
        std::cout << std::endl;
     }

     if (args.m_ProtectionCapability.IsSet()) {
        std::cout << "Capability: " << args.m_ProtectionCapability();
        std::cout << std::endl;
     }

     if (args.m_Ep.IsSet()) {
        PANA_DeviceIdIterator i = args.m_Ep()->begin();
        for (;i != args.m_Ep()->end(); i++) {
             id = (*i);
          
             PANA_DeviceIdConverter::FormatToAddr(*id, addr);
             addr.addr_to_string(buf, sizeof(buf));
          
             std::cout << "EP device id: " << id->type;
             std::cout << " value: ";
             std::cout << buf << std::endl;
        }
     }
  }
  void Disconnect(ACE_UINT32 cause) {
      eap->Stop();
      pacSession.Abort();
      semaphore.release();
  }
  void Error(ACE_UINT32 resultCode) { 
  }
  PANA_PacSession &pac() { return pacSession; }

 private:
  PANA_PacSession pacSession;
  EapJobHandle handle;
  boost::shared_ptr<MyPeerSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
// Modified by Santiago Zapata Hernández for UMU
// Old
//  EapContinuedPolicyElement md5Method;
// New
EapContinuedPolicyElement methodTls;
// End New
};

// ----------------- Definition --------------
void MyPeerSwitchStateMachine::Send(AAAMessageBlock *b)
{
  std::cout << "EAP Response sent from peer" << std::endl;
  JobData(Type2Type<PeerApplication>()).pac().EapSendResponse(b);
}

void MyPeerSwitchStateMachine::Success()
  {
    std::cout << "Authentication success detected at peer" << std::endl;
    std::cout << "Welcome to the world, " 
	      << PeerIdentity() 
	      << " !!!" << std::endl;

// Modified by Santiago Zapata Hernández for UMU
// New
    EAPTLS_session_t_peer* session_peer= ((MyEapPeerTlsStateMachine&)MethodStateMachine()).get_tls_session();
    AAAMessageBlock *skey_peer=EAPTLSCrypto_callbacks::eaptls_gen_mppe_keys(session_peer->get_master_key(),
                                                                            session_peer->get_client_random(),
                                                                            session_peer->get_server_random());
    EAP_LOG(LM_DEBUG,"Peer TLS session key\n");
    print_cadena((ACE_Byte *)skey_peer->base(),skey_peer->length());
// End New
	      
    // Let PANA bind on success to the EAP event
// Modified by Santiago Zapata Hernandez for UMU
// New
//    std::string key (skey_peer->base (), skey_peer->length());
//    JobData(Type2Type<PeerApplication>()).pac().(key);
// End New
  }
void MyPeerSwitchStateMachine::Failure()
  {
    std::cout << "Authentication failure detected at peer" << std::endl;
    std::cout << "Sorry, " 
	      << PeerIdentity() 
	      << " try next time !!!" << std::endl;
    JobData(Type2Type<PeerApplication>()).pac().EapFailure();
    JobData(Type2Type<PeerApplication>()).Eap().Stop();
    JobData(Type2Type<PeerApplication>()).Semaphore().release();
  }
void MyPeerSwitchStateMachine::Notification(std::string &str)
  {
    std::cout << "Following notification received" << std::endl;
    std::cout << str << std::endl;
  }
void MyPeerSwitchStateMachine::Abort()
  {
    std::cout << "Peer aborted for an error in state machine" << std::endl;
  JobData(Type2Type<PeerApplication>()).Eap().Stop();
  JobData(Type2Type<PeerApplication>()).pac().Abort();
    JobData(Type2Type<PeerApplication>()).Semaphore().release();
  }
std::string& MyPeerSwitchStateMachine::InputIdentity() 
  {
    identity = std::string("ohba");
    std::cout << "Input username (within 10sec.): " << std::endl;
    std::cin >> identity;
    std::cout << "username = " << identity << std::endl;
    return identity;
  }

int main(int argc, char **argv)
{
  std::string cfgfile;

  // Gather command line options
  ACE_Get_Opt opt(argc, argv, "f:", 1);
    
  for (int c; (c = opt()) != (-1); ) {
      switch (c) {
          case 'f': cfgfile.assign(opt.optarg); break;
      }
  }

  if ((opt.argc() < 1) ||
      (cfgfile.length() == 0)) {
    std::cout << "Usage: pana_test -f <configuration file>" << std::endl;
    return (0);
  }

  // Initialize the log.
#ifdef WIN32
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::STDERR);
#else
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::STDERR);
#endif
  EapLogMsg_S::instance()->enable_debug_messages();

  // Register the mapping from EapType to the creator of the
  // user-defined method class for each user-defined method
  // implementation.

  EapMethodStateMachineCreator<MyEapPeerTlsStateMachine> 
    myPeerTlsCreator;

  EapMethodRegistrar methodRegistrar;

  methodRegistrar.registerMethod
    (std::string("TLS"), EapType(TLS_METHOD_TYPE), 
     Peer, myPeerTlsCreator);

  EapTask task;
  ACE_Semaphore semaphore(0);

  task.Start(5);
  
  try {
          PANA_Node node(task, cfgfile);
          PeerApplication peer(node, semaphore);
          peer.pac().Start();
          semaphore.acquire();
          task.Stop();
  }
  catch (PANA_Exception &e) {
      std::cout << "PANA exception: " << e.description() << std::endl;
  }
  catch (...) {
      std::cout << "Unknown exception ... aborting" << std::endl;
  }

  return 0;
}
