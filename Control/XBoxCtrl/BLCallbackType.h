/* BLCallbackType.h ***********************************************************
Copyright:  2009 Paul Watt
Author:     Paul Watt
Date:       6/17/2009
Purpose:    Generalized Template definitions to allow callbacks of any type
            to be transferred and assigned in the portable bridge.

            This file contains the help definitions for each of the callback types.
            Include BLCallback.h for usage and instructions.

******************************************************************************/
#ifndef BLCALLBACKTYPE_H_INCLUDED
#define BLCALLBACKTYPE_H_INCLUDED
#include "compiler.h"

namespace pbl
{
namespace callback_type
{
namespace detail
{
/* Class **********************************************************************
Author:     Paul Watt
Date:       6/17/2009
Purpose:    Simple internal reference count base.
******************************************************************************/
class CountedBody
{
public:
            CountedBody() : 
                m_count_(0) {}
  virtual  ~CountedBody()   {}
  void      Inc()           {m_count_++;}
  void      Dec()           {m_count_--;}
  int       Count() const   {return m_count_;}
  
private:
  int m_count_;
};

/*****************************************************************************/
/* Callback Base Class Definitions ********************************************
Author:     Paul Watt
Date:       6/17/2009
Purpose:    The set of class in this section act as the base class to implement
            a Handle/Body idiom in order to abstract the type of callback that is held.
******************************************************************************/
/* Class **********************************************************************
Purpose:    Base for callback with return void and 0arguments.
******************************************************************************/
class BaseV0 : public CountedBody
{
public:
  virtual void operator()() = 0;
};

/* Class **********************************************************************
Purpose:    Base for callback with return type and 0 arguments.
******************************************************************************/
template <typename R>
class BaseR0 : public CountedBody
{
public:
  virtual R operator()() = 0;
};

/* Class **********************************************************************
Purpose:    Base for callback with return void and 1 argument.
******************************************************************************/
template <typename A1>
class BaseV1 : public CountedBody
{
public:
  virtual void operator()(A1) = 0;
};

/* Class **********************************************************************
Purpose:    Base for callback with return type and 1 argument.
******************************************************************************/
template <typename R, typename A1>
class BaseR1 : public CountedBody
{
public:
  virtual R operator()(A1) = 0;
};

/* Class **********************************************************************
Purpose:    Base for callback with return void and 2 arguments.
******************************************************************************/
template <typename A1, typename A2>
class BaseV2 : public CountedBody
{
public:
  virtual void operator()(A1, A2) = 0;
};

/* Class **********************************************************************
Purpose:    Base for callback with return type and 2 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2>
class BaseR2 : public CountedBody
{
public:
  virtual R operator()(A1, A2) = 0;
};

/* Class **********************************************************************
Purpose:    Base for callback with return void and 3 arguments.
******************************************************************************/
template <typename A1, typename A2, typename A3>
class BaseV3 : public CountedBody
{
public:
  virtual void operator()(A1, A2, A3) = 0;
};

/* Class **********************************************************************
Purpose:    Base for callback with return type and 3 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2, typename A3>
class BaseR3 : public CountedBody
{
public:
  virtual R operator()(A1, A2, A3) = 0;
};

/*****************************************************************************/
/* Nil Callback Class Definitions *********************************************
Author:     Paul Watt
Date:       6/17/2009
Purpose:    Nil callback.  
            To invoke this class as a callback is an error, 
            similar to invoking a NULL pointer.

            Debug:    assertion will be triggered.
            Release:  no-op returns 0.
******************************************************************************/
/* Class **********************************************************************
Purpose:    Nil callback with return void and no arguments.
******************************************************************************/
class NilV0 : public BaseV0
{
public:
  void operator()() {BLASSERT(0);}
};

/* Class **********************************************************************
Purpose:    Nil callback with return type and no arguments.
******************************************************************************/
template <typename R>
class NilR0 : public BaseR0<R>
{
public:
  R operator()() { BLASSERT(0); return 0;}
};

/* Class **********************************************************************
Purpose:    Nil callback with return void and 1 argument.
******************************************************************************/
template <typename A1>
class NilV1 : public BaseV1<A1>
{
public:
  void operator()(A1) {BLASSERT(0);}
};

/* Class **********************************************************************
Purpose:    Nil callback with return type and 1 argument.
******************************************************************************/
template <typename R, typename A1>
class NilR1 : public BaseR1<R, A1>
{
public:
  R operator()(A1) { BLASSERT(0); return 0;}
};

/* Class **********************************************************************
Purpose:    Nil callback with return void and 2 arguments.
******************************************************************************/
template <typename A1, typename A2>
class NilV2 : public BaseV2<A1, A2>
{
public:
  void operator()(A1, A2) {BLASSERT(0);}
};

/* Class **********************************************************************
Purpose:    Nil callback with return type and 2 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2>
class NilR2 : public BaseR2<R, A1, A2>
{
public:
  R operator()(A1, A2) { BLASSERT(0); return 0;}
};

/* Class **********************************************************************
Purpose:    Nil callback with return void and 3 arguments.
******************************************************************************/
template <typename A1, typename A2, typename A3>
class NilV3 : public BaseV3<A1, A2, A3>
{
public:
  void operator()(A1, A2, A3) {BLASSERT(0);}
};

/* Class **********************************************************************
Purpose:    Nil callback with return type and 3 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2, typename A3>
class NilR3 : public BaseR3<R, A1, A2, A3>
{
public:
  R operator()(A1, A2, A3) { BLASSERT(0); return 0;}
};

/*****************************************************************************/
/* Function Callback Class Definitions ****************************************
Author:     Paul Watt
Date:       6/17/2009
Purpose:    Template class that abstracts the ability to have a callback to
            either a function pointer or a functor.

            <Function>: The function pointer type to be called by the callback.
******************************************************************************/
/* Class **********************************************************************
Purpose:    Function callback with return void and no arguments.
******************************************************************************/
template <typename Function>
class FnV0: public BaseV0
{
public:
  FnV0(const Function& fn) : m_fn_(fn)  { }
  void operator()()                     { return m_fn_();}
private:
  Function m_fn_;
};

/* Class **********************************************************************
Purpose:    Function callback with return type and no arguments.
******************************************************************************/
template <typename R, typename Function>
class FnR0: public BaseR0<R>
{
public:
  FnR0(const Function& fn) : m_fn_(fn)  { }
  R operator()()                        { return m_fn_();}
private:
  Function m_fn_;
};

/* Class **********************************************************************
Purpose:    Function callback with return void and 1 argument.
******************************************************************************/
template <typename A1, typename Function>
class FnV1: public BaseV1<A1>
{
public:
  FnV1(const Function& fn) : m_fn_(fn)  { }
  void operator()(A1 a1)                { return m_fn_(a1);}
private:
  Function m_fn_;
};

/* Class **********************************************************************
Purpose:    Function callback with return type and 1 argument.
******************************************************************************/
template <typename R, typename A1, typename Function>
class FnR1: public BaseR1<R, A1>
{
public:
  FnR1(const Function& fn) : m_fn_(fn)  { }
  R operator()(A1 a1)                   { return m_fn_(a1);}
private:
  Function m_fn_;
};

/* Class **********************************************************************
Purpose:    Function callback with return void and 2 arguments.
******************************************************************************/
template <typename A1, typename A2, typename Function>
class FnV2: public BaseV2<A1, A2>
{
public:
  FnV2(const Function& fn) : m_fn_(fn)  { }
  void operator()(A1 a1, A2 a2)         { return m_fn_(a1, a2);}
private:
  Function m_fn_;
};

/* Class **********************************************************************
Purpose:    Function callback with return type and 2 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2, typename Function>
class FnR2: public BaseR2<R, A1, A2>
{
public:
  FnR2(const Function& fn) : m_fn_(fn)  { }
  R operator()(A1 a1, A2 a2)            { return m_fn_(a1, a2);}
private:
  Function m_fn_;
};

/* Class **********************************************************************
Purpose:    Function callback with return void and 3 arguments.
******************************************************************************/
template <typename A1, typename A2, typename A3, typename Function>
class FnV3: public BaseV3<A1, A2, A3>
{
public:
  FnV3(const Function& fn) : m_fn_(fn)  { }
  void operator()(A1 a1, A2 a2, A3 a3)  { return m_fn_(a1, a2, a3);}
private:
  Function m_fn_;
};

/* Class **********************************************************************
Purpose:    Function callback with return type and 3 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2, typename A3, typename Function>
class FnR3: public BaseR3<R, A1, A2, A3>
{
public:
  FnR3(const Function& fn) : m_fn_(fn)  { }
  R operator()(A1 a1, A2 a2, A3 a3)     { return m_fn_(a1, a2, a3);}
private:
  Function m_fn_;
};

/*****************************************************************************/
/* Member Callback Class Definitions ******************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Template class that abstracts the ability to have a member function
            of an object as a callback.

            <Obj>: Obj object type to be called by the callback.
            <Member>: Member function or data member pointer type to be called.
              This is what will be invoked by the callback.
******************************************************************************/
/* Class **********************************************************************
Purpose:    Function callback with return void and no arguments.
******************************************************************************/
template <class Obj, class Member>
class MemV0 : public BaseV0
{
public:
  MemV0(Obj* pObj, Member member) :
    m_pObj(pObj), 
    m_member(member)      { }

