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

#include "aaa_route_table.h"
#include "aaa_log_facility.h"

DiameterRouteServers::~DiameterRouteServers()
{
    while (! empty()) {
        DiameterRouteServerEntry *e = front();
        pop_front();
        delete e;
    }
}
    
int DiameterRouteServers::Add(DiameterRouteServerEntry &e)
{
    Remove(e.Server());
    DiameterRouteServerList::iterator i;
    for (i = begin(); i != end(); i++) {
        if ((*i)->Metric() >= e.Metric()) {
            insert(i, &e);
            break;
        }
    }
    if (i == end()) {
        push_back(&e);
    }
    return (0);
}

DiameterRouteServerEntry *DiameterRouteServers::Lookup(std::string &server)
{
    DiameterRouteServerList::iterator i;
    for (i = begin(); i != end(); i ++) {
        if (server == (*i)->Server()) {
           return (*i);
        }
    }
    return (NULL);
}

DiameterRouteServerEntry *DiameterRouteServers::First()
{
    DiameterRouteServerEntry *e = front();
    return (e);
}

DiameterRouteServerEntry *DiameterRouteServers::Next(
    DiameterRouteServerEntry &prev)
{
    DiameterRouteServerList::iterator i;
    for (i = begin(); i != end(); i ++) {
       if (&prev == (*i)) {
           i ++;
           if (i != end()) {
               return (*i);
           }
           break;
       }
    }
    return (NULL);    
}

int DiameterRouteServers::Remove(std::string &server)
{
    DiameterRouteServerList::iterator i;
    for (i = begin(); i != end(); i ++) {
       if (server == (*i)->Server()) {
           delete (*i);
           erase(i);
           return (1);
       }
    }
    return (0);
}

int DiameterRouteEntry::Add(DiameterRouteApplication &a)
{
    DiameterRouteAppIdMap *idMap;
    Remove(a.VendorId(), a.ApplicationId());
    DiameterRouteVendorIdMap::iterator x = m_Identifiers.find(a.VendorId());
    if (x == m_Identifiers.end()) {
        idMap = new DiameterRouteAppIdMap;
        m_Identifiers.insert(std::pair<int, DiameterRouteAppIdMap*>
             (a.VendorId(), idMap));
    }
    else {
        idMap = x->second;
    }
    idMap->insert(std::pair<int, DiameterRouteApplication*>
                  (a.ApplicationId(),
                   static_cast<DiameterRouteApplication*>(&a)));
    return (0);
}

DiameterRouteApplication *DiameterRouteEntry::Lookup(int appId, int vendorId)
{
    DiameterRouteVendorIdMap::iterator x = m_Identifiers.find(vendorId);
    if (x != m_Identifiers.end()) {
        DiameterRouteAppIdMap *id = x->second;
        DiameterRouteAppIdMap::iterator y = id->find(appId);
        return (y != id->end()) ? y->second : 0;
    }
    return (0);
}

DiameterRouteApplication *DiameterRouteEntry::First()
{
    DiameterRouteVendorIdMap::iterator x = m_Identifiers.begin();
    if (x != m_Identifiers.end()) {
        DiameterRouteAppIdMap *id = x->second;
        DiameterRouteAppIdMap::iterator y = id->begin();
        return (y != id->end()) ? y->second : 0;
    }
    return (0);
}

DiameterRouteApplication *DiameterRouteEntry::Next(DiameterRouteApplication &app)
{
    bool lookup = true;
    DiameterRouteVendorIdMap::iterator x = m_Identifiers.find(app.VendorId());
    for (; x != m_Identifiers.end(); x++) {
        DiameterRouteAppIdMap *id = x->second;
        DiameterRouteAppIdMap::iterator y = (lookup) ?
            id->find(app.ApplicationId()) : id->begin();
        for (y++; y != id->end(); y++) {
            return y->second;
        }
        lookup = false;
    }
    return (0);
}

int DiameterRouteEntry::Remove(int appId, int vendorId)
{
    DiameterRouteVendorIdMap::iterator x = m_Identifiers.find(vendorId);
    if (x != m_Identifiers.end()) {
       DiameterRouteAppIdMap *id = x->second;
       DiameterRouteAppIdMap::iterator y = id->find(appId);
       if (y != id->end()) {
           delete y->second;
           id->erase(y);
           if (id->empty()) {
               m_Identifiers.erase(x);
               delete id;
           }
           return (0);
        }
    }
    return (-1);
}

void DiameterRouteEntry::clear(void *userData)
{
    while (! m_Identifiers.empty()) {
        DiameterRouteVendorIdMap::iterator x = m_Identifiers.begin();
        DiameterRouteAppIdMap *id = x->second;
        while (! id->empty()) {
            DiameterRouteAppIdMap::iterator y = id->begin();
            DiameterRouteApplication *app = y->second;
            id->erase(y);
            delete app;
        }
        m_Identifiers.erase(x);
        delete id;
    }
}

void DiameterRouteEntry::Dump(void *userData)
{
    AAA_LOG((LM_INFO, "(%P|%t)              Route  : Realm = %s, Action = %d, Redirect-Usage = %d\n",
            m_Realm.c_str(), m_Action, m_RedirectUsage));

    DiameterRouteVendorIdMap::iterator x = m_Identifiers.begin();
    for (; x != m_Identifiers.end(); x++) {
        DiameterRouteAppIdMap *id = x->second;
        DiameterRouteAppIdMap::iterator y = id->begin();
        for (; y != id->end(); y++) {
            DiameterRouteApplication *app = y->second;
            AAA_LOG((LM_INFO, "(%P|%t)                       Application Id=%d, Vendor=%d\n",
                 app->ApplicationId(), app->VendorId()));
            DiameterRouteServerEntry *server = app->Servers().First();
            while (server) {
                AAA_LOG((LM_INFO, "(%P|%t)                          Server = %s, metric = %d\n", 
                     server->Server().c_str(), server->Metric()));
                server = app->Servers().Next(*server);
            }
        }
    }
}

void DiameterRouteTable::Dump()
{
    AAA_LOG((LM_INFO, "(%P|%t)  Dumping Route Table\n"));
    AAA_LOG((LM_INFO, "(%P|%t)            Exp Time : %d\n", m_ExpireTime));
    m_Routes.Dump();
    if (m_DefaultRoute) {
        AAA_LOG((LM_INFO, "(%P|%t)      Default Route\n"));
        m_DefaultRoute->Dump(0);
    }
}
