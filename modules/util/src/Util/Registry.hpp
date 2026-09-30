#pragma once

#include <Util/NotNull.hpp>

#include <cassert>
#include <unordered_map>
#include <unordered_set>

using RegItemID = uint32_t;

template <typename T>
class Registry {
public:
  using ItemType = T;

  ///  Iterator-related typedefs.
  using pointer              = ItemType*;
  using const_pointer        = const ItemType*;
  using reference            = ItemType&;
  using const_reference      = const ItemType&;
  using iterator             = std::unordered_map<RegItemID, ItemType>::iterator;
  using const_iterator       = std::unordered_map<RegItemID, ItemType>::const_iterator;
  using local_iterator       = std::unordered_map<RegItemID, ItemType>::local_iterator;
  using const_local_iterator = std::unordered_map<RegItemID, ItemType>::const_local_iterator;
  using size_type            = std::unordered_map<RegItemID, ItemType>::size_type;
  using difference_type      = std::unordered_map<RegItemID, ItemType>::difference_type;

  class Handle {
    friend class Registry;

  public:
    // using ItemType = ItemType;

    constexpr Handle(const Handle&) = default;
    constexpr Handle(Handle&&) = default;
    constexpr ~Handle() = default;

    [[nodiscard]] static constexpr Handle null() {
      return Handle();
    }

    constexpr Handle& operator=(const Handle&) = default;
    constexpr Handle& operator=(Handle&&) = default;

    constexpr ItemType& operator*() { return get(); }
    constexpr ItemType* operator->() { return &get(); }
    constexpr const ItemType& operator*() const { return get(); }
    constexpr const ItemType* operator->() const { return &get(); }

    void erase() {
      NotNull(mOwner)->erase(mItemID);
    }

    [[nodiscard]] ItemType& get() {
      return NotNull(mOwner)->at(mItemID);
    }

    [[nodiscard]] const ItemType& get() const {
      return NotNull(mOwner)->at(mItemID);
    }

    [[nodiscard]] ItemType& getOrDefault(ItemType& defaultValue) {
      return this->isNull() ? defaultValue : this->get();
    }

    [[nodiscard]] const ItemType& getOrDefault(const ItemType& defaultValue) const {
      return this->isNull() ? defaultValue : this->get();
    }

    [[nodiscard]] ItemType& getOrDefault(Handle defaultValueHandle) {
      assert(!defaultValueHandle.isNull());
      return this->isNull() ? defaultValueHandle.get() : this->get();
    }

    [[nodiscard]] const ItemType& getOrDefault(const Handle defaultValueHandle) const {
      assert(!defaultValueHandle.isNull());
      return this->isNull() ? defaultValueHandle.get() : this->get();
    }

    [[nodiscard]] RegItemID itemID() const {
      return mItemID;
    }

    [[nodiscard]] bool isNull() const {
      return mOwner == nullptr;
    }

    [[nodiscard]] bool exists() const {
      return !this->isNull() && mOwner->contains(mItemID);
    }

    bool operator==(const Handle& other) const {
      if (mOwner == nullptr && other.mOwner == nullptr) {
        return true; // All null handles are equivalent.
      }
      return mOwner == other.mOwner && mItemID == other.mItemID;
    }

  private:
    constexpr Handle() = default;
    constexpr Handle(NotNull<Registry> owner, const RegItemID id) : mOwner(owner), mItemID(id) {}

  private:
    Registry* mOwner = nullptr;
    RegItemID mItemID = 0;
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
