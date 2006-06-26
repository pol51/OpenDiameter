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

#include "aaa_route_table.h"
#include "aaa_log_facility.h"

AAA_RouteServers::~AAA_RouteServers()
{
    while (! empty()) {
        AAA_RouteServerEntry *e = front();
        pop_front();
        delete e;
    }
}
    
int AAA_RouteServers::Add(AAA_RouteServerEntry &e)
{
    Remove(e.Server());
    AAA_RouteServerList::iterator i;
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

AAA_RouteServerEntry *AAA_RouteServers::Lookup(std::string &server)
{
    AAA_RouteServerList::iterator i;
    for (i = begin(); i != end(); i ++) {
        if (server == (*i)->Server()) {
           return (*i);
        }
    }
    return (NULL);
}

AAA_RouteServerEntry *AAA_RouteServers::First()
{
    AAA_RouteServerEntry *e = front();
    return (e);
}

AAA_RouteServerEntry *AAA_RouteServers::Next(
    AAA_RouteServerEntry &prev)
{
    AAA_RouteServerList::iterator i;
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

int AAA_RouteServers::Remove(std::string &server)
{
    AAA_RouteServerList::iterator i;
    for (i = begin(); i != end(); i ++) {
       if (server == (*i)->Server()) {
           delete (*i);
           erase(i);
           return (1);
       }
    }
    return (0);
}

int AAA_RouteEntry::Add(AAA_RouteApplication &a)
{
    AAA_RouteAppIdMap *idMap;
    Remove(a.VendorId(), a.ApplicationId());
    AAA_RouteVendorIdMap::iterator x = m_Identifiers.find(a.VendorId());
    if (x == m_Identifiers.end()) {
        idMap = new AAA_RouteAppIdMap;
        m_Identifiers.insert(std::pair<int, AAA_RouteAppIdMap*>
             (a.VendorId(), idMap));
    }
    else {
        idMap = x->second;
    }
    idMap->insert(std::pair<int, AAA_RouteApplication*>
                  (a.ApplicationId(),
                   static_cast<AAA_RouteApplication*>(&a)));
    return (0);
}

AAA_RouteApplication *AAA_RouteEntry::Lookup(int appId, int vendorId)
{
    AAA_RouteVendorIdMap::iterator x = m_Identifiers.find(vendorId);
    if (x != m_Identifiers.end()) {
        AAA_RouteAppIdMap *id = x->second;
        AAA_RouteAppIdMap::iterator y = id->find(appId);
        return (y != id->end()) ? y->second : 0;
    }
    return (0);
}

AAA_RouteApplication *AAA_RouteEntry::First()
{
    AAA_RouteVendorIdMap::iterator x = m_Identifiers.begin();
    if (x != m_Identifiers.end()) {
        AAA_RouteAppIdMap *id = x->second;
        AAA_RouteAppIdMap::iterator y = id->begin();
        return (y != id->end()) ? y->second : 0;
    }
    return (0);
}

AAA_RouteApplication *AAA_RouteEntry::Next(AAA_RouteApplication &app)
{
    bool lookup = true;
    AAA_RouteVendorIdMap::iterator x = m_Identifiers.find(app.VendorId());
    for (; x != m_Identifiers.end(); x++) {
        AAA_RouteAppIdMap *id = x->second;
        AAA_RouteAppIdMap::iterator y = (lookup) ?
            id->find(app.ApplicationId()) : id->begin();
        for (y++; y != id->end(); y++) {
            return y->second;
        }
        lookup = false;
    }
    return (0);
}

int AAA_RouteEntry::Remove(int appId, int vendorId)
{
    AAA_RouteVendorIdMap::iterator x = m_Identifiers.find(vendorId);
    if (x != m_Identifiers.end()) {
       AAA_RouteAppIdMap *id = x->second;
       AAA_RouteAppIdMap::iterator y = id->find(appId);
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

void AAA_RouteEntry::clear(void *userData)
{
    while (! m_Identifiers.empty()) {
        AAA_RouteVendorIdMap::iterator x = m_Identifiers.begin();
        AAA_RouteAppIdMap *id = x->second;
        while (! id->empty()) {
            AAA_RouteAppIdMap::iterator y = id->begin();
            AAA_RouteApplication *app = y->second;
            id->erase(y);
            delete app;
        }
        m_Identifiers.erase(x);
        delete id;
    }
}

void AAA_RouteEntry::Dump(void *userData)
{
    AAA_LOG(LM_INFO, "(%P|%t)              Route  : Realm = %s, Action = %d, Redirect-Usage = %d\n", 
            m_Realm.data(), m_Action, m_RedirectUsage);

    AAA_RouteVendorIdMap::iterator x = m_Identifiers.begin();
    for (; x != m_Identifiers.end(); x++) {
        AAA_RouteAppIdMap *id = x->second;
        AAA_RouteAppIdMap::iterator y = id->begin();
        for (; y != id->end(); y++) {
            AAA_RouteApplication *app = y->second;
            AAA_LOG(LM_INFO, "(%P|%t)                       Application Id=%d, Vendor=%d\n",
                 app->ApplicationId(), app->VendorId());
            AAA_RouteServerEntry *server = app->Servers().First();
            while (server) {
                AAA_LOG(LM_INFO, "(%P|%t)                          Server = %s, metric = %d\n", 
                     server->Server().data(), server->Metric());
                server = app->Servers().Next(*server);
            }
        }
    }
}

void AAA_RouteTable::Dump()
{
    AAA_LOG(LM_INFO, "(%P|%t)  Dumping Route Table\n");
    AAA_LOG(LM_INFO, "(%P|%t)            Exp Time : %d\n", m_ExpireTime);
    m_Routes.Dump();
    if (m_DefaultRoute) {
        AAA_LOG(LM_INFO, "(%P|%t)      Default Route\n");
        m_DefaultRoute->Dump(0);
    }
}
