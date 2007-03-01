
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

#include "aaa_data_defs.h"
#include "aaa_route_table.h"
#include "aaa_peer_table.h"
#include "aaa_route_msg_router.h"
#include "resultcodes.h"

/*
  6.1.  Diameter Request Routing Overview

    A request is sent towards its final destination using a combination
    of the Destination-Realm and Destination-Host AVPs, in one of these
    three combinations:

    -  a request that is not able to be proxied (such as CER) MUST NOT
       contain either Destination-Realm or Destination-Host AVPs.

    -  a request that needs to be sent to a home server serving a
       specific realm, but not to a specific server (such as the first
       request of a series of round-trips), MUST contain a Destination-
       Realm AVP, but MUST NOT contain a Destination-Host AVP.

    -  otherwise, a request that needs to be sent to a specific home
       server among those serving a given realm, MUST contain both the
       Destination-Realm and Destination-Host AVPs. 

    The Destination-Host AVP is used as described above when the
    destination of the request is fixed, which includes: 

    -  Authentication requests that span multiple round trips

    -  A Diameter message that uses a security mechanism that makes use
       of a pre-established session key shared between the source and the
       final destination of the message.

    -  Server initiated messages that MUST be received by a specific
       Diameter client (e.g., access device), such as the Abort-Session-
       Request message, which is used to request that a particular user's
       session be terminated.

    Note that an agent can forward a request to a host described in the
    Destination-Host AVP only if the host in question is included in its
    peer table (see Section 2.7).  Otherwise, the request is routed based
    on the Destination-Realm only (see Sections 6.1.6).

    The Destination-Realm AVP MUST be present if the message is
    proxiable.  Request messages that may be forwarded by Diameter agents
    (proxies, redirects or relays) MUST also contain an Acct-
    Application-Id AVP, an Auth-Application-Id AVP or a Vendor-Specific-
    Application-Id AVP.  A message that MUST NOT be forwarded by Diameter
    agents (proxies, redirects or relays) MUST not include the
    Destination-Realm in its ABNF.  The value of the Destination-Realm
    AVP MAY be extracted from the User-Name AVP, or other application-
    specific methods.

    When a message is received, the message is processed in the following
    order:

    1. If the message is destined for the local host, the procedures
       listed in Section 6.1.4 are followed.

    2. If the message is intended for a Diameter peer with whom the local
       host is able to directly communicate, the procedures listed in
       Section 6.1.5 are followed.  This is known as Request Forwarding.

    3. The procedures listed in Section 6.1.6 are followed, which is
       known as Request Routing.

    4. If none of the above is successful, an answer is returned with the
       Result-Code set to AAA_UNABLE_TO_DELIVER, with the E-bit set.

    For routing of Diameter messages to work within an administrative
    domain, all Diameter nodes within the realm MUST be peers.

    Note the processing rules contained in this section are intended to
    be used as general guidelines to Diameter developers.  Certain
    implementations MAY use different methods than the ones described
    here, and still comply with the protocol specification.  See Section
    7 for more detail on error handling.
*/

AAA_ROUTE_RESULT DiameterMsgRouter::RcLocal::Lookup(std::auto_ptr<DiameterMsg> &msg,
                                                    std::auto_ptr<DiameterMsg> &p,
                                                    DiameterPeerEntry *&dest)
{
    /*
       6.1.4.  Processing Local Requests

        A request is known to be for local consumption when one of the
        following conditions occur:

        -  The Destination-Host AVP contains the local host's identity,

        -  The Destination-Host AVP is not present, the Destination-Realm AVP
           contains a realm the server is configured to process locally, and
           the Diameter application is locally supported, or

        -  Both the Destination-Host and the Destination-Realm are not
           present.

        When a request is locally processed, the rules in Section 6.2 should
        be used to generate the corresponding answer.
    */
    DiameterIdentityAvpContainerWidget destHost(msg->acl);
    DiameterIdentityAvpContainerWidget destRealm(msg->acl);

    diameter_identity_t *DestHost = destHost.GetAvp(DIAMETER_AVPNAME_DESTHOST);
    diameter_identity_t *DestRealm = destRealm.GetAvp(DIAMETER_AVPNAME_DESTREALM);

    if (DestHost) {
        //
        // Warning: This is a case in-sensitive lookup which may not
        //          be generally appropriate if we consider FQDN as
        //          a non ascii value.
        //
        // Deprecated:
        // if (*DestHost == DIAMETER_CFG_TRANSPORT()->identity) {
        //
        if (! strcasecmp((*DestHost).c_str(),
                      DIAMETER_CFG_TRANSPORT()->identity.c_str())) {
            return (AAA_ROUTE_RESULT_SUCCESS);
        }

        AAA_LOG((LM_INFO, "(%P|%t) DestHost(%s) present but not ours so try realm routing\n", 
                DestHost->c_str()));
        // next chain
    }
    else if (DestRealm) {
        //
        // Warning: This is a case in-sensitive lookup which may not
        //          be generally appropriate if we consider FQDN as
        //          a non ascii value.
        //
        // Deprecated:
        // if (*DestRealm == DIAMETER_CFG_TRANSPORT()->realm) {
        //
        if (! strcasecmp((*DestRealm).c_str(),
                      DIAMETER_CFG_TRANSPORT()->realm.c_str())) {
            DiameterApplicationIdLst *idList[] = {
                &DIAMETER_CFG_GENERAL()->authAppIdLst,
                &DIAMETER_CFG_GENERAL()->acctAppIdLst
            };
            for (unsigned int i=0;
                 i<sizeof(idList)/sizeof(DiameterApplicationIdLst*);
                 i++) {
                DiameterApplicationIdLst::iterator x = idList[i]->begin();
                for (; x != idList[i]->end(); x++) {
                    if (*x == msg->hdr.appId) {
                        return (AAA_ROUTE_RESULT_SUCCESS);
                    }
                }
            }
            DiameterVendorSpecificIdLst::iterator y =
                DIAMETER_CFG_GENERAL()->vendorSpecificId.begin();
            for (; y != DIAMETER_CFG_GENERAL()->vendorSpecificId.end();
                 y++) {
                if (((*y).authAppId == msg->hdr.appId) ||
                    ((*y).acctAppId == msg->hdr.appId)) {
                    return (AAA_ROUTE_RESULT_SUCCESS);
                }
            }
        }

        AAA_LOG((LM_INFO, "(%P|%t) DestRealm(%s) present but not ours or no supported id\n", 
                DestRealm->c_str()));
        // next chain
    }
    else {
        return (AAA_ROUTE_RESULT_SUCCESS);
    }
    return (AAA_ROUTE_RESULT_NEXT_CHAIN);
}

