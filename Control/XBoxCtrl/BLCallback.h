/* BLCallback.h ***************************************************************
Copyright:  2009 Paul Watt
Author:     Paul Watt
Date:       6/17/2009
Purpose:    Generalized Template definitions to allow callbacks of any type
            to be transferred and assigned in the portable bridge.

            These defintions will allow pointers to member functions without
            the requirement of a static member variable.  Just the same a 
            Standalone function could be assigned in the same context.

Example Usage:
  
  // 1) For ease of use add a typedef of the callback type.
  //    This will create a callback for a function  that takes 1 arguement 
  //    of type char* and returns a size_t type.
  typedef pbl::callback_type::R1<size_t, const char*> fnR1;

  // 2) Declare callback variable.
  //    Use the BLCallback class, and request the format of call back:
  //        R1(...), callback that takes 1 parameter and returns a value
  //    Assign, the callback function in this request, ::strlen in this example
  //    The first parameter "(fnR1*)0", is used as a hint to the compiler for
  //    the type of variables.  Otherwise they would have to be supplied explicitly:
  //        BLCallback::R1<size_t, const char*>(::strlen);
  fnR1 fn = BLCallback::R1((fnR1*)0, ::strlen);

  // 3) Call function.
  //    This example would return the value 11.
  size_t len = fn("Hello World");


Example Usage with Member Function:
  class Object
  {
  public:
    DWORD thread_proc(LPARAM lParam)
    {
      // Thread processing code...
      return 0;
    }
  };

  // Instance of object is created at some point.
  Object obj;

  typedef pbl::callback_type::R1<DWORD, LPARAM> fnR1Ex2;
  // For Object member function pointers, include object reference and member reference.
  fnR1Ex2 memFn = BLCallback::R1((fnR1Ex2*)0, &obj, &Object::thread_proc);

  ... 

  // when memFn is invoked, it will call thread_proc, from inside the instance of obj
  // - No need to pass object pointer as lParam of thread proc. 
  // - All internal state of obj is accessible to thread_proc instance.
  // - LPARAM can represent instance data for the thread.
  bool  isProcessSpecial = false;
  DWORD retVal = memFn((LPARAM)isProcessSpecial);


******************************************************************************/
#ifndef BLCALLBACK_H_INCLUDED
#define BLCALLBACK_H_INCLUDED
#include "BLCallbackType.h"

//#include "boost/function.hpp"
//#include "boost/bind.hpp"

// Adopt boost callback model.
//boost::function<size_t(const char*)> slen;
//slen = ::strlen;
//size_t len = slen("Hello world");
//
//class Object
//{
//	DWORD thread_proc(LPARAM lParam)
//	{
//		return 0;
//	}
//};
//
//Object obj;
//boost::function<DWORD(LPARAM lParam)> proc;
//proc = boost::bind(&object::thread_proc, &obj);
//
//proc(0);

namespace pbl
{

/* Class **********************************************************************
Author:     Paul Watt
Date:       6/18/2009
Purpose:    Class wrapper for typedef purposes.
            Use overloaded calls in order to create callback objects of specific type.

            Functions that start with 'R' indicate callbacks with a return value.
            Functions that start with 'V' indicate callbacks that return void.

            The number after the R or V indicate the number of parameters the
            callback will support.

Example:    R0, represents a function that takes 0 argument and returns a value.
            v2, represents a function that takes 2 argument and returns void.
******************************************************************************/
class Callback
{
public:

  /* Return void, 0 argument ***********************************************/
  template <typename Function>
  static callback_type::V0 
  V0(callback_type::V0* hint, Function f)
  {
    callback_type::detail::FnV0<Function>* pBody = 
      new callback_type::detail::FnV0<Function>(f);
    return pBody ? callback_type::V0(pBody) : callback_type::V0::Nil();
  } 

  template <class Obj, class Member>
  static callback_type::V0 
  V0(callback_type::V0* hint, Obj* pObj, Member member)
  {
    callback_type::detail::MemV0<Obj,Member>* pBody = 
      new callback_type::detail::MemV0<Obj,Member>(pObj,member);
    return pBody ? callback_type::V0(pBody) : callback_type::V0::Nil();
  } 

  /* Return Type, 0 argument ***********************************************/
  template <typename R, typename Function>
  static callback_type::R0<R> 
  R0(callback_type::R0<R>* hint, Function f)
  {
    callback_type::detail::FnR0<R,Function>* pBody = 
      new callback_type::detail::FnR0<R,Function>(f);
    return pBody ? callback_type::R0<R>(pBody) : callback_type::R0<R>::Nil();
  } 

  template <typename R, class Obj, class Member>
  static callback_type::R0<R> 
  R0(callback_type::R0<R>* hint, Obj* pObj, Member member)
  {
    callback_type::detail::MemR0<R,Obj,Member>* pBody = 
      new callback_type::detail::MemR0<R,Obj,Member>(pObj,member);
    return pBody ? callback_type::R0<R>(pBody) : callback_type::R0<R>::Nil();
  } 

  /* Return void, 1 arguement ************************************************/
  template <typename A1, typename Function>
  static callback_type::V1<A1> 
  V1(callback_type::V1<A1>* hint, Function f)
  {
    callback_type::detail::FnV1<A1, Function>* pBody;
    pBody = 
      new callback_type::detail::FnV1<A1, Function>(f);
    return pBody ? callback_type::V1<A1>(pBody) : callback_type::V1<A1>::Nil();
  } 

  template <typename A1, class Obj, class Member>
  static callback_type::V1<A1> 
  V1(callback_type::V1<A1>* hint, Obj* pObj, Member member)
  {
    callback_type::detail::MemV1<A1, Obj,Member>* pBody = 
      new callback_type::detail::MemV1<A1, Obj,Member>(pObj,member);
    return pBody ? callback_type::V1<A1>(pBody) : callback_type::V1<A1>::Nil();
  } 