  void operator()()       { (m_pObj->*m_member)();}
private:
  Obj*    m_pObj;
  Member  m_member;
};

/* Class **********************************************************************
Purpose:    Function callback with return type and no arguments.
******************************************************************************/
template <typename R, class Obj, class Member>
class MemR0 : public BaseR0<R>
{
public:
  MemR0(Obj* pObj, Member member) :
    m_pObj(pObj), 
    m_member(member)      { }

  R operator()()          { return (m_pObj->*m_member)();}
private:
  Obj*    m_pObj;
  Member  m_member;
};

/* Class **********************************************************************
Purpose:    Function callback with return void and 1 argument.
******************************************************************************/
template <typename A1, class Obj, class Member>
class MemV1 : public BaseV1<A1>
{
public:
  MemV1(Obj* pObj, Member member) :
    m_pObj(pObj), 
    m_member(member)      { }

  void operator()(A1 a1)  { (m_pObj->*m_member)(a1);}
private:
  Obj*    m_pObj;
  Member  m_member;
};

/* Class **********************************************************************
Purpose:    Function callback with return type and 1 argument.
******************************************************************************/
template <typename R, typename A1, class Obj, class Member>
class MemR1 :
    public BaseR1<R, A1>
{
public:
  MemR1(Obj* pObj, Member member) :
    m_pObj(pObj), 
    m_member(member)      { }

  R operator()(A1 a1)     { return (m_pObj->*m_member)(a1);}
private:
  Obj*    m_pObj;
  Member  m_member;
};

/* Class **********************************************************************
Purpose:    Function callback with return void and 2 arguments.
******************************************************************************/
template <typename A1, typename A2, class Obj, class Member>
class MemV2 : public BaseV2<A1, A2>
{
public:
  MemV2(Obj* pObj, Member member) :
    m_pObj(pObj), 
    m_member(member)      { }