AAA_ROUTE_RESULT DiameterMsgRouter::RcForwarded::Lookup(std::auto_ptr<DiameterMsg> &msg,
                                                        std::auto_ptr<DiameterMsg> &p,
                                                        DiameterPeerEntry *&dest)
{
    /*
       6.1.5.  Request Forwarding

        Request forwarding is done using the Diameter Peer Table.  The
        Diameter peer table contains all of the peers that the local node is
        able to directly communicate with.

        When a request is received, and the host encoded in the Destination-
        Host AVP is one that is present in the peer table, the message SHOULD
        be forwarded to the peer.
    */
    DiameterIdentityAvpContainerWidget destHost(msg->acl);
    diameter_identity_t *DestHost = destHost.GetAvp(DIAMETER_AVPNAME_DESTHOST);

    if (DestHost) {
        DiameterPeerEntry *peer = DIAMETER_PEER_TABLE()->Lookup(*DestHost);
        if (peer) {
            if (peer->IsOpen()) {
                dest = peer;
                return (AAA_ROUTE_RESULT_SUCCESS);
            }
            // try routing it
        }
        AAA_LOG((LM_INFO, "(%P|%t) DestHost(%s) does not match any peer\n", 
            DestHost->c_str()));
    }

    return (AAA_ROUTE_RESULT_NEXT_CHAIN);
}

