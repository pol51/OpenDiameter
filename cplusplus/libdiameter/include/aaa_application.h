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

#ifndef __AAA_APPLICATION_H__
#define __AAA_APPLICATION_H__

#include "framework.h"
#include "aaa_session_server.h"
#include "aaa_session_msg_rx.h"
#include "aaa_peer_interface.h"

// An application identifies itself to the diameter class library by
// an instance of the application core. All other classes are services
// that operates using an instance of the core. 
class DIAMETERBASEPROTOCOL_EXPORT DiameterApplication :
    public AAA_JobData
{
    private: // General Timer hanlders
        class ReTransmissionTimerHandler : 
            public ACE_Event_Handler 
        {
            public:
               int handle_timeout(const ACE_Time_Value &tv, 
                                  const void *arg) {
                   DIAMETER_MSG_ROUTER()->ReTransmitEvent();
                   return (0);
               }
               long &Handle() {
                   return m_TimerHandle;
               }
            private:
               long m_TimerHandle;
        };

    public:
        DiameterApplication(AAA_Task &task,
                            char *cfgfile = NULL) :
            m_Task(task) {
            if (cfgfile) {
                Open(cfgfile);
            }
        }
        virtual ~DiameterApplication() {
            m_ReTxHandler.reactor(NULL);
        }

        // Config file loading/un-loading
        AAAReturnCode Open(char *cfgfile);
        AAAReturnCode Close();

        // Check for active peers
        int NumActivePeers() {
            return DiameterPeerConnector::GetNumOpenPeers();
        }

        // Factory Registration
        AAAReturnCode RegisterServerSessionFactory
            (DiameterServerSessionFactory &factory);

        // Factory de-registration
        AAAReturnCode RemoveServerSessionFactory(diameter_unsigned32_t appId);

        // Proxy Handler Registration
        AAAReturnCode RegisterProxyHandler(AAA_ProxyHandler &handler);

        // Proxy Handler De-Registration
        AAAReturnCode RemoveProxyHandler(diameter_unsigned32_t appId);

        // Framework task
        AAA_Task& Task() {
            return m_Task;
        }

    private: // Global Classes
        AAA_Task &m_Task;
        DiameterPeerAcceptor m_PeerAcceptor;
        DiameterSessionMsgRx m_SessionMsgRx;
        Diameter_IO_SigMask m_IOSigMask;
        ReTransmissionTimerHandler m_ReTxHandler;
};

#endif   // __AAA_APPLICATION_H__ 
