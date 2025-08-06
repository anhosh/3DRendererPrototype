#pragma once

#include <Util/NotNull.hpp>

#include <unordered_map>
#include <unordered_set>

using RegItemID = uint32_t;

template <typename ItemType>
class Registry {
public:
  ///  Iterator-related typedefs.
  using pointer              = ItemType*;
  using const_pointer        = const ItemType*;
  using reference            = ItemType&;
  using const_reference      = const ItemType&;
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
    ~Handle() = default;

    Handle& operator=(const Handle&) = default;
    Handle& operator=(Handle&&) = default;

    ItemType& operator*() { return get(); }
    ItemType* operator->() { return &get(); }
    const ItemType& operator*() const { return get(); }
    const ItemType* operator->() const { return &get(); }

    void erase() {
      mOwner->erase(mItemID);
    }

    [[nodiscard]] ItemType& get() {
      return mOwner->at(mItemID);
    }

    [[nodiscard]] const ItemType& get() const {
      return mOwner->at(mItemID);
    }

    [[nodiscard]] RegItemID itemID() const {
      return mItemID;
    }

    [[nodiscard]] bool exists() const {
      return mOwner->contains(mItemID);
    }

    auto operator<=>(const Handle&) const = default;

  private:
    Handle(NotNull<Registry> owner, const RegItemID id) : mOwner(owner), mItemID(id) {}

  private:
    NotNull<Registry> mOwner;
    RegItemID mItemID;
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

  [[nodiscard]] ItemType& at(const RegItemID id) {
    return mItems.at(id);
  }

  [[nodiscard]] const ItemType& at(const RegItemID id) const {
    return mItems.at(id);
  }

  [[nodiscard]] bool contains(const RegItemID id) const {
    return mItems.contains(id);
  }

  [[nodiscard]] bool empty() const {
    return mItems.empty();
  }

  [[nodiscard]] size_t size() const {
    return mItems.size();
  }

private:
  std::unordered_map<RegItemID, ItemType> mItems;
  RegItemID mNextItemID = 0;
};