AAA_ROUTE_RESULT DiameterMsgRouter::RcRouted::Lookup(std::auto_ptr<DiameterMsg> &msg,
                                                     std::auto_ptr<DiameterMsg> &p,
                                                     DiameterPeerEntry *&dest)
{
    /*
       6.1.6.  Request Routing

        Diameter request message routing is done via realms and applications.
        A Diameter message that may be forwarded by Diameter agents (proxies,
        redirects or relays) MUST include the target realm in the
        Destination-Realm AVP and one of the application identification AVPs
        Auth-Application-Id, Acct-Application-Id or Vendor-Specific-
        Application-Id.  The realm MAY be retrieved from the User-Name AVP,
        which is in the form of a Network Access Identifier (NAI).  The realm
        portion of the NAI is inserted in the Destination-Realm AVP.

        Diameter agents MAY have a list of locally supported realms and
        applications, and MAY have a list of externally supported realms and
        applications.  When a request is received that includes a realm
        and/or application that is not locally supported, the message is
        routed to the peer configured in the Realm Routing Table (see Section
        2.7).
    */
    try {
        diameter_identity_t DestRealm;
        DiameterIdentityAvpContainerWidget destRealm(msg->acl);
        diameter_identity_t *LookupRealm = destRealm.GetAvp(DIAMETER_AVPNAME_DESTREALM);

        if (! LookupRealm) {
            DiameterUtf8AvpContainerWidget username(msg->acl);
            diameter_utf8string_t *UserName = username.GetAvp(DIAMETER_AVPNAME_USERNAME);
            if (! UserName) {
                AAA_LOG((LM_INFO, "(%P|%t) Can't determine DestRealm during realm routing\n"));
                throw (0);
            }
            DestRealm = UserName->substr(UserName->find("@",0), UserName->length()-1);
        }
        else {
            DestRealm.assign(LookupRealm->data(), LookupRealm->length());
        }

        DiameterRouteEntry *route = DIAMETER_ROUTE_TABLE()->Lookup(DestRealm);
        if (route == NULL) {
            AAA_LOG((LM_INFO, "(%P|%t) DestRealm(%s) not in routing table\n",
                    DestRealm.c_str()));
            throw (0);
        }

        // look in the header first, base protocol only
        DiameterRouteApplication *app = route->Lookup(msg->hdr.appId, 0);
        if (app == NULL) {

            diameter_unsigned32_t *idPtr = NULL;
            char *avpNames[] = { DIAMETER_AVPNAME_AUTHAPPID,
                                 DIAMETER_AVPNAME_ACCTAPPID };
            DiameterUInt32AvpContainerWidget id(msg->acl);
            for (unsigned int i=0; i<sizeof(avpNames)/sizeof(char*); i++) {
                if ((idPtr = id.GetAvp(avpNames[i]))) {
                    break;
                }
            }

            if (idPtr) {
                app = route->Lookup(*idPtr, 0); // base protocol only
            }
            else {
                DiameterGroupedAvpContainerWidget vendorSpecificId(msg->acl);
                diameter_grouped_t *grouped = vendorSpecificId.GetAvp
                    (DIAMETER_AVPNAME_VENDORAPPID);
                if (grouped) {
                    DiameterUInt32AvpContainerWidget gVendorId(*grouped);
                    diameter_unsigned32_t *vid = gVendorId.GetAvp(DIAMETER_AVPNAME_VENDORID);
                    if (vid == NULL) {
                        AAA_LOG((LM_INFO, "(%P|%t) Vendor-Specific-Application-Id has no vendor ID AVP\n",
                                 DestRealm.c_str()));
                        return (AAA_ROUTE_RESULT_NEXT_CHAIN);
                    }

                    diameter_unsigned32_t *value = NULL;
                    char *avpNames[] = { DIAMETER_AVPNAME_AUTHAPPID,
                                         DIAMETER_AVPNAME_ACCTAPPID };
                    DiameterUInt32AvpContainerWidget gId(*grouped);
                    for (unsigned int i=0; i<sizeof(avpNames)/sizeof(char*); i++) {
                        if ((value = gId.GetAvp(avpNames[i]))) {
                           break;
                        }
                    }

                    if (value) {
                        app = route->Lookup(*value, *vid);
                    }
                }
            }
        }

        if (app) {
            // Local action is to redirect
            if (route->Action() == DIAMETER_ROUTE_ACTION_REDIRECT) {
                AAA_LOG((LM_INFO, "(%P|%t) DestRealm(%s) in routing table but it is set to re-direct\n",
                       DestRealm.c_str()));
                return (AAA_ROUTE_RESULT_SUCCESS);
            }

            // get the first available/open peer
            DiameterPeerEntry *peer = NULL;
            DiameterRouteServerEntry *server = app->Servers().First();
            while (server) {
                peer = DIAMETER_PEER_TABLE()->Lookup(server->Server());
                if (peer && peer->IsOpen()) {
                    dest = peer;
                    return (AAA_ROUTE_RESULT_SUCCESS);
                }
                server = app->Servers().Next(*server);
            }
            AAA_LOG((LM_INFO, "(%P|%t) DestRealm(%s) in routing table but no open peers support it\n",
                    DestRealm.c_str()));
        }
        else {
            AAA_LOG((LM_INFO, "(%P|%t) DestRealm(%s) in routing table but no matching app Id\n",
                    DestRealm.c_str()));
        }
    }
    catch (...) {
    }

    return (AAA_ROUTE_RESULT_NEXT_CHAIN);
}

AAA_ROUTE_RESULT DiameterMsgRouter::RcRejected::Lookup(std::auto_ptr<DiameterMsg> &msg,
                                                       std::auto_ptr<DiameterMsg> &p,
                                                       DiameterPeerEntry *&dest)
{
    /*
       6.1.  Diameter Request Routing Overview
                     .
                     .
                     .
         4. If none of the above is successful, an answer is returned with the
            Result-Code set to AAA_UNABLE_TO_DELIVER, with the E-bit set.
     */

    AAA_LOG((LM_INFO, "(%P|%t) Router cannot deliver message, sending back with an error\n"));
    return (AAA_ROUTE_RESULT_SUCCESS);
}

AAA_ROUTE_RESULT DiameterMsgRouter::DcLocal::Process(std::auto_ptr<DiameterMsg> msg,
                                                     std::auto_ptr<DiameterMsg> &p,
                                                     DiameterPeerEntry *source,
                                                     DiameterPeerEntry *dest)
{
    if (m_Arg.m_RedirectAgent.IsRedirected(msg)) {
        return (m_Arg.m_RedirectAgent.Answer(msg, p, source, dest) == 0) ?
            AAA_ROUTE_RESULT_SUCCESS: AAA_ROUTE_RESULT_FAILED;
    }

    if (msg->hdr.flags.e) {
        return ErrorHandling(msg, source, dest);
    }
    else if (dest) {
        return (dest->Send(msg) >= 0) ?
            AAA_ROUTE_RESULT_SUCCESS: AAA_ROUTE_RESULT_FAILED;
    }
    else {
        // original request must have been locally generated
        m_Arg[DiameterMsgRouterHandlerTable::H_LOCAL]->
            Answer(msg, source, dest);
    }
    return (AAA_ROUTE_RESULT_SUCCESS);
}

