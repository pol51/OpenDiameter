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


#ifndef __ROUTE_TABLE_H__
#define __ROUTE_TABLE_H__

#include <list>
#include <string>
#include <map>
#include "ace/Singleton.h"
#include "od_utl_rbtree.h"

typedef enum {
   DIAMETER_ROUTE_ACTION_LOCAL,
   DIAMETER_ROUTE_ACTION_RELAY,
   DIAMETER_ROUTE_ACTION_PROXY,
   DIAMETER_ROUTE_ACTION_REDIRECT
} DIAMETER_ROUTE_ACTION;

typedef enum {
   DIAMETER_REDIRECT_USAGE_DONT_CACHE,
   DIAMETER_REDIRECT_USAGE_ALL_SESSION,
   DIAMETER_REDIRECT_USAGE_ALL_REALM,
   DIAMETER_REDIRECT_USAGE_REALM_AND_APPLICATION,
   DIAMETER_REDIRECT_USAGE_ALL_APPLICATION,
   DIAMETER_REDIRECT_USAGE_ALL_HOST,
   DIAMETER_REDIRECT_USAGE_ALL_USER
} DIAMETER_REDIRECT_USAGE;

class DiameterRouteServerEntry
{
   public:
      DiameterRouteServerEntry() :
          m_Metric(0) {
      }    
      DiameterRouteServerEntry(std::string &s,
                           int metric = 0) :
          m_Server(s), m_Metric(metric) {
      }    
      std::string &Server() {
          return m_Server;
      }
      int &Metric() {
          return m_Metric;
      }
   private:
      std::string m_Server;
      int m_Metric;
};

typedef std::list<DiameterRouteServerEntry*> DiameterRouteServerList;

class DiameterRouteServers : private DiameterRouteServerList
{
   public:
      DiameterRouteServers() { }
      virtual ~DiameterRouteServers();
      int Add(DiameterRouteServerEntry &e);
      DiameterRouteServerEntry *Lookup(std::string &server);
      DiameterRouteServerEntry *First();
      DiameterRouteServerEntry *Next(DiameterRouteServerEntry &prev);
      int Remove(std::string &server);
};

class DiameterRouteApplication
{
   public:
      DiameterRouteApplication(int appId = 0,
                           int vendorId = 0) :
          m_ApplicationId(appId),
          m_VendorId(vendorId) {
      }
      int &ApplicationId() {
          return m_ApplicationId;
      }
      int &VendorId() {
          return m_VendorId;
      }
      DiameterRouteServers &Servers() {
          return m_Servers;
      }

   private:
      int m_ApplicationId;
      int m_VendorId;
      DiameterRouteServers m_Servers;
};

typedef std::map<int, DiameterRouteApplication*> DiameterRouteAppIdMap;
typedef std::map<int, DiameterRouteAppIdMap*> DiameterRouteVendorIdMap;

class DiameterRouteEntry : public OD_Utl_RbTreeData
{
   public:
      DiameterRouteEntry(std::string &realm,
                     DIAMETER_ROUTE_ACTION a = DIAMETER_ROUTE_ACTION_LOCAL) :
          m_Realm(realm),
          m_Action(a),
          m_RedirectUsage(DIAMETER_REDIRECT_USAGE_DONT_CACHE) {
      }
      ~DiameterRouteEntry() {
          clear();
      }
      std::string &Realm() {
          return m_Realm;
      }
      DIAMETER_ROUTE_ACTION &Action() {
          return m_Action;
      }
      DIAMETER_REDIRECT_USAGE &RedirectUsage() {
          return m_RedirectUsage;
      }
      int Add(DiameterRouteApplication &a);
      DiameterRouteApplication *Lookup(int appId,
                                   int vendorId = 0);
      DiameterRouteApplication *First();
      DiameterRouteApplication *Next(DiameterRouteApplication &app);
      int Remove(int appId, int vendorId = 0);
      virtual void Dump(void *userData);

   protected:
      std::string m_Realm;
      DIAMETER_ROUTE_ACTION m_Action;
      DIAMETER_REDIRECT_USAGE m_RedirectUsage;
      DiameterRouteVendorIdMap m_Identifiers; 

   private:
      int operator < (OD_Utl_RbTreeData &cmp) {
          DiameterRouteEntry *e = reinterpret_cast<DiameterRouteEntry*>(&cmp);
          //
          // Warning: This is a case in-sensitive lookup which may not
          //          be generally appropriate if we consider FQDN as
          //          a non ascii value.
          //
          // Deprecated:
          //  return (m_Realm < e->Realm());
          //
          return (strcasecmp(m_Realm.c_str(), e->Realm().c_str()) < 0) ? 1 : 0;
      }
      int operator == (OD_Utl_RbTreeData &cmp) {
          DiameterRouteEntry *e = reinterpret_cast<DiameterRouteEntry*>(&cmp);
          //
          // Warning: This is a case in-sensitive lookup which may not
          //          be generally appropriate if we consider FQDN as
          //          a non ascii value.
          //
          // Deprecated:
          //  return (m_Realm == e->Realm());
          //
          return (strcasecmp(m_Realm.c_str(), e->Realm().c_str()) == 0) ? 1 : 0;
      }
      void clear(void *userData = 0);
};

class DiameterRouteTable 
{
   public:
      DiameterRouteTable(int expireTime = 0) :
          m_ExpireTime(expireTime),
          m_DefaultRoute(NULL) {
      }
      ~DiameterRouteTable() {
          if (m_DefaultRoute) {
              delete m_DefaultRoute;
          }
          m_Routes.Clear();
      }
      int ExpireTime() {
          return m_ExpireTime;
      }
      void ExpireTime(int expireTime) {
          m_ExpireTime = expireTime;
      }
      DiameterRouteEntry *DefaultRoute() {
          return m_DefaultRoute;
      }
      void DefaultRoute(DiameterRouteEntry &e) {
          if (m_DefaultRoute) {
              delete m_DefaultRoute;
          }
          m_DefaultRoute = &e;
      }
      int Add(DiameterRouteEntry &e) {
          Remove(e.Realm());
          return m_Routes.Insert(&e) ? (0) : (-1);
      }
      DiameterRouteEntry *Lookup(std::string &realm) {
          DiameterRouteEntry search(realm);
          DiameterRouteEntry *rte = reinterpret_cast
               <DiameterRouteEntry*>(m_Routes.Find(&search));
          return (rte == NULL) ? DefaultRoute() : rte;
      }
      int Remove(std::string &realm) {
          DiameterRouteEntry search(realm);
          DiameterRouteEntry *e = reinterpret_cast<
              DiameterRouteEntry*>(m_Routes.Remove(&search));
          if (e) {
              delete e;
              return (0);
          }
          return (-1);
      }
      void Dump();
    
   private:
      OD_Utl_RbTreeTree m_Routes;
      int m_ExpireTime;
      DiameterRouteEntry *m_DefaultRoute;
};

typedef ACE_Singleton<DiameterRouteTable, ACE_Recursive_Thread_Mutex> DiameterRouteTable_S;
#define DIAMETER_ROUTE_TABLE() DiameterRouteTable_S::instance()

#endif /* __ROUTE_TABLE_H__ */

