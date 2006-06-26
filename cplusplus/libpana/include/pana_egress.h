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

#ifndef __PANA_EGRESS_H__
#define __PANA_EGRESS_H__

#include "pana_session.h"
#include "pana_io.h"
#include "framework.h"
#include "boost/shared_ptr.hpp"

class PANA_EXPORT PANA_EgressSender : public AAA_Job
{
    public:
        typedef enum {
            RETRY_COUNT = 3,
        };
    public:
        PANA_EgressSender(AAA_GroupedJob &g,
                          PANA_ResilientIO &io,
                          boost::shared_ptr<PANA_Message> msg) :
           m_IO(io),
           m_Group(g),
           m_Msg(msg) {
        }
        virtual bool ExistBacklog() { 
           return (false); 
        }
        virtual int Schedule(AAA_Job*job, size_t backlogSize=1) { 
           // destination port check
           if (m_Msg->srcPort() <= 0) {
              throw (PANA_Exception(PANA_Exception::TRANSPORT_FAILED,
                    "Invalid destination port bounded to message"));
           }
           return m_Group.Schedule(job);
        }

    protected:
        int PANA_EgressSender::Serve();
    
    protected:
        PANA_ResilientIO &m_IO;
        AAA_GroupedJob &m_Group;
        boost::shared_ptr<PANA_Message> m_Msg;
};

#endif // __PANA_EGRESS_H__