AAA_ROUTE_RESULT DiameterMsgRouter::DcLocal::ErrorHandling(std::auto_ptr<DiameterMsg> msg,
                                                           DiameterPeerEntry *source,
                                                           DiameterPeerEntry *dest)
{
    /*
      Result-Code AVP values that are used to report protocol errors MUST
      only be present in answer messages whose 'E' bit is set.  When a
      request message is received that causes a protocol error, an answer
      message is returned with the 'E' bit set, and the Result-Code AVP is
      set to the appropriate protocol error value.  As the answer is sent
      back towards the originator of the request, each proxy or relay agent
      MAY take action on the message.

                          1. Request        +---------+ Link Broken
                +-------------------------->|Diameter |----///----+
                |     +---------------------|         |           v
         +------+--+  | 2. answer + 'E' set | Relay 2 |     +--------+
         |Diameter |<-+ (Unable to Forward) +---------+     |Diameter|
         |         |                                        |  Home  |
         | Relay 1 |--+                     +---------+     | Server |
         +---------+  |   3. Request        |Diameter |     +--------+
                      +-------------------->|         |           ^
                                            | Relay 3 |-----------+
                                            +---------+

            Figure 7:  Example of Protocol Error causing answer message
 
      Figure 7 provides an example of a message forwarded upstream by a
      Diameter relay.  When the message is received by Relay 2, and it
      detects that it cannot forward the request to the home server, an
      answer message is returned with the 'E' bit set and the Result-Code
      AVP set to DIAMETER_UNABLE_TO_DELIVER.
    */

    if (dest) {
        return (dest->Send(msg) >= 0) ?
            AAA_ROUTE_RESULT_SUCCESS: AAA_ROUTE_RESULT_FAILED;
    }
    else {
        // original request must have been locally generated
        m_Arg[DiameterMsgRouterHandlerTable::H_ERROR]->
           Answer(msg, source, dest);
    }
    return (AAA_ROUTE_RESULT_SUCCESS);
}

int DiameterMsgRouter::DcLocal::RequestMsg(std::auto_ptr<DiameterMsg> msg,
                                           DiameterPeerEntry *source,
                                           DiameterPeerEntry *dest)
{
    if (source) {
        Add(msg->hdr.hh, msg, source, dest);
        m_Arg[DiameterMsgRouterHandlerTable::H_LOCAL]->
            Request(msg, source, dest);
    }
    else {
        AAA_LOG((LM_INFO, "(%P|%t) **** Looped back message, discarded ***\n"));
        DiameterMsgHeaderDump::Dump(*msg);
    }
    return (0);
}

AAA_ROUTE_RESULT DiameterMsgRouter::DcForward::Process(std::auto_ptr<DiameterMsg> msg,
                                                       std::auto_ptr<DiameterMsg> &p,
                                                       DiameterPeerEntry *source,
                                                       DiameterPeerEntry *dest)
{
    return m_Arg.m_rcLocal.Delivery().Process(msg, p, source, dest);
}

int DiameterMsgRouter::DcForward::LoopDetection(std::auto_ptr<DiameterMsg> &msg)
{
    /*
      A relay or proxy agent MUST check for forwarding loops when receiving
      requests.  A loop is detected if the server finds its own identity in
      a Route-Record AVP.  When such an event occurs, the agent MUST answer
      with the Result-Code AVP set to AAA_LOOP_DETECTED.
    */
    DiameterIdentityAvpContainerWidget rrecord(msg->acl);
    diameter_identity_t *rteRec = rrecord.GetAvp(DIAMETER_AVPNAME_ROUTERECORD);
    for (int p=1; rteRec; p++) {
        if (*rteRec == (DIAMETER_CFG_TRANSPORT()->identity)) {
            // send back answer
            AAA_LOG((LM_INFO, "(%P|%t) !!! WARNING !!!: Route record shows a loop in the message, sending back with error\n"));
            return (0);
        }
        rteRec = rrecord.GetAvp(DIAMETER_AVPNAME_ROUTERECORD, p);
    }
    return (-1);
}

int DiameterMsgRouter::DcForward::RequestMsg(std::auto_ptr<DiameterMsg> msg,
                                             DiameterPeerEntry *source,
                                             DiameterPeerEntry *dest)
{
    if (msg->hdr.flags.t && LookupQueuedMessage(msg->hdr.hh)) {
        // locally re-transmitted messages
        int rcode = (dest->Send(msg, false) >= 0) ? 0 : (-1);
        StoreRequestMessage(msg->hdr.hh, msg);
        return rcode;
    }
    else if (LoopDetection(msg) == 0) {
        if (source) {
            std::auto_ptr<DiameterMsg> answerMsg = DiameterErrorMsg::Generate(*msg, AAA_LOOP_DETECTED);
            return (source->Send(answerMsg) >= 0) ? 0 : (-1);
        }
        else {
            // request must have been locally generated
            m_Arg[DiameterMsgRouterHandlerTable::H_ERROR]->
                Request(msg, source, dest);
            return (-1);
        }
    }

    /*
       The Diameter protocol requires that agents maintain
       transaction state, which is used for failover purposes.
       Transaction state implies that upon forwarding a request,
       the Hop-by-Hop identifier is saved; the field is replaced
       with a locally unique identifier, which is restored to its
       original value when the corresponding answer is received.
       The request's state is released upon receipt of the answer.
       A stateless agent is one that only maintains transaction
       state.
     */
    int h2hIndex, rcode;
    if (source) {
        int localHH = DIAMETER_HOPBYHOP_GEN()->Get();
        h2hIndex = Add(localHH, msg, source, dest);
        msg->hdr.hh = localHH;
    }
    else {
        // locally generated
        if (msg->hdr.hh == 0) {
            msg->hdr.hh = DIAMETER_HOPBYHOP_GEN()->Get();
        }
        if (msg->hdr.ee == 0){
            msg->hdr.ee = DIAMETER_ENDTOEND_GEN()->Get();
        }
        h2hIndex = Add(msg->hdr.hh, msg, source, dest);
    }

    rcode = (dest->Send(msg, false) >= 0) ? 0 : (-1);
    if (h2hIndex > 0) {
        StoreRequestMessage(h2hIndex, msg);
    }
    return rcode;
}