  void operator()(A1 a1, A2 a2)  { (m_pObj->*m_member)(a1, a2);}
private:
  Obj*    m_pObj;
  Member  m_member;
};

/* Class **********************************************************************
Purpose:    Function callback with return type and 2 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2, class Obj, class Member>
class MemR2 :
    public BaseR2<R, A1, A2>
{
public:
  MemR2(Obj* pObj, Member member) :
    m_pObj(pObj), 
    m_member(member)        { }

  R operator()(A1 a1, A2 a2){ return (m_pObj->*m_member)(a1, a2);}
private:
  Obj*    m_pObj;
  Member  m_member;
};

/* Class **********************************************************************
Purpose:    Function callback with return void and 3 arguments.
******************************************************************************/
template <typename A1, typename A2, typename A3, class Obj, class Member>
class MemV3 
  : public BaseV3<A1, A2, A3>
{
public:
  MemV3(Obj* pObj, Member member) :
    m_pObj(pObj), 
    m_member(member)      { }

  void operator()(A1 a1, A2 a2, A3 a3)  { (m_pObj->*m_member)(a1, a2, a3);}
private:
  Obj*    m_pObj;
  Member  m_member;
};

/* Class **********************************************************************
Purpose:    Function callback with return type and 3 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2, typename A3, class Obj, class Member>
class MemR3 
  : public BaseR3<R, A1, A2, A3>
{
public:
  MemR3(Obj* pObj, Member member) :
    m_pObj(pObj), 
    m_member(member)        { }

  R operator()(A1 a1, A2 a2, A3 a3){ return (m_pObj->*m_member)(a1, a2, a3);}
private:
  Obj*    m_pObj;
  Member  m_member;
};

} // namespace detail

/*****************************************************************************/
/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Callback object for return void with 0 arguments.
******************************************************************************/
class V0
{
public:
  /* Typedefs ******************************************************************/
  typedef detail::BaseV0  BaseType;
  typedef detail::NilV0   NilType;
  typedef V0              ThisType;

