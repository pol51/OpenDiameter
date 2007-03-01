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

#ifndef __AAA_LOG_FACILITY_H__
#define __AAA_LOG_FACILITY_H__

#include "aaa_data_defs.h"

class DiameterLogFacility 
{
   public:
      DiameterLogFacility() {
      }
      DiameterLogFacility(DiameterDataLog &data) {
          Open(data);
      }
      static inline void Open(DiameterDataLog &data) {
          ACE_TCHAR pname[32];
          AAALogMsg *logger = AAALogMsg_S::instance();

          u_long mask = logger->priority_mask(ACE_Log_Msg::PROCESS);
          mask &= ~LM_DEBUG;
          mask &= ~LM_TRACE;
          mask &= ~LM_INFO;
          
          mask |= (data.flags.debug) ? LM_DEBUG : 0;
          mask |= (data.flags.trace) ? LM_TRACE : 0;
          mask |= (data.flags.info)  ? LM_INFO  : 0;
          logger->priority_mask(mask, ACE_Log_Msg::PROCESS);
          
          u_long flags = logger->flags();
          flags &= ~ACE_Log_Msg::STDERR;
          flags &= ~ACE_Log_Msg::SYSLOG;
          
          flags |= (data.targets.console) ? ACE_Log_Msg::STDERR : 0;
          flags |= (data.targets.syslog) ? ACE_Log_Msg::SYSLOG : 0;
          ACE_OS::sprintf(pname, "Open Diameter %d.%d.%d\n", 
                   DIAMETER_VERSION_MAJOR,
                   DIAMETER_VERSION_MINOR,
                   DIAMETER_VERSION_MICRO);
          logger->open(pname, flags);
      }
      static inline void Close() {
      }
};

#endif /* __AAA_LOG_FACILITY_H__ */