AAA_ROUTE_RESULT DiameterMsgRouter::DcRouted::Process(std::auto_ptr<DiameterMsg> msg,
                                                      std::auto_ptr<DiameterMsg> &p,
                                                      DiameterPeerEntry *source,
                                                      DiameterPeerEntry *dest)
{
    /*
      6.2.2.  Relaying and Proxying Answers

       If the answer is for a request which was proxied or relayed, the
       agent MUST restore the original value of the Diameter header's Hop-
       by-Hop Identifier field.

       If the last Proxy-Info AVP in the message is targeted to the local
       Diameter server, the AVP MUST be removed before the answer is
       forwarded.

       If a relay or proxy agent receives an answer with a Result-Code AVP
       indicating a failure, it MUST NOT modify the contents of the AVP.
       Any additional local errors detected SHOULD be logged, but not
       reflected in the Result-Code AVP.  If the agent receives an answer
       message with a Result-Code AVP indicating success, and it wishes to
       modify the AVP to indicate an error, it MUST modify the Result-Code
       AVP to contain the appropriate error in the message destined towards
       the access device as well as include the Error-Reporting-Host AVP and
       it MUST issue an STR on behalf of the access device.

       The agent MUST then send the answer to the host that it received the
       original request from.
    */
    if (m_Arg.m_RedirectAgent.IsRedirected(msg)) {
        return (m_Arg.m_RedirectAgent.Answer(msg, p, source, dest) == 0) ?
            AAA_ROUTE_RESULT_SUCCESS: AAA_ROUTE_RESULT_FAILED;
    }

    AAA_ROUTE_RESULT result = AAA_ROUTE_RESULT_SUCCESS;
    if (! dest) {
        // original request must have been locally generated
        DiameterMsgRouterHandlerTable::H_TYPE type = (msg->hdr.flags.e) ? 
             DiameterMsgRouterHandlerTable::H_ERROR : DiameterMsgRouterHandlerTable::H_LOCAL;
        m_Arg[type]->Answer(msg, source, dest);
    }
    else {
        DiameterMsgRouterHandlerTable::H_TYPE type = (msg->hdr.flags.e) ? 
             DiameterMsgRouterHandlerTable::H_ERROR : DiameterMsgRouterHandlerTable::H_PROXY;
        m_Arg[type]->Answer(msg, source, dest);
        result = (dest->Send(msg) >= 0) ? AAA_ROUTE_RESULT_SUCCESS :
               AAA_ROUTE_RESULT_FAILED;
    }
    return result;
}

