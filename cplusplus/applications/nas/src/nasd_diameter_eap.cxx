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

#include "nasd_diameter_eap.h"
#include "diameter_eap_client_session.hxx"

static AAAApplicationCore NASD_DiameterCore;

class NASD_DiameterEapClientSession :
   public DiameterEapClientSession
{
   public:
      NASD_DiameterEapClientSession(AAAApplicationCore &c, 
                                    DiameterJobHandle &h,
                                    NASD_CallNode &n) :
           DiameterEapClientSession(c, h),
           m_Node(n) {
      }

   public:
      void Abort() {
      }
      void SignalContinue(std::string &eapMsg) {
          AAAMessageBlock *msg = AAAMessageBlock::Acquire
                  ((char*)eapMsg.data(), (ACE_UINT32)eapMsg.length());
          m_Node.SendEgress(*msg);
          msg->Release();
      }
      void SignalSuccess(std::string &eapMsg) {
          if (eapMsg.length() > 0) {
              AAAMessageBlock *msg = AAAMessageBlock::Acquire
                      ((char*)eapMsg.data(), (ACE_UINT32)eapMsg.length());
              m_Node.PrevNode()->Success(msg);              
              msg->Release();
          }
          else {
              m_Node.PrevNode()->Success();
          }
      }
      void SignalFailure(std::string &eapMsg) {
          if (eapMsg.length() > 0) {
              AAAMessageBlock *msg = AAAMessageBlock::Acquire
                      ((char*)eapMsg.data(), (ACE_UINT32)eapMsg.length());
              m_Node.PrevNode()->Failure(msg);              
              msg->Release();
          }
          else {
              m_Node.PrevNode()->Failure();
          }
      }
      void SignalReauthentication() {
          m_Node.PrevNode()->Stop();
          m_Node.PrevNode()->Start();
      }      
      void SignalDisconnect() {
          m_Node.PrevNode()->Stop();
      }
      void SetDestinationRealm
      (DiameterScholarAttribute<diameter_utf8string_t> &realm) {
	  std::string ident;
          if (m_Node.PrevNode()->Identity(ident)) {
	      int pos = ident.find('@');
              if (pos > 0) {
		  realm.Set(ident.substr(pos + 1));
	      }
	  }
      }
      void SetUserName
      (DiameterScholarAttribute<diameter_utf8string_t> &username) {
	  std::string ident;
          if (m_Node.PrevNode()->Identity(ident)) {
	      int pos = ident.find('@');
              if (pos > 0) {
		  std::string uname;
                  uname.assign(ident, 0, pos - 1);
		  username.Set(uname);
	      }
              else {
		  username.Set(ident);
	      }
	  }
      }
      
   private:
      NASD_CallNode &m_Node;
};
  
class NASD_DiameterEap : 
   public NASD_CallNode
{
   public:
      NASD_DiameterEap(AAAApplicationCore &c) : 
         m_JobHandle(AAA_GroupedJob::Create
                      (c.GetTask().Job(), 
                        this, "diameter-eap")),
         m_AAAClient(new NASD_DiameterEapClientSession(c, m_JobHandle, *this)) {
      }
      virtual ~NASD_DiameterEap() {
	 m_AAAClient->Stop();
         m_JobHandle.reset();
      }
      
      /// Node events
   public:  
      int Start() {
         m_AAAClient->Start();
         return (0);
      }
      bool IsRunning() {
         return false;
      }
      void Stop() {
         m_AAAClient->Stop();
      }
      bool CurrentKey(std::string &key) {
         return false; // don't generate keys
      }
      bool Identity(std::string &ident) {
	 return false; // don't support
      }
      int ReceiveIngress(AAAMessageBlock &msg) {
         std::string eapMsg;
         eapMsg.assign(msg.base(), msg.size());
         m_AAAClient->ForwardResponse(eapMsg);
         return (0);
      }
      int ReceiveEgress(AAAMessageBlock &msg) {
         return (0); // diameter eap has no next node
      }
      void Success(AAAMessageBlock *msg = 0) {
      }
      void Failure(AAAMessageBlock *msg = 0) {
      }
      void Timeout() {
      }
      void Error() {
      }
      std::string CreationPolicy() {
         return std::string("aaa-diameter");
      }
         
   private:
      DiameterJobHandle m_JobHandle;
      std::auto_ptr<NASD_DiameterEapClientSession> m_AAAClient;
};

bool NASD_DiameterEapInitializer::Initialize(AAA_Task &t)
{
     NASD_LOG(LM_INFO, "(%P|%t) Initializing Diameter-EAP AAA protocol\n");

     std::string name("diameter_eap");
     NASD_AaaDiameterEapData *aaa = (NASD_AaaDiameterEapData*)
         NASD_AAAPROTO_TBL().Lookup(name);
     if (aaa == NULL) {
         NASD_LOG(LM_ERROR, "(%P|%t) Diameter-EAP configuration entry not found\n");
         return false;
     }

     return (NASD_DiameterCore.Open
             ((char*)aaa->Protocol().DiameterCfgFile().data(), t) 
	     != AAA_ERR_SUCCESS) ? false : true;
}

NASD_CallElement *NASD_DiameterEapInitializer::Create(AAA_Task &t)
{
     return (new NASD_DiameterEap(NASD_DiameterCore));
}

bool NASD_DiameterEapInitializer::UnInitialize()
{
     NASD_DiameterCore.Close();
     return true;
}



