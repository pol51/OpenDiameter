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

#ifndef __PANA_PAA_H__
#define __PANA_PAA_H__

#include "pana_exports.h"
#include "pana_session.h"
#include "pana_provider_info.h"

// Hard values (values specific to this implementation)
#define PANA_CARRY_PCAP_IN_PSR   false
#define PANA_CARRY_PCAP_IN_PBR   true

typedef union {
    struct {
        ACE_UINT32 CarryPcapInPSR   :  1;
        ACE_UINT32 CarryPcapInPBR   :  1;
        ACE_UINT32 Reserved         : 30;
    } i;
    ACE_UINT32 p;
} PANA_PaaSupportFlags;

class PANA_EXPORT PANA_PaaEventInterface : public PANA_SessionEventInterface
{
   public:
      virtual bool IsUserAuthorized() = 0;
      virtual void EapResponse(AAAMessageBlock *request, bool nap) = 0;
#if defined(PANA_MPA_SUPPORT)
      virtual bool IsPacIpAddressAvailable(PANA_DeviceId &ip,
                                           PANA_DeviceId &local,
                                           ACE_INET_Addr &remote) = 0;
#endif
};

class PANA_EXPORT PANA_Paa : public PANA_Session
{
   public:
       typedef enum {
           EAP_SUCCESS,
           EAP_FAILURE,
           EAP_TIMEOUT
       } EAP_EVENT;

   public:
      PANA_Paa(PANA_SessionTxInterface &tp,
               PANA_SessionTimerInterface &tm,
               PANA_PaaEventInterface &ev);
      virtual ~PANA_Paa() { 
      }

      virtual void NotifyAuthorization();
      virtual void NotifyEapRestart();
      virtual void NotifyEapResponse(pana_octetstring_t &payload);
      virtual void NotifyEapTimeout();
      virtual void NotifyEapReAuth();

      virtual bool IsUserAuthorized();

      virtual void TxPSR();
      virtual void TxPAR();
      virtual void TxPBR(pana_unsigned32_t rcode,
                         EAP_EVENT ev);
      virtual void TxPFER(pana_unsigned32_t rcode,
                          EAP_EVENT ev);
      virtual void TxPAN();
      virtual void TxPRAA();

      virtual void RxPSA();
      virtual void RxPBA(bool success);
      virtual void RxPFEA(bool success);
      virtual void RxPAR();
      virtual void RxPAN();
      virtual void RxPRAR();

      PANA_PaaSupportFlags &SupportFlags() {
          return m_Flags;
      }
      PANA_SessionTimerInterface &Timer() {
          return m_Timer;
      }

   protected:
      virtual void TxFormatAddress(PANA_Message &msg);

   private:
      PANA_PaaSupportFlags m_Flags;
};

#endif /* __PANA_PAA_H__ */