int DiameterMsgRouter::DcRouted::RequestMsg(std::auto_ptr<DiameterMsg> msg,
                                            DiameterPeerEntry *source,
                                            DiameterPeerEntry *dest)
{
    DiameterIdentityAvpContainerWidget originHost(msg->acl);
    DiameterIdentityAvpContainerWidget destRealm(msg->acl);
    diameter_identity_t *DestRealm = destRealm.GetAvp(DIAMETER_AVPNAME_DESTREALM);
    if (! DestRealm) {
        throw (0);
    }

    DiameterRouteEntry *route = DIAMETER_ROUTE_TABLE()->Lookup(*DestRealm);
    if (route == NULL) {
        throw (0);
    }

    if (route->Action() == DIAMETER_ROUTE_ACTION_LOCAL) {
        return m_Arg.m_rcLocal.Delivery().RequestMsg(msg, source, dest);
    }
    else if (route->Action() == DIAMETER_ROUTE_ACTION_REDIRECT) {
        return m_Arg.m_RedirectAgent.Request(msg, source, dest);
    }
    else {
        /*
          6.1.8.  Relaying and Proxying Requests

           A relay or proxy agent MUST append a Route-Record AVP to all requests
           forwarded.  The AVP contains the identity of the peer the request was
           received from.

           The Hop-by-Hop identifier in the request is saved, and replaced with
           a locally unique value.  The source of the request is also saved,
           which includes the IP address, port and protocol.

           A relay or proxy agent MAY include the Proxy-Info AVP in requests if
           it requires access to any local state information when the
           corresponding response is received.  Proxy-Info AVP has certain
           security implications and SHOULD contain an embedded HMAC with a
           node-local key.  Alternatively, it MAY simply use local storage to
           store state information.

           The message is then forwarded to the next hop, as identified in the
           Realm Routing Table.

           Figure 6 provides an example of message routing using the procedures
           listed in these sections.

             (Origin-Host=nas.mno.net)    (Origin-Host=nas.mno.net)
             (Origin-Realm=mno.net)       (Origin-Realm=mno.net)
             (Destination-Realm=example.com)  (Destination-
                                               Realm=example.com)
                                          (Route-Record=nas.example.net)
             +------+      ------>      +------+      ------>      +------+
             |      |     (Request)     |      |      (Request)    |      |
             | NAS  +-------------------+ DRL  +-------------------+ HMS  |
             |      |                   |      |                   |      |
             +------+     <------       +------+     <------       +------+
            example.net    (Answer)   example.net     (Answer)   example.com
                (Origin-Host=hms.example.com)   (Origin-Host=hms.example.com)
                (Origin-Realm=example.com)      (Origin-Realm=example.com)

                        Figure 6: Routing of Diameter messages
        */
        int h2hIndex, rcode;
        diameter_identity_t *oHost = originHost.GetAvp(DIAMETER_AVPNAME_ORIGINHOST);
        if (! oHost || ((*oHost) != DIAMETER_CFG_TRANSPORT()->identity)) {

            if (msg->hdr.flags.t && LookupQueuedMessage(msg->hdr.hh)) {
                // locally re-transmitted messages
                rcode = (dest->Send(msg, false) >= 0) ? 0 : (-1);
                StoreRequestMessage(msg->hdr.hh, msg);
                return rcode;
            }
            else {
                if (m_Arg.m_rcForward.Delivery().LoopDetection(msg) == 0) {
                    std::auto_ptr<DiameterMsg> answerMsg = DiameterErrorMsg::Generate(*msg, AAA_LOOP_DETECTED);
                    return (source->Send(answerMsg) >= 0) ? 0 : (-1);
                }

                DiameterIdentityAvpWidget rrecord(DIAMETER_AVPNAME_ROUTERECORD);
                rrecord.Get() = DIAMETER_CFG_TRANSPORT()->identity;
                msg->acl.add(rrecord());
            }
        }

        if (route->Action() == DIAMETER_ROUTE_ACTION_PROXY) {
            if (m_Arg[DiameterMsgRouterHandlerTable::H_PROXY]->
                    Request(msg, source, dest) != 0) {
                // Proxy has enforced a policy. 
                // Message will not be sent.
                return (0);
            }
        }

        /*
          The Diameter protocol requires that agents maintain
          transaction state, which is used for failover purposes.
          Transaction state implies that upon forwarding a request,
          the Hop-by-Hop identifier is saved; the field is replaced
          with a locally unique identifier, which is restored to its
          original value when the corresponding answer is received.
          The request's state is released upon receipt of the answer.
          A stateless agent is one that only maintains transaction
          state.
         */
        if (source) {
            int localHH = DIAMETER_HOPBYHOP_GEN()->Get();
            h2hIndex = Add(localHH, msg, source, dest);
            msg->hdr.hh = localHH;
        }
        else {
            // locally generated
            if (msg->hdr.hh == 0) {
                msg->hdr.hh = DIAMETER_HOPBYHOP_GEN()->Get();
            }
            if (msg->hdr.ee == 0) {
                msg->hdr.ee = DIAMETER_ENDTOEND_GEN()->Get();
            }
            h2hIndex = Add(msg->hdr.hh, msg, source, dest);
        }

        rcode = (dest->Send(msg, false) >= 0) ? 0 : (-1);
        if (h2hIndex > 0) {
            StoreRequestMessage(h2hIndex, msg);
        }
        return rcode;
    }
    return (0);
}

AAA_ROUTE_RESULT DiameterMsgRouter::DcReject::Process(std::auto_ptr<DiameterMsg> msg,
                                                      std::auto_ptr<DiameterMsg> &p,
                                                      DiameterPeerEntry *source,
                                                      DiameterPeerEntry *dest)
{
    /*
       6.2.1.  Processing received Answers

        A Diameter client or proxy MUST match the Hop-by-Hop Identifier in an
        answer received against the list of pending requests.  The
        corresponding message should be removed from the list of pending
        requests.  It SHOULD ignore answers received that do not match a
        known Hop-by-Hop Identifier.
    */
    AAA_LOG((LM_INFO, "(%P|%t) *** Router rejected answer message ***\n"));
    DiameterMsgHeaderDump::Dump(*msg);
    return (AAA_ROUTE_RESULT_SUCCESS);
}

int DiameterMsgRouter::DcReject::RequestMsg(std::auto_ptr<DiameterMsg> msg,
                                            DiameterPeerEntry *source,
                                            DiameterPeerEntry *dest)
{
    /*
       6.1.  Diameter Request Routing Overview
                     .
                     .
                     .
         4. If none of the above is successful, an answer is returned with the
            Result-Code set to AAA_UNABLE_TO_DELIVER, with the E-bit set.
     */
    AAA_LOG((LM_INFO, "(%P|%t) *** Router rejected request message ***\n"));
    DiameterMsgHeaderDump::Dump(*msg);

    if (! source) {
        // request must have been locally generated
        m_Arg[DiameterMsgRouterHandlerTable::H_ERROR]->
                Request(msg, source, dest);
        return (0);
    }
    else {
        std::auto_ptr<DiameterMsg> answerMsg = DiameterErrorMsg::Generate(*msg, AAA_UNABLE_TO_DELIVER);
        return (source->Send(answerMsg) == 0) ? 0 : (-1);
    }
}

