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

#ifndef __OD_UTL_PATTERN_H__
#define __OD_UTL_PATTERN_H__

// ---- function object definitions

template<class ARG>
class OD_Utl_CbFunction1
{
   public:
      virtual ~OD_Utl_CbFunction1() { }
      virtual void operator()(ARG arg) = 0;
      virtual OD_Utl_CbFunction1<ARG> *clone() const = 0;
};

template<class ARG1, class ARG2>
class OD_Utl_CbFunction2
{
   public:
      virtual ~OD_Utl_CbFunction2() { }
      virtual void operator()(ARG1 arg1, ARG2 arg2) = 0;
      virtual OD_Utl_CbFunction2<ARG1, ARG2> *clone() const = 0;
};

// ---- parameterized method name pattern adapter definitions

template<class CLIENT, class ARG1>
class OD_Utl_PMNAdapter1 : public OD_Utl_CbFunction1<ARG1>
{
   public:
      typedef void (CLIENT::*PMEMFUNC)(ARG1 arg);
      OD_Utl_PMNAdapter1(CLIENT &c, PMEMFUNC func) : m_Client(c),
                                                  m_MemberFunc(func) { }
      OD_Utl_CbFunction1<ARG1> *clone() const { 
         return (new OD_Utl_PMNAdapter1<CLIENT, ARG1>(*this)); 
      }
      virtual void operator()(ARG1 arg) { (m_Client.*m_MemberFunc)(arg); }
   protected:
      CLIENT &m_Client;
      PMEMFUNC m_MemberFunc;
};

template<class CLIENT, class ARG1, class ARG2>
class OD_Utl_PMNAdapter2 : public OD_Utl_CbFunction2<ARG1, ARG2>
{
   public:
      typedef void (CLIENT::*PMEMFUNC)(ARG1 arg1, ARG2 arg2);
      OD_Utl_PMNAdapter2(CLIENT &c, PMEMFUNC func) : m_Client(c),
                                                  m_MemberFunc(func) { }
      OD_Utl_CbFunction2<ARG1, ARG2> *clone() const { 
         return (new OD_Utl_PMNAdapter2<CLIENT, ARG1, ARG2>(*this)); 
      }
      virtual void operator()(ARG1 arg1, ARG2 arg2) {
          (m_Client.*m_MemberFunc)(arg1, arg2);
      }
   protected:
      CLIENT &m_Client;
      PMEMFUNC m_MemberFunc;
};

// ---- separate client and server interface pattern adapter definitions

template<class CLIENT, class MEMBER, class ARG1>
class OD_Utl_SCSIAdapter1 : public OD_Utl_CbFunction1<ARG1>
{
   public:
      OD_Utl_SCSIAdapter1(CLIENT &c, MEMBER func) : m_Client(c),
                                                 m_MemberFunc(func) { }
      OD_Utl_CbFunction1<ARG1> *clone() const { 
         return (new OD_Utl_SCSIAdapter1<CLIENT, MEMBER, ARG1>(*this)); 
      }
      virtual void operator()(ARG1 arg) { ((&m_Client)->*m_MemberFunc)(arg); }
   protected:
      CLIENT &m_Client;
      MEMBER m_MemberFunc;
};

template<class CLIENT, class MEMBER, class ARG1, class ARG2>
class OD_Utl_SCSIAdapter2 : public OD_Utl_CbFunction2<ARG1, ARG2>
{
   public:
      OD_Utl_SCSIAdapter2(CLIENT &c, MEMBER func) : m_Client(c),
                                                 m_MemberFunc(func) { }
      OD_Utl_CbFunction2<ARG1, ARG2> *clone() const { 
         return (new OD_Utl_SCSIAdapter2<CLIENT, MEMBER, ARG1, ARG2>(*this)); 
      }
      virtual void operator()(ARG1 arg1, ARG2 arg2) {
          ((&m_Client)->*m_MemberFunc)(arg1, arg2);
      }
   protected:
      CLIENT &m_Client;
      MEMBER m_MemberFunc;
};

// ---- handle body adapter

template<class ARG1>
class OD_Utl_CbBody1
{
   public:
      OD_Utl_CbBody1(OD_Utl_CbFunction1<ARG1> &cbfn) :
          m_Body(cbfn->clone()) { }
      OD_Utl_CbBody1(OD_Utl_CbFunction1<ARG1> *cbfn) :
        m_Body(cbfn) { }
      ~OD_Utl_CbBody1() { delete m_Body; }

      OD_Utl_CbBody1<ARG1>& operator=(OD_Utl_CbBody1<ARG1> &cbfn) {
          if (&cbfn != m_Body) {
              delete m_Body;
              m_Body = cbfn.m_Body->clone();
          }
          return *this;
      }
      virtual void operator()(ARG1 arg) { (*m_Body)()(arg); }
    
   protected:
      OD_Utl_CbFunction1<ARG1>* m_Body;
};

template<class ARG1, class ARG2>
class OD_Utl_CbBody2
{
   public:
      OD_Utl_CbBody2(OD_Utl_CbFunction2<ARG1, ARG2> &cbfn) :
          m_Body(cbfn->clone()) { }
      OD_Utl_CbBody2(OD_Utl_CbFunction2<ARG1, ARG2> *cbfn) :
        m_Body(cbfn) { }
      ~OD_Utl_CbBody2() { delete m_Body; }

      OD_Utl_CbBody2<ARG1, ARG2>& operator=(OD_Utl_CbBody2<ARG1, ARG2> &cbfn) {
          if (&cbfn != m_Body) {
              delete m_Body;
              m_Body = cbfn.m_Body->clone();
          }
          return *this;
      }
      virtual void operator()(ARG1 arg1, ARG2 arg2) {
          (*m_Body)()(arg1, arg2);
      }
    
   protected:
      OD_Utl_CbFunction1<ARG1>* m_Body;
};

template<class CLIENT, class MEMBER, class ARG1>
OD_Utl_CbBody1<ARG1> make_scsi_callback1(OD_Utl_CbBody1<ARG1>* cb,
                                      CLIENT &client,
                                      MEMBER member) {
    return OD_Utl_CbBody1<ARG1>(new
          OD_Utl_SCSIAdapter1<CLIENT, MEMBER, ARG1>(client, member));
}

template<class CLIENT, class MEMBER, class ARG1, class ARG2>
OD_Utl_CbBody2<ARG1, ARG2> make_scsi_callback2(OD_Utl_CbBody2<ARG1, ARG2>* cb,
                                            CLIENT &client,
                                            MEMBER member) {
    return OD_Utl_CbBody2<ARG1, ARG2>(new
          OD_Utl_SCSIAdapter2<CLIENT, MEMBER, ARG1, ARG2>(client, member));
}

#endif // __OD_UTL_PATTERN_H__
