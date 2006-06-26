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

#ifndef __AAA_SESSION_SERVER_FACTORY_H__
#define __AAA_SESSION_SERVER_FACTORY_H__

#include "aaa_session.h"

class DIAMETERBASEPROTOCOL_EXPORT AAA_ServerSessionFactory 
{
    public:
       virtual ~AAA_ServerSessionFactory() { 
       }
       diameter_unsigned32_t GetApplicationId() { 
           return m_ApplicationId; 
       }

       // This function is implemented by derived class
       virtual AAA_SessionIO *CreateInstance() = 0;

    protected:
       AAA_ServerSessionFactory(AAA_Task &task,
                                diameter_unsigned32_t appId) :
           m_Task(task),
           m_ApplicationId(appId) { 
       }

    protected:
       AAA_Task &m_Task;
       diameter_unsigned32_t m_ApplicationId;
};

template<class SESSION_SERVER>
class AAA_ServerSessionAllocator : 
    public AAA_ServerSessionFactory
{
    public:
       AAA_ServerSessionAllocator(AAA_Task &task,
                                  diameter_unsigned32_t appId) : 
            AAA_ServerSessionFactory(task, appId) { 
       }
       AAA_SessionIO *CreateInstance() {
            return new SESSION_SERVER(m_Task, m_ApplicationId);
       }
};

#endif