AAA_ROUTE_RESULT DiameterMsgRouter::DcReject::Lookup(std::auto_ptr<DiameterMsg> &msg,
                                                     std::auto_ptr<DiameterMsg> &p,
                                                     DiameterPeerEntry *&dest)
{
    /*
       6.2.1.  Processing received Answers

        A Diameter client or proxy MUST match the Hop-by-Hop Identifier in an
        answer received against the list of pending requests.  The
        corresponding message should be removed from the list of pending
        requests.  It SHOULD ignore answers received that do not match a
        known Hop-by-Hop Identifier.
    */
    return (AAA_ROUTE_RESULT_SUCCESS);
}

int DiameterMsgRouter::RedirectAgent::Request(std::auto_ptr<DiameterMsg> &msg,
                                              DiameterPeerEntry *source,
                                              DiameterPeerEntry *dest)
{
    /*
      6.1.7.  Redirecting requests

       When a redirect agent receives a request whose routing entry is set
       to REDIRECT, it MUST reply with an answer message with the 'E' bit
       set, while maintaining the Hop-by-Hop Identifier in the header, and
       include the Result-Code AVP to AAA_REDIRECT_INDICATION.  Each of
       the servers associated with the routing entry are added in separate
       Redirect-Host AVP.

                      +------------------+
                      |     Diameter     |
                      |  Redirect Agent  |
                      +------------------+
                       ^    |    2. command + 'E' bit
        1. Request     |    |    Result-Code =
       joe@example.com |    |    AAA_REDIRECT_INDICATION +
                       |    |    Redirect-Host AVP(s)
                       |    v
                   +-------------+  3. Request  +-------------+
                   | example.com |------------->| example.net |
                   |    Relay    |              |   Diameter  |
                   |    Agent    |<-------------|    Server   |
                   +-------------+  4. Answer   +-------------+

                         Figure 5: Diameter Redirect Agent

       The receiver of the answer message with the 'E' bit set, and the
       Result-Code AVP set to AAA_REDIRECT_INDICATION uses the hop-by-
       hop field in the Diameter header to identify the request in the
       pending message queue (see Section 5.3) that is to be redirected.  If
       no transport connection exists with the new agent, one is created,
       and the request is sent directly to it.

       Multiple Redirect-Host AVPs are allowed.  The receiver of the answer
       message with the 'E' bit set selects exactly one of these hosts as
       the destination of the redirected message.
    */
    DiameterIdentityAvpContainerWidget destRealm(msg->acl);
    diameter_identity_t *DestRealm = destRealm.GetAvp(DIAMETER_AVPNAME_DESTREALM);
    DiameterRouteEntry *route = DIAMETER_ROUTE_TABLE()->Lookup(*DestRealm);

    std::auto_ptr<DiameterMsg> answerMsg = DiameterErrorMsg::Generate(*msg, AAA_REDIRECT_INDICATION);

    AAA_LOG((LM_INFO, "(%P|%t) Request is being re-directed\n"));

    try {
        diameter_unsigned32_t *idPtr;
        char *avpNames[] = { DIAMETER_AVPNAME_AUTHAPPID,
                             DIAMETER_AVPNAME_ACCTAPPID };
        DiameterUInt32AvpContainerWidget id(msg->acl);
        for (unsigned int i=0; i<sizeof(avpNames)/sizeof(char*); i++) {
            if ((idPtr = id.GetAvp(avpNames[i]))) {
                break;
            }
        }

        DiameterRouteApplication *app = route->Lookup(msg->hdr.appId, 0);
        if ((app == NULL) && idPtr) {
            app = route->Lookup(*idPtr, 0); // base protocol only
        }
        else {
            DiameterGroupedAvpContainerWidget vendorSpecificId(msg->acl);
            diameter_grouped_t *grouped = vendorSpecificId.GetAvp
                (DIAMETER_AVPNAME_VENDORAPPID);
            if (grouped) {
                DiameterUInt32AvpContainerWidget gVendorId(*grouped);
                diameter_unsigned32_t *vid = gVendorId.GetAvp(DIAMETER_AVPNAME_VENDORID);
                if (vid == NULL) {
                    AAA_LOG((LM_INFO, "(%P|%t) Vendor-Specific-Application-Id has no vendor ID AVP\n"));
                    return (-1);
                }

                diameter_unsigned32_t *value = NULL;
                char *avpNames[] = { DIAMETER_AVPNAME_AUTHAPPID,
                                     DIAMETER_AVPNAME_ACCTAPPID };
                DiameterUInt32AvpContainerWidget gId(*grouped);
                for (unsigned int i=0; i<sizeof(avpNames)/sizeof(char*); i++) {
                    if ((value = gId.GetAvp(avpNames[i]))) {
                       break;
                    }
                }

                if (value) {
                    app = route->Lookup(*value, *vid);
                }
            }
        }

        if (app) {
            DiameterDiamUriAvpWidget redirect(DIAMETER_AVPNAME_REDIRECTHOST);
            DiameterRouteServerEntry *server = app->Servers().First();
            while (server) {
                diameter_uri_t &uri = redirect.Get();
                uri.fqdn = server->Server();
                server = app->Servers().Next(*server);
            }
            if (! redirect.empty()) {
                DiameterEnumAvpWidget redirectUsage(DIAMETER_AVPNAME_REDIRECTHOSTUSAGE);
                redirectUsage.Get() = route->RedirectUsage();
                answerMsg->acl.add(redirectUsage());
                answerMsg->acl.add(redirect());
            }
        }
    }
    catch (...) {
        return (-1);
    }

    return (source->Send(answerMsg) >= 0) ? 0 : (-1);
}

