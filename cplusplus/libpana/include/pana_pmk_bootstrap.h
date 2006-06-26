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

#ifndef __PANA_PMK_BOOTSTRAP_H__
#define __PANA_PMK_BOOTSTRAP_H__

#include "pana_exports.h"
#include "diameter_parser_api.h"

/*
10.2.2  PANA with Bootstrapping WPA/IEEE 802.11i

   In this model, PANA is used for authentication and authorization, and
   L2 ciphering is used for access control, the latter is enabled by the
   former.  The L2 ciphering is based on using PSK (Pre-Shared Key) mode
   of WPA (Wi-Fi Protected Access) [WPA] or IEEE 802.11i [802.11i],
   which is derived from the EAP MSK as a result of successful PANA
   authentication.  In this document, the pre-shared key shared between
   station and AP is referred to as PMK (Pair-wise Master Key).  In this
   model, MAC address is used as the device identifier in PANA.

   This model allows the separation of PAA from APs (EPs).  A typical
   purpose of using this model is to reduce AP management cost by
   allowing physical separation of RADIUS/Diameter client from access
   points, where AP management can be a significant issue when deploying
   a large number of access points.

   By bootstrapping PSK mode of WPA and IEEE 802.11i from PANA it is
   also possible to improve wireless LAN security by providing protected
   disconnection procedure at L3.

   This model does not require any change in the current WPA and IEEE
   802.11i specifications.  This also means that PANA doesn't provide
   any L2 security features beyond those already provided for in WPA and
   IEEE 802.11i.

   The IEEE 802.11 specification [802.11] allows Class 1 data frames to
   be received in any state.  Also, the latest version of IEEE 802.11i
   [802.11i] optionally allows higher-layer data traffic to be received
   and processed on their IEEE 802.1X Uncontrolled Ports.  This feature
   allows processing IP-based traffic (such as ARP, IPv6 neighbor
   discovery, DHCP, and PANA) on IEEE 802.1X Uncontrolled Port prior to
   client authentication.  (Note: WPA does not explicitly define this
   operation, so it may be safer not to use this in WPA).

   Until the PaC is successfully authenticated, only a selected type of
   IP traffic is allowed over the IEEE 802.1X Uncontrolled Port.  Any
   other IP traffic is dropped on the AP without being forwarded to the
   DS (Distribution System).  Upon successful PANA authentication, the
   traffic switches to the controlled port.  Host configuration,
   including obtaining an (potentially new) IP address, takes place on
   this port.  Usual DHCP-based, and also in the case of IPv6 stateless
   autoconfiguration, mechanism is available to the PaC.  After this
   point, the rest of the IP traffic, including PANA exchanges, are
   processed on the controlled port.

   When a PaC does not have a PMK for the AP, the following procedure is
   taken:

   1.  The PaC associates with the AP.

   2.  The PaC configures a PRPA by using DHCP (in the case of IPv4) or
       configures a link-local address (in the case of IPv6), and then
       runs PANA by using the address.

   3.  Upon successful authentication, the PaC obtains a PMK for each AP
       controlled by the PAA.

   4.  The AP initiates IEEE 802.11i 4-way handshake to establish a PTK
       (Pair-wise Transient Key) with the PaC, by using the PMK.

   5.  The PaC obtains a POPA by using any method that the client
       normally uses.
*/

class PANA_EXPORT PANA_PMKKey
{
    public:
        PANA_PMKKey(diameter_octetstring_t &aaaKey,
                    diameter_octetstring_t &supplicantAddr,   
                    diameter_octetstring_t &authenticatorAddr,
                    size_t bit_length = 256) {
            Seed(aaaKey, supplicantAddr, authenticatorAddr, bit_length); 
        }
        virtual ~PANA_PMKKey() {
        }
        virtual diameter_octetstring_t &Key() {
            return m_Key;
        }

    protected:
        virtual void Seed(diameter_octetstring_t &aaaKey,
                    diameter_octetstring_t &supplicantAddr,   
                    diameter_octetstring_t &authenticatorAddr,
                    size_t bit_length);

    private:
        diameter_octetstring_t m_Key;
};

typedef std::list< diameter_octetstring_t > PANA_PMKKeyList;
typedef PANA_PMKKeyList::iterator PAMA_PMKKeyListIterator;

#endif /* __PANA_PMK_BOOTSTRAP_H__ */

