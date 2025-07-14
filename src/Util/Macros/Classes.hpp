#pragma once

#include <Util/NotNull.hpp>

#include <cstdint>

#define DECLARE_ITEM_HANDLE(OwnerClass) \
  template <typename Item> \
  class Handle { \
    friend class OwnerClass; \
  private: \
    Handle(size_t index, NotNull<OwnerClass> owner) : index(index), owner(owner) {} \
  public: \
    Handle(const Handle& other) = default; \
    Handle& operator=(const Handle& other) = default; \
    Item& get(); \
    size_t index = SIZE_MAX; \
  private: \
    NotNull<OwnerClass> owner; \
  };

#define DEFINE_HANDLE_ITEM_GET(OwnerClass, ItemType, container) \
  template<> \
  ItemType& OwnerClass::Handle<ItemType>::get() { \
    return owner->container[index]; \
  }