int DiameterMsgRouter::RedirectAgent::Answer(std::auto_ptr<DiameterMsg> &msg,
                                             std::auto_ptr<DiameterMsg> p,
                                             DiameterPeerEntry *source,
                                             DiameterPeerEntry *dest)
{
    /*
      2.8.3.  Redirect Agents

       Redirect agents are useful in scenarios where the Diameter routing
       configuration needs to be centralized.  An example is a redirect
       agent that provides services to all members of a consortium, but does
       not wish to be burdened with relaying all messages between realms.
       This scenario is advantageous since it does not require that the
       consortium provide routing updates to its members when changes are
       made to a member's infrastructure.

       Since redirect agents do not relay messages, and only return an
       answer with the information necessary for Diameter agents to
       communicate directly, they do not modify messages.  Since redirect
       agents do not receive answer messages, they cannot maintain session
       state.  Further, since redirect agents never relay requests, they are
       not required to maintain transaction state.

       The example provided in Figure 3 depicts a request issued from the
       access device, NAS, for the user bob@example.com.  The message is
       forwarded by the NAS to its relay, DRL, which does not have a routing
       entry in its Diameter Routing Table for example.com.  DRL has a
       default route configured to DRD, which is a redirect agent that
       returns a redirect notification to DRL, as well as HMS' contact
       information.  Upon receipt of the redirect notification, DRL
       establishes a transport connection with HMS, if one doesn't already
       exist, and forwards the request to it.

                                   +------+
                                   |      |
                                   | DRD  |
                                   |      |
                                   +------+
                                    ^    |
                        2. Request  |    | 3. Redirection
                                    |    |    Notification
                                    |    v
        +------+    --------->     +------+     --------->    +------+
        |      |    1. Request     |      |     4. Request    |      |
        | NAS  |                   | DRL  |                   | HMS  |
        |      |    6. Answer      |      |     5. Answer     |      |
        +------+    <---------     +------+     <---------    +------+
       example.net                example.net               example.com

                     Figure 3: Redirecting a Diameter Message

       Since redirect agents do not perform any application level
       processing, they provide relaying services for all Diameter
       applications, and therefore MUST advertise the Relay Application
       Identifier.
    */

    AAA_LOG((LM_INFO, "(%P|%t) Processing re-directed answer\n"));

    DiameterUInt32AvpContainerWidget resultCode(msg->acl);
    resultCode.DelAvp(DIAMETER_AVPNAME_RESULTCODE);

    DiameterUriAvpContainerWidget redirect(msg->acl);
    diameter_uri_t *RedirectHost = redirect.GetAvp(DIAMETER_AVPNAME_REDIRECTHOST);

    for (int h=1; RedirectHost; h++) {
        DiameterPeerEntry *peer = DIAMETER_PEER_TABLE()->Lookup(RedirectHost->fqdn);
        if (peer) {
            if (peer->IsOpen()) {
                DiameterIdentityAvpContainerWidget dhost(p->acl);
                diameter_identity_t *hostname = dhost.GetAvp(DIAMETER_AVPNAME_DESTHOST);
                if (hostname) {
                    *hostname = RedirectHost->fqdn;
                }
                else {
                    DiameterIdentityAvpWidget destHost(DIAMETER_AVPNAME_DESTHOST);
                    destHost.Get() = RedirectHost->fqdn;
                    p->acl.add(destHost());
                }
                AAA_LOG((LM_INFO, "(%P|%t) Redirecting msg to [%s]\n",
                        RedirectHost->fqdn.c_str()));
                return (peer->Send(p) >= 0) ? 0 : (-1);
            }
            else {
                AAA_LOG((LM_INFO, "(%P|%t) TBD: Have to try and connect to [%s]\n",
                        RedirectHost->fqdn.c_str()));
                // try opening it [TBD]
            }
        }
        else {
            AAA_LOG((LM_INFO, "(%P|%t) TBD: Have to try and connect to [%s]\n",
                    RedirectHost->fqdn.c_str()));
            // try to make connection to it [TBD]
        }

        RedirectHost = redirect.GetAvp(DIAMETER_AVPNAME_REDIRECTHOST, h);
    }

    AAA_LOG((LM_INFO, "(%P|%t) Suggested re-direct host is un-available\n"));
    return (0);
}

bool DiameterMsgRouter::RedirectAgent::IsRedirected(std::auto_ptr<DiameterMsg> &msg)
{
    /*
       The receiver of the answer message with the 'E' bit set, and the
       Result-Code AVP set to AAA_REDIRECT_INDICATION uses the hop-by-
       hop field in the Diameter header to identify the request in the
       pending message queue (see Section 5.3) that is to be redirected.  If
       no transport connection exists with the new agent, one is created,
       and the request is sent directly to it.

       Multiple Redirect-Host AVPs are allowed.  The receiver of the answer
       message with the 'E' bit set selects exactly one of these hosts as
       the destination of the redirected message.
    */
    DiameterMsgResultCode rcode(*msg);
    return ((msg->hdr.flags.r == 0) &&
            (msg->hdr.flags.e) &&
            (rcode.ResultCode() == AAA_REDIRECT_INDICATION));
}
