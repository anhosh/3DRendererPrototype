#pragma once

#include <Util/NotNull.hpp>

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
    void erase(); \
    size_t index = SIZE_MAX; \
  private: \
    NotNull<OwnerClass> owner; \
  };

#define DEFINE_HANDLE_ITEM_FUNCTIONS(OwnerClass, ItemType, container) \
  template <> ItemType& OwnerClass::Handle<ItemType>::get() { \
    return owner->container.at(index); \
  } \
  template <> void OwnerClass::Handle<ItemType>::erase() { \
    owner->container.erase(index); \
  }
