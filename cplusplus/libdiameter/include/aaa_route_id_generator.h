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

#ifndef __ROUTE_ID_GENERATOR_H__
#define __ROUTE_ID_GENERATOR_H__

#include <time.h>
#include <stdlib.h>
#include "ace/Singleton.h"

class DiameterIdGenerator {
    public:
       virtual int Get() = 0;

    protected:
       DiameterIdGenerator() : m_Id(0) { }
       virtual ~DiameterIdGenerator() { }
       int m_Id;
};

class DiameterHopByHopGenerator : public DiameterIdGenerator {
    public:
       int Get() {
           if (m_Id == 0) {
#ifndef WIN32
               struct timeval tv;
               gettimeofday(&tv, 0);
               srand((unsigned int)(tv.tv_sec + tv.tv_usec));
#else
			   srand((unsigned int)time(0));
#endif
               m_Id = rand(); // random number seed
           }
           return (++m_Id);
       }
};

class DiameterEndToEndGenerator : public DiameterIdGenerator {
    public:
       int Get() {
           if (m_Id == 0) {
#ifndef WIN32
               struct timeval tv;
               gettimeofday(&tv, 0);
               srand((unsigned int)(tv.tv_sec + tv.tv_usec));
               m_Id = (unsigned int)(tv.tv_sec) << 20; // set high 12 bit
#else
			   srand((unsigned int)time(0));
               m_Id = (unsigned int)(rand()) << 20; // set high 12 bit
#endif
           }
           m_Id &= 0xFFF00000; // clear lower 20 bits
           m_Id |= rand() & 0x000FFFFF; // set low 20 bits
           return (m_Id);
       }
};

typedef ACE_Singleton<DiameterHopByHopGenerator, ACE_Recursive_Thread_Mutex>
                      DiameterHopByHopGenerator_S;
#define DIAMETER_HOPBYHOP_GEN() DiameterHopByHopGenerator_S::instance()

typedef ACE_Singleton<DiameterEndToEndGenerator, ACE_Recursive_Thread_Mutex>
                      DiameterEndToEndGenerator_S;
#define DIAMETER_ENDTOEND_GEN() DiameterEndToEndGenerator_S::instance()

#endif


