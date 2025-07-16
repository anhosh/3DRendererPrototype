#pragma once

#include <Util/NotNull.hpp>

#include <unordered_map>

using RegItemID = uint32_t;

template <typename ItemType>
class Registry {
public:

  ///  Iterator-related typedefs.
  using pointer              = typename std::unordered_map<RegItemID, ItemType>::pointer;
  using const_pointer        = typename std::unordered_map<RegItemID, ItemType>::const_pointer;
  using reference            = typename std::unordered_map<RegItemID, ItemType>::reference;
  using const_reference      = typename std::unordered_map<RegItemID, ItemType>::const_reference;
  using iterator             = typename std::unordered_map<RegItemID, ItemType>::iterator;
  using const_iterator       = typename std::unordered_map<RegItemID, ItemType>::const_iterator;
  using local_iterator       = typename std::unordered_map<RegItemID, ItemType>::local_iterator;
  using const_local_iterator = typename std::unordered_map<RegItemID, ItemType>::const_local_iterator;
  using size_type            = typename std::unordered_map<RegItemID, ItemType>::size_type;
  using difference_type      = typename std::unordered_map<RegItemID, ItemType>::difference_type;

  class Handle {
    friend class Registry;

  public:
    Handle(const Handle&) = default;
    Handle(Handle&&) = default;

    Handle& operator=(const Handle&) = default;
    Handle& operator=(Handle&&) = default;

    ItemType& operator*() { return get(); }
    ItemType* operator->() { return &get(); }
    const ItemType& operator*() const { return get(); }
    const ItemType* operator->() const { return &get(); }

    void erase() {
      mOwner->erase(mID);
    }

    [[nodiscard]] ItemType& get() {
      return mOwner->at(mID);
    }

    [[nodiscard]] const ItemType& get() const {
      return mOwner->at(mID);
    }

    [[nodiscard]] RegItemID id() const {
      return mID;
    }

    [[nodiscard]] bool exists() {
      return mOwner->contains(mID);
    }

    auto operator<=>(const Handle&) const = default;

  private:
    Handle(NotNull<Registry> owner, const RegItemID id) : mOwner(owner), mID(id) {}

  private:
    NotNull<Registry> mOwner;
    RegItemID mID;
  };

  iterator begin() { return mItems.begin(); }
  const_iterator begin() const { return mItems.begin(); }
  const_iterator cbegin() const { return mItems.cbegin(); }
  iterator end() { return mItems.end(); }
  const_iterator end() const { return mItems.end(); }
  const_iterator cend() const { return mItems.cend(); }

  Handle add(ItemType&& item) {
    mItems.emplace(mNextItemID, std::forward<ItemType>(item));
    return Handle(this, mNextItemID++);
  }

  void erase(const RegItemID id) {
    mItems.erase(id);
  }

  void clear() {
    mItems.clear();
  }

  ItemType& at(const RegItemID id) {
    return mItems.at(id);
  }

  const ItemType& at(const RegItemID id) const {
    return mItems.at(id);
  }

  bool contains(const RegItemID id) {
    return mItems.contains(id);
  }

private:
  std::unordered_map<RegItemID, ItemType> mItems;
  RegItemID mNextItemID = 0;
};
