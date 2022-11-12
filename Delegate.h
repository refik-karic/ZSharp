#pragma once

#include "ZAssert.h"
#include "List.h"

namespace ZSharp {

template<class... Signature>
class Delegate final {
  public:
  Delegate() : mObjPtr(nullptr), mClassSignature(nullptr), mFreeFunctionSignature(nullptr) {}

  bool operator==(const Delegate& rhs) const {
    return (mObjPtr == rhs.mObjPtr) && 
      (mFreeFunctionSignature == rhs.mFreeFunctionSignature) && 
      (mClassSignature == rhs.mClassSignature);
  }

  static Delegate FromFreeFunction(void (*funcPtr)(Signature...)) {
    Delegate delegate(funcPtr);
    return delegate;
  }

  template<class T, void (T::*MemberFunc)(Signature...)>
  static Delegate FromMember(T* objectPtr) {
    Delegate delegate(objectPtr, &InvokeMemberFunc<T, MemberFunc>);
    return delegate;
  }

  void operator()(Signature... args) const {
    if (mFreeFunctionSignature != nullptr) {
      (*mFreeFunctionSignature)(args...);
    }
    else if(mClassSignature != nullptr) {
      (*mClassSignature)(mObjPtr, args...);
    }
    else {
      ZAssert(false); // Calling an unbound delegate.
    }
  }

  private:
  typedef void (*FreeFunctionSignature)(Signature...);
  typedef void (*ClassSignature)(void* objectPtr, Signature...);
  
  void* mObjPtr;
  FreeFunctionSignature mFreeFunctionSignature;
  ClassSignature mClassSignature;

  Delegate(void* objPtr, ClassSignature classSignature) : mObjPtr(objPtr), mClassSignature(classSignature), mFreeFunctionSignature(nullptr) {};

  Delegate(FreeFunctionSignature freeFuncSignature) : mObjPtr(nullptr), mClassSignature(nullptr), mFreeFunctionSignature(freeFuncSignature) {};

  template<class T, void (T::*MemberFunc)(Signature...)>
  static void InvokeMemberFunc(void* objectPtr, Signature... args) {
    T* typedPtr = static_cast<T*>(objectPtr);
    (typedPtr->*MemberFunc)(args...);
  }
};

template<class... Signature>
class BroadcastDelegate final {
  public:
  BroadcastDelegate() {}

  // TODO: It might make this cleaner to have similar delegate creation helpers like above.
  void Add(const Delegate<Signature...>& delegate) {
    if (mCallList.Contains(delegate)) {
      ZAssert(false);
    }
    else {
      mCallList.Add(delegate);
    }
  }

  void Remove(const Delegate<Signature...>& delegate) {
    mCallList.Remove(delegate);
  }

  void Broadcast(Signature... args) {
    for (Delegate<Signature...>& delegate : mCallList) {
      delegate(args...);
    }
  }

  private:
  List<Delegate<Signature...>> mCallList;
};

// TODO: Create typed return delegate.

}