  /* Construction **************************************************************/
  V0()                    : m_pBody(Nil().m_pBody)  { IncBody();}
  V0(BaseType* pBody)     : m_pBody(pBody)          { IncBody();}
  V0(const ThisType& rhs) : m_pBody(rhs.m_pBody)    { IncBody();}
  ~V0()                                             { DecBody();}

  ThisType& operator=(const ThisType& rhs)          { if (m_pBody != rhs.m_pBody)
                                                      { DecBody();
                                                        m_pBody = rhs.m_pBody;
                                                        IncBody();
                                                      }
                                                      return *this;
                                                    }

  bool IsEmpty()                       const        { return (*this == Nil());}
  bool operator! ()                    const        { return IsEmpty();}
  bool operator==(const ThisType& rhs) const        { return m_pBody == rhs.m_pBody;}
  bool operator!=(const ThisType& rhs) const        { return !(*this == rhs);}
  bool operator< (const ThisType& rhs) const         { return m_pBody < rhs.m_pBody;}
  void operator()()                                 { return m_pBody->operator()();} 

  static ThisType&     Nil()    { static ThisType nil(nilBody_());
                                  return nil;
                                }

private:
  /* Members *******************************************************************/
  BaseType* m_pBody;

  /* Methods *******************************************************************/
  void IncBody()                { m_pBody->Inc();}
  void DecBody()                { m_pBody->Dec();
                                  if (!m_pBody->Count())
                                    delete m_pBody;
                                }

  static NilType*  nilBody_()   { static NilType* body = new NilType;
                                  return body;
                                }
};

/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Callback object for return type with 0 arguments.
******************************************************************************/
template <typename R>
class R0
{
public:
  /* Typedefs ******************************************************************/
  typedef detail::BaseR0<R> BaseType;
  typedef detail::NilR0<R>  NilType;
  typedef R0<R>             ThisType;
  typedef R                 ReturnType;

  /* Construction **************************************************************/
  R0()                    : m_pBody(Nil().m_pBody)  { IncBody();}
  R0(BaseType* pBody)     : m_pBody(pBody)          { IncBody();}
  R0(const ThisType& rhs) : m_pBody(rhs.m_pBody)    { IncBody();}
  ~R0()                                             { DecBody();}

  ThisType& operator=(const ThisType& rhs)          { if (m_pBody != rhs.m_pBody)
                                                      { DecBody();
                                                        m_pBody = rhs.m_pBody;
                                                        IncBody();
                                                      }
                                                      return *this;
                                                    }

  bool IsEmpty()                       const        { return (*this == Nil());}
  bool operator! ()                    const        { return IsEmpty();}
  bool operator==(const ThisType& rhs) const        { return m_pBody == rhs.m_pBody;}
  bool operator!=(const ThisType& rhs) const        { return !(*this == rhs);}
  bool operator<(const ThisType& rhs) const         { return m_pBody < rhs.m_pBody;}
  ReturnType operator()()                           { return m_pBody->operator()();} 

  static ThisType&     Nil()                        { static ThisType nil(nilBody_());
                                                      return nil;
                                                    }
private:
  /* Members *******************************************************************/
  BaseType* m_pBody;

