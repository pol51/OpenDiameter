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

#ifndef __PANA_UTL_H__
#define __PANA_UTL_H__

#include "ace/Synch.h"
#include "ace/Singleton.h"

/*
 * Auxilliary class for generating session id's
 */
class PANA_SessionIdGenerator
{
    public:
        void Seed(std::string &id) { m_Identity = id; }
        void Generate(std::string &newSessionId) {
           if (m_Identity.size() == 0) {
              char hostname[128];
              ACE_INET_Addr base;
              if (base.get_host_name(hostname, sizeof(hostname)) == 0) {
                 m_Identity = hostname;
              }
              else {
                 throw (PANA_Exception(PANA_Exception::SESSIONID_ERROR,
                                       "Unable to retrieve local hostname"));
              }
           }
           ACE_Time_Value tv = ACE_OS::gettimeofday();
           m_Incrementor ++;

           char id[256];
           ACE_OS::sprintf(id, "%s;%d;%d",
                           m_Identity.data(),
                           tv.sec(),
                           m_Incrementor);
           newSessionId = id;
	}

    private:
        std::string m_Identity;
        long m_Incrementor;
};

typedef ACE_Singleton<PANA_SessionIdGenerator, 
                      ACE_Null_Mutex> 
                      PANA_SessionIdGenerator_S;
#define PANA_SESSIONID_GENERATOR() (*PANA_SessionIdGenerator_S::instance())

#endif /* __PANA_UTL_H__ */

