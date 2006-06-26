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


#ifndef __ROUTE_TABLE_H__
#define __ROUTE_TABLE_H__

#include <list>
#include <string>
#include <map>
#include "ace/Singleton.h"
#include "od_utl_rbtree.h"

typedef enum {
   AAA_ROUTE_ACTION_LOCAL,
   AAA_ROUTE_ACTION_RELAY,
   AAA_ROUTE_ACTION_PROXY,
   AAA_ROUTE_ACTION_REDIRECT
} AAA_ROUTE_ACTION;

typedef enum {
   AAA_REDIRECT_USAGE_DONT_CACHE,
   AAA_REDIRECT_USAGE_ALL_SESSION,
   AAA_REDIRECT_USAGE_ALL_REALM,
   AAA_REDIRECT_USAGE_REALM_AND_APPLICATION,
   AAA_REDIRECT_USAGE_ALL_APPLICATION,
   AAA_REDIRECT_USAGE_ALL_HOST,
   AAA_REDIRECT_USAGE_ALL_USER
} AAA_REDIRECT_USAGE;

class AAA_RouteServerEntry
{
   public:
      AAA_RouteServerEntry() :
          m_Metric(0) {
      }    
      AAA_RouteServerEntry(std::string &s,
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

typedef std::list<AAA_RouteServerEntry*> AAA_RouteServerList;

class AAA_RouteServers : private AAA_RouteServerList
{
   public:
      AAA_RouteServers() { }
      virtual ~AAA_RouteServers();
      int Add(AAA_RouteServerEntry &e);
      AAA_RouteServerEntry *Lookup(std::string &server);
      AAA_RouteServerEntry *First();
      AAA_RouteServerEntry *Next(AAA_RouteServerEntry &prev);
      int Remove(std::string &server);
};

class AAA_RouteApplication
{
   public:
      AAA_RouteApplication(int appId = 0,
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
      AAA_RouteServers &Servers() {
          return m_Servers;
      }

   private:
      int m_ApplicationId;
      int m_VendorId;
      AAA_RouteServers m_Servers;
};

typedef std::map<int, AAA_RouteApplication*> AAA_RouteAppIdMap;
typedef std::map<int, AAA_RouteAppIdMap*> AAA_RouteVendorIdMap;

class AAA_RouteEntry : public OD_Utl_RbTreeData
{
   public:
      AAA_RouteEntry(std::string &realm,
                     AAA_ROUTE_ACTION a = AAA_ROUTE_ACTION_LOCAL) :
          m_Realm(realm), m_Action(a) {
      }
      ~AAA_RouteEntry() {
          clear();
      }
      std::string &Realm() {
          return m_Realm;
      }
      AAA_ROUTE_ACTION &Action() {
          return m_Action;
      }
      AAA_REDIRECT_USAGE &RedirectUsage() {
          return m_RedirectUsage;
      }
      int Add(AAA_RouteApplication &a);
      AAA_RouteApplication *Lookup(int appId,
                                   int vendorId = 0);
      AAA_RouteApplication *First();
      AAA_RouteApplication *Next(AAA_RouteApplication &app);
      int Remove(int appId, int vendorId = 0);
      virtual void Dump(void *userData);

   protected:
      std::string m_Realm;
      AAA_ROUTE_ACTION m_Action;
      AAA_REDIRECT_USAGE m_RedirectUsage;
      AAA_RouteVendorIdMap m_Identifiers; 

   private:
      int operator < (OD_Utl_RbTreeData &cmp) {
          AAA_RouteEntry *e = reinterpret_cast<AAA_RouteEntry*>(&cmp);
          return (m_Realm < e->Realm());
      }
      int operator == (OD_Utl_RbTreeData &cmp) {
          AAA_RouteEntry *e = reinterpret_cast<AAA_RouteEntry*>(&cmp);
          return (m_Realm == e->Realm());
      }
      void clear(void *userData = 0);
};

class AAA_RouteTable 
{
   public:
      AAA_RouteTable(int expireTime = 0) :
          m_ExpireTime(expireTime),
          m_DefaultRoute(NULL) {
      }
      ~AAA_RouteTable() {
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
      AAA_RouteEntry *DefaultRoute() {
          return m_DefaultRoute;
      }
      void DefaultRoute(AAA_RouteEntry &e) {
          if (m_DefaultRoute) {
              delete m_DefaultRoute;
          }
          m_DefaultRoute = &e;
      }
      int Add(AAA_RouteEntry &e) {
          Remove(e.Realm());
          return m_Routes.Insert(&e) ? (0) : (-1);
      }
      AAA_RouteEntry *Lookup(std::string &realm) {
          AAA_RouteEntry search(realm);
          AAA_RouteEntry *rte = reinterpret_cast
               <AAA_RouteEntry*>(m_Routes.Find(&search));
          return (rte == NULL) ? DefaultRoute() : rte;
      }
      int Remove(std::string &realm) {
          AAA_RouteEntry search(realm);
          AAA_RouteEntry *e = reinterpret_cast<
              AAA_RouteEntry*>(m_Routes.Remove(&search));
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
      AAA_RouteEntry *m_DefaultRoute;
};

typedef ACE_Singleton<AAA_RouteTable, ACE_Recursive_Thread_Mutex> AAA_RouteTable_S;
#define AAA_ROUTE_TABLE() AAA_RouteTable_S::instance() 

#endif /* __ROUTE_TABLE_H__ */