  /* Methods *******************************************************************/
  void IncBody()                { m_pBody->Inc();}
  void DecBody()                { m_pBody->Dec();
                                  if (!m_pBody->Count())
                                    delete m_pBody;
                                }

  static NilType*  nilBody_()   { static NilType* body = new NilType;
                                  return body;
                                }
};

/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Callback object for return void with 1 argument.
******************************************************************************/
template <typename A1>
class V1
{
public:
  /* Typedefs ******************************************************************/
  typedef detail::BaseV1<A1>  BaseType;
  typedef detail::NilV1<A1>   NilType;
  typedef V1<A1>              ThisType;

  /* Construction **************************************************************/
  V1()                    : m_pBody(Nil().m_pBody)  { IncBody();}
  V1(BaseType* pBody)     : m_pBody(pBody)          { IncBody();}
  V1(const ThisType& rhs) : m_pBody(rhs.m_pBody)    { IncBody();}
  ~V1()                                             { DecBody();}

  ThisType& operator=(const ThisType& rhs)          { if (m_pBody != rhs.m_pBody)
                                                      { DecBody();
                                                        m_pBody = rhs.m_pBody;
                                                        IncBody();
                                                      }
                                                      return *this;
                                                    }

  bool IsEmpty()                       const        { return (*this == Nil());}
  bool operator! ()                    const        { return IsEmpty();}
  bool operator==(const ThisType& rhs) const        { return m_pBody == rhs.m_pBody;}
  bool operator!=(const ThisType& rhs) const        { return !(*this == rhs);}
  bool operator<(const ThisType& rhs) const         { return m_pBody < rhs.m_pBody;}
  void operator()(A1 a1)                            { return m_pBody->operator()(a1);} 

  static ThisType&     Nil()    { static ThisType nil(nilBody_());
                                  return nil;
                                }

private:
  /* Members *******************************************************************/
  BaseType* m_pBody;

  /* Methods *******************************************************************/
  void IncBody()                { m_pBody->Inc();}
  void DecBody()                { m_pBody->Dec();
                                  if (!m_pBody->Count())
                                    delete m_pBody;
                                }

  static NilType*  nilBody_()   { static NilType* body = new NilType;
                                  return body;
                                }
};

/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Callback object for return type with 1 argument.
******************************************************************************/
template <typename R, typename A1>
class R1
{
public:
  /* Typedefs ******************************************************************/
  typedef detail::BaseR1<R, A1> BaseType;
  typedef detail::NilR1<R, A1>  NilType;
  typedef R1<R, A1>             ThisType;
  typedef R                     ReturnType;

  /* Construction **************************************************************/
  R1()                    : m_pBody(Nil().m_pBody)  { IncBody();}
  R1(BaseType* pBody)     : m_pBody(pBody)          { IncBody();}
  R1(const ThisType& rhs) : m_pBody(rhs.m_pBody)    { IncBody();}
  ~R1()                                             { DecBody();}

  ThisType& operator=(const ThisType& rhs)          { if (m_pBody != rhs.m_pBody)
                                                      { DecBody();
                                                        m_pBody = rhs.m_pBody;
                                                        IncBody();
                                                      }
                                                      return *this;
                                                    }

  bool IsEmpty()                       const        { return (*this == Nil());}
  bool operator! ()                    const        { return IsEmpty();}
  bool operator==(const ThisType& rhs) const        { return m_pBody == rhs.m_pBody;}
  bool operator!=(const ThisType& rhs) const        { return !(*this == rhs);}
  bool operator<(const ThisType& rhs) const         { return m_pBody < rhs.m_pBody;}
  ReturnType operator()(A1 a1)                      { return m_pBody->operator()(a1);} 

  static ThisType&     Nil()                        { static ThisType nil(nilBody_());
                                                      return nil;
                                                    }
private:
  /* Members *******************************************************************/
  BaseType* m_pBody;

  /* Methods *******************************************************************/
  void IncBody()                { m_pBody->Inc();}
  void DecBody()                { m_pBody->Dec();
                                  if (!m_pBody->Count())
                                    delete m_pBody;
                                }

