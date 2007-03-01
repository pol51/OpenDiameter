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

#ifndef __PANA_CLIENT_H__
#define __PANA_CLIENT_H__

#include "pana_exports.h"
#include "pana_session.h"

class PANA_EXPORT PANA_ClientEventInterface :
   public PANA_SessionEventInterface
{
   public:
      virtual void EapRequest(AAAMessageBlock *request) = 0;
};

class PANA_EXPORT PANA_Client :
   public PANA_Session
{
   public:
      PANA_Client(PANA_SessionTxInterface &tp,
                  PANA_SessionTimerInterface &tm,
                  PANA_ClientEventInterface &ev);

      virtual void NotifyEapRestart();
      virtual void NotifyAuthorization();
      virtual void NotifyEapRequest(pana_octetstring_t &payload);

      virtual void TxPCI();
      virtual void TxPSA(bool eapOptimization);
      virtual void TxPAR();
      virtual void TxPAN(bool eapPiggyBack);
      virtual void TxPBA(bool authSuccess);
      virtual void TxPRR();

      virtual void RxPSR();
      virtual void RxPAR();
      virtual void RxPAN();
      virtual void RxPBR();
      virtual void RxPRA();

      PANA_SessionTimerInterface &Timer() {
          return m_Timer;
      }

   private:
      virtual void TxPrepareMessage(PANA_Message &msg);
};

#endif /* __PANA_CLIENT_H__ */

