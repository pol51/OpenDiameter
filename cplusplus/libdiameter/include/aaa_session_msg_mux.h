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

#ifndef __AAA_SESSION_MSG_MUX_H__
#define __AAA_SESSION_MSG_MUX_H__

#include <map>
#include "diameter_parser.h"

template<class ARG>
class DiameterSessionMsgMuxHandler
{
    public:
        virtual ~DiameterSessionMsgMuxHandler() {
	}
        /// This function is called when incomming request message is received
        virtual AAAReturnCode RequestMsg(ARG &arg, DiameterMsg &msg) = 0;

        /// This function is called when incomming answer message is received
        virtual AAAReturnCode AnswerMsg(ARG &arg, DiameterMsg &msg) = 0;

        /// This function is called when incomming error message is received
        virtual AAAReturnCode ErrorMsg(ARG &arg, DiameterMsg &msg) = 0;

    protected:
        DiameterSessionMsgMuxHandler() {
	}
};

template<class ARG>
class DiameterSessionMsgMux
{
    public:
        DiameterSessionMsgMux(ARG &arg) :
            m_Arg(arg) {
	}
        virtual ~DiameterSessionMsgMux() {
	}
        void Register(AAACommandCode code, DiameterSessionMsgMuxHandler<ARG> &handler) {
	    m_Map.insert(std::pair<AAACommandCode, 
                         DiameterSessionMsgMuxHandler<ARG>* >
                          (code, &handler));
	}
        void Remove(AAACommandCode code) {
	    typename std::map<AAACommandCode, 
		    DiameterSessionMsgMuxHandler<ARG>* >::iterator i = m_Map.find(code);
            if (i != m_Map.end()) {
		m_Map.erase(i);
	    }
	}
        AAAReturnCode Mux(DiameterMsg &msg) {
	    typename std::map<AAACommandCode, 
		    DiameterSessionMsgMuxHandler<ARG>* >::iterator i = m_Map.find(msg.hdr.code);
            if (i != m_Map.end()) {
                if (msg.hdr.flags.e) {
                    return i->second->ErrorMsg(m_Arg, msg);
                }
                else {
                    return (msg.hdr.flags.r) ? 
                            i->second->RequestMsg(m_Arg, msg) :
                            i->second->AnswerMsg(m_Arg, msg);
		}
	    }
            return (AAA_ERR_FAILURE);
	}

    private:
        ARG &m_Arg;
	std::map<AAACommandCode, 
		 DiameterSessionMsgMuxHandler<ARG>* > m_Map;
};

#endif