  static NilType*  nilBody_()   { static NilType* body = new NilType;
                                  return body;
                                }
};

/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Callback object for return void with 2 arguments.
******************************************************************************/
template <typename A1, typename A2>
class V2
{
public:
  /* Typedefs ******************************************************************/
  typedef detail::BaseV2<A1, A2>  BaseType;
  typedef detail::NilV2<A1, A2>   NilType;
  typedef V2<A1, A2>              ThisType;

  /* Construction **************************************************************/
  V2()                    : m_pBody(Nil().m_pBody)  { IncBody();}
  V2(BaseType* pBody)     : m_pBody(pBody)          { IncBody();}
  V2(const ThisType& rhs) : m_pBody(rhs.m_pBody)    { IncBody();}
  ~V2()                                             { DecBody();}

  ThisType& operator=(const ThisType& rhs)          { if (m_pBody != rhs.m_pBody)
                                                      { DecBody();
                                                        m_pBody = rhs.m_pBody;
                                                        IncBody();
                                                      }
                                                      return *this;
                                                    }

  bool IsEmpty()                       const        { return (*this == Nil());}
  bool operator! ()                    const        { return IsEmpty();}
  bool operator==(const ThisType& rhs) const        { return m_pBody == rhs.m_pBody;}
  bool operator!=(const ThisType& rhs) const        { return !(*this == rhs);}
  bool operator<(const ThisType& rhs) const         { return m_pBody < rhs.m_pBody;}
  void operator()(A1 a1, A2 a2)                     { return m_pBody->operator()(a1, a2);} 

  static ThisType&     Nil()    { static ThisType nil(nilBody_());
                                  return nil;
                                }

private:
  /* Members *******************************************************************/
  BaseType* m_pBody;

  /* Methods *******************************************************************/
  void IncBody()                { m_pBody->Inc();}
  void DecBody()                { m_pBody->Dec();
                                  if (!m_pBody->Count())
                                    delete m_pBody;
                                }

  static NilType*  nilBody_()   { static NilType* body = new NilType;
                                  return body;
                                }
};

/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Callback object for return type with 2 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2>
class R2
{
public:
  /* Typedefs ******************************************************************/
  typedef detail::BaseR2<R, A1, A2> BaseType;
  typedef detail::NilR2<R, A1, A2>  NilType;
  typedef R2<R, A1, A2>             ThisType;
  typedef R                         ReturnType;

  /* Construction **************************************************************/
  R2()                    : m_pBody(Nil().m_pBody)  { IncBody();}
  R2(BaseType* pBody)     : m_pBody(pBody)          { IncBody();}
  R2(const ThisType& rhs) : m_pBody(rhs.m_pBody)    { IncBody();}
  ~R2()                                             { DecBody();}

  ThisType& operator=(const ThisType& rhs)          { if (m_pBody != rhs.m_pBody)
                                                      { DecBody();
                                                        m_pBody = rhs.m_pBody;
                                                        IncBody();
                                                      }
                                                      return *this;
                                                    }

  bool IsEmpty()                       const        { return (*this == Nil());}
  bool operator! ()                    const        { return IsEmpty();}
  bool operator==(const ThisType& rhs) const        { return m_pBody == rhs.m_pBody;}
  bool operator!=(const ThisType& rhs) const        { return !(*this == rhs);}
  bool operator<(const ThisType& rhs) const         { return m_pBody < rhs.m_pBody;}
  ReturnType operator()(A1 a1, A2 a2)               { return m_pBody->operator()(a1, a2);} 

  static ThisType&     Nil()                        { static ThisType nil(nilBody_());
                                                      return nil;
                                                    }
private:
  /* Members *******************************************************************/
  BaseType* m_pBody;

  /* Methods *******************************************************************/
  void IncBody()                { m_pBody->Inc();}
  void DecBody()                { m_pBody->Dec();
                                  if (!m_pBody->Count())
                                    delete m_pBody;
                                }

