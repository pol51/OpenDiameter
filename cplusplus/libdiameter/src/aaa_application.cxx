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

#include "aaa_xml_data.h"
#include "aaa_application.h"
#include "aaa_log_facility.h"

AAAReturnCode AAA_Application::Open(char *cfgfile)
{
    /// sanity check
    if (! m_Task.Running()) {
        AAA_LOG(LM_ERROR, "(%P|%t) Task MUST be running before openning core\n");
        return (AAA_ERR_FAILURE);
    }

    AAA_LOG(LM_INFO, "(%P|%t) Starting diameter core\n");

    /// parse config filename
    try {
        AAA_XMLConfigParser parser;
        parser.Load(m_Task, cfgfile);
    }
    catch (...) {
        AAA_LOG(LM_INFO, "(%P|%t) % failed to parse %s\n",
                cfgfile);
        return (AAA_ERR_PARSING_ERROR);
    }

    /// setup configure logging
    AAA_LogFacility::Open(*AAA_CFG_LOG());

    /// initialize origin state
    AAA_CFG_RUNTIME()->originStateId = time(0);

    /// initialize dictionary 
    AAADictionaryManager dm;
    dm.init((char*)AAA_CFG_PARSER()->dictionary.c_str());

    /// make sure we have a reactor
    do {
       ACE_Time_Value tm(0, 100);
       ACE_OS::sleep(tm);
    } while (! m_Task.reactor());

    /// initialize garbage collectors
    AAA_AUTH_SESSION_GC_ROOT()->Initialize(m_Task);
    AAA_ACCT_SESSION_GC_ROOT()->Initialize(m_Task);

    /// start contacting peers
    int ports[AAA_PEER_TTYPE_MAX] = { AAA_CFG_TRANSPORT()->tls_port,
                                      AAA_CFG_TRANSPORT()->tcp_port };
    m_PeerAcceptor.Start(ports);
    AAA_PeerConnector::Start();

    /// start re-transmission timer
    ACE_Time_Value interval
             (AAA_CFG_TRANSPORT()->retx_interval/AAA_ROUTER_RETX_DIVISOR, 0);
    m_ReTxHandler.Handle() = m_Task.ScheduleTimer
             (&m_ReTxHandler, 0, interval, interval);
                  
    return (AAA_ERR_SUCCESS);
}

AAAReturnCode AAA_Application::Close()
{
    AAA_LOG(LM_INFO, "(%P|%t) Stopping diameter core\n");

    /// cancel re-transmission timer
    m_Task.CancelTimer(m_ReTxHandler.Handle(), 0);
            
    /// disconnect from peers
    m_PeerAcceptor.Stop();
    AAA_PeerConnector::Stop(AAA_DISCONNECT_REBOOTING);
           
    /// close logging facility
    AAA_LogFacility::Close();

    return (AAA_ERR_SUCCESS);
}
