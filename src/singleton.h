#pragma once

#include <memory>
#include "macros.h"

// http://cflat-inc.hatenablog.com/entry/2014/03/04/214608
template <class T> class Singleton {
public:
  virtual ~Singleton() = default;
  static T &singleton() {
    static typename T::singleton_pointer_type s_singleton(T::createInstance());
    return getReference(s_singleton);
  }

private:
  typedef std::unique_ptr<T> singleton_pointer_type;

  inline static T *createInstance() { return new T(); }
  inline static T &getReference(const singleton_pointer_type &ptr) {
    return *ptr;
  }

protected:
  Singleton() {}

private:
  DISABLE_COPY_AND_MOVE(Singleton)
};