  /* Return Type, 1 argument *************************************************/
  template <typename R, typename A1, typename Function>
  static callback_type::R1<R, A1> 
  R1(callback_type::R1<R, A1>* hint, Function f)
  {
    callback_type::detail::FnR1<R,A1,Function>* pBody = 
      new callback_type::detail::FnR1<R,A1,Function>(f);
    return pBody ? callback_type::R1<R,A1>(pBody) : callback_type::R1<R,A1>::Nil();
  } 

  template <typename R, typename A1, class Obj, class Member>
  static callback_type::R1<R,A1> 
  R1(callback_type::R1<R,A1>* hint, Obj* pObj, Member member)
  {
    callback_type::detail::MemR1<R,A1,Obj,Member>* pBody = 
      new callback_type::detail::MemR1<R,A1,Obj,Member>(pObj,member);
    return pBody ? callback_type::R1<R,A1>(pBody) : callback_type::R1<R,A1>::Nil();
  } 

  /* Return void, 2 argument ***********************************************/
  template <typename A1, typename A2, typename Function>
  static callback_type::V2<A1, A2> 
  V2(callback_type::V2<A1, A2>* hint, Function f)
  {
    callback_type::detail::FnV2<A1, A2, Function>* pBody = 
      new callback_type::detail::FnV2<A1, A2, Function>(f);
    return pBody ? callback_type::V2<A1, A2>(pBody) : callback_type::V2<A1, A2>::Nil();
  } 

  template <typename A1, typename A2, class Obj, class Member>
  static callback_type::V2<A1, A2> 
  V2(callback_type::V2<A1, A2>* hint, Obj* pObj, Member member)
  {
    callback_type::detail::MemV2<A1, A2, Obj,Member>* pBody = 
      new callback_type::detail::MemV2<A1, A2, Obj,Member>(pObj,member);
    return pBody ? callback_type::V2<A1, A2>(pBody) : callback_type::V2<A1, A2>::Nil();
  } 

  /* Return Type, 2 argument ***********************************************/
  template <typename R, typename A1, typename A2, typename Function>
  static callback_type::R2<R, A1, A2> 
  R2(callback_type::R2<R, A1, A2>* hint, Function f)
  {
    callback_type::detail::FnR2<R,A1, A2,Function>* pBody = 
      new callback_type::detail::FnR2<R,A1, A2,Function>(f);
    return pBody ? callback_type::R2<R,A1, A2>(pBody) : callback_type::R2<R,A1, A2>::Nil();
  } 

  template <typename R, typename A1, typename A2, class Obj, class Member>
  static callback_type::R2<R,A1, A2> 
  R2(callback_type::R2<R,A1, A2>* hint, Obj* pObj, Member member)
  {
    callback_type::detail::MemR2<R,A1, A2,Obj,Member>* pBody = 
      new callback_type::detail::MemR2<R,A1, A2,Obj,Member>(pObj,member);
    return pBody ? callback_type::R2<R,A1, A2>(pBody) : callback_type::R2<R,A1, A2>::Nil();
  } 

  /* Return void, 3 argument ***********************************************/
  template <typename A1, typename A2, typename A3, typename Function>
  static callback_type::V3<A1, A2, A3> 
  V3(callback_type::V3<A1, A2, A3>* hint, Function f)
  {
    callback_type::detail::FnV3<A1, A2, A3, Function>* pBody = 
      new callback_type::detail::FnV3<A1, A2, A3, Function>(f);
    return pBody ? callback_type::V3<A1, A2, A3>(pBody) : callback_type::V3<A1, A2, A3>::Nil();
  } 

  template <typename A1, typename A2, typename A3, class Obj, class Member>
  static callback_type::V3<A1, A2, A3> 
  V3(callback_type::V3<A1, A2, A3>* hint, Obj* pObj, Member member)
  {
    callback_type::detail::MemV3<A1, A2, A3, Obj,Member>* pBody = 
      new callback_type::detail::MemV3<A1, A2, A3, Obj,Member>(pObj,member);
    return pBody ? callback_type::V3<A1, A2, A3>(pBody) : callback_type::V3<A1, A2, A3>::Nil();
  } 

  /* Return Type, 3 argument ***********************************************/
  template <typename R, typename A1, typename A2, typename A3, typename Function>
  static callback_type::R3<R, A1, A2, A3> 
  R3(callback_type::R3<R, A1, A2, A3>* hint, Function f)
  {
    callback_type::detail::FnR3<R,A1, A2, A3, Function>* pBody = 
      new callback_type::detail::FnR3<R,A1, A2, A3, Function>(f);
    return pBody ? callback_type::R3<R,A1, A2, A3>(pBody) : callback_type::R3<R,A1, A2, A3>::Nil();
  } 

  template <typename R, typename A1, typename A2, typename A3, class Obj, class Member>
  static callback_type::R3<R,A1, A2, A3> 
  R3(callback_type::R3<R,A1, A2, A3>* hint, Obj* pObj, Member member)
  {
    callback_type::detail::MemR3<R,A1, A2, A3,Obj,Member>* pBody = 
      new callback_type::detail::MemR3<R,A1, A2, A3, Obj,Member>(pObj,member);
    return pBody ? callback_type::R3<R,A1, A2, A3>(pBody) : callback_type::R3<R,A1, A2, A3>::Nil();
  } 

};

} // namespace pbl

/* Typedefs ******************************************************************/
typedef pbl::Callback  BLCallback;

#endif //BLCALLBACK_H_INCLUDED