  static NilType*  nilBody_()   { static NilType* body = new NilType;
                                  return body;
                                }
};

/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Callback object for return void with 3 arguments.
******************************************************************************/
template <typename A1, typename A2, typename A3>
class V3
{
public:
  /* Typedefs ******************************************************************/
  typedef detail::BaseV3<A1, A2, A3>  BaseType;
  typedef detail::NilV3<A1, A2, A3>   NilType;
  typedef V3<A1, A2, A3>              ThisType;

  /* Construction **************************************************************/
  V3()                    : m_pBody(Nil().m_pBody)  { IncBody();}
  V3(BaseType* pBody)     : m_pBody(pBody)          { IncBody();}
  V3(const ThisType& rhs) : m_pBody(rhs.m_pBody)    { IncBody();}
  ~V3()                                             { DecBody();}

  ThisType& operator=(const ThisType& rhs)          { if (m_pBody != rhs.m_pBody)
                                                      { DecBody();
                                                        m_pBody = rhs.m_pBody;
                                                        IncBody();
                                                      }
                                                      return *this;
                                                    }

  bool IsEmpty()                       const        { return (*this == Nil());}
  bool operator! ()                    const        { return IsEmpty();}
  bool operator==(const ThisType& rhs) const        { return m_pBody == rhs.m_pBody;}
  bool operator!=(const ThisType& rhs) const        { return !(*this == rhs);}
  bool operator< (const ThisType& rhs) const        { return m_pBody < rhs.m_pBody;}
  void operator()(A1 a1, A2 a2, A3 a3)              { return m_pBody->operator()(a1, a2, a3);} 

  static ThisType&     Nil()    { static ThisType nil(nilBody_());
                                  return nil;
                                }

private:
  /* Members *******************************************************************/
  BaseType* m_pBody;

  /* Methods *******************************************************************/
  void IncBody()                { m_pBody->Inc();}
  void DecBody()                { m_pBody->Dec();
                                  if (!m_pBody->Count())
                                    delete m_pBody;
                                }

  static NilType*  nilBody_()   { static NilType* body = new NilType;
                                  return body;
                                }
};

/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Callback object for return type with 3 arguments.
******************************************************************************/
template <typename R, typename A1, typename A2, typename A3>
class R3
{
public:
  /* Typedefs ******************************************************************/
  typedef detail::BaseR3<R, A1, A2, A3> BaseType;
  typedef detail::NilR3<R, A1, A2, A3>  NilType;
  typedef R3<R, A1, A2, A3>             ThisType;
  typedef R                             ReturnType;

  /* Construction **************************************************************/
  R3()                    : m_pBody(Nil().m_pBody)  { IncBody();}
  R3(BaseType* pBody)     : m_pBody(pBody)          { IncBody();}
  R3(const ThisType& rhs) : m_pBody(rhs.m_pBody)    { IncBody();}
  ~R3()                                             { DecBody();}

  ThisType& operator=(const ThisType& rhs)          { if (m_pBody != rhs.m_pBody)
                                                      { DecBody();
                                                        m_pBody = rhs.m_pBody;
                                                        IncBody();
                                                      }
                                                      return *this;
                                                    }

  bool IsEmpty()                       const        { return (*this == Nil());}
  bool operator! ()                    const        { return IsEmpty();}
  bool operator==(const ThisType& rhs) const        { return m_pBody == rhs.m_pBody;}
  bool operator!=(const ThisType& rhs) const        { return !(*this == rhs);}
  bool operator< (const ThisType& rhs) const        { return m_pBody < rhs.m_pBody;}
  ReturnType operator()(A1 a1, A2 a2, A3 a3)        { return m_pBody->operator()(a1, a2, a3);} 

  static ThisType&     Nil()                        { static ThisType nil(nilBody_());
                                                      return nil;
                                                    }
private:
  /* Members *******************************************************************/
  BaseType* m_pBody;

  /* Methods *******************************************************************/
  void IncBody()                { m_pBody->Inc();}
  void DecBody()                { m_pBody->Dec();
                                  if (!m_pBody->Count())
                                    delete m_pBody;
                                }

  static NilType*  nilBody_()   { static NilType* body = new NilType;
                                  return body;
                                }
};

} // namespace callback_type
} // namespace pbl

#endif //BLCALLBACKTYPE_H_INCLUDED


