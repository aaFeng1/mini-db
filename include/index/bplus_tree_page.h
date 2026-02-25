#pragma once

#include "common/page.h"
#include "storage/table_heap.h"
#include <cstddef>
#include <cstdint>
#include <vector>
namespace mini {

// 无法支持string的key类型，因为string的长度不固定，无法直接存储在page中

struct BPlusTreePageHeader {
  page_id_t parent_page_id;
  uint16_t key_count;
  uint16_t max_key_count;
  page_id_t page_id;
  page_id_t next_page_id; // only for leaf page
  bool is_leaf;
};

class BPlusTreePage {
public:
  static BPlusTreePage *From(Page *page);

  page_id_t GetParentPageId() const;
  void SetParentPageId(page_id_t parent_page_id);
  uint16_t GetKeyCount() const;
  void SetKeyCount(uint16_t key_count);
  uint16_t GetMaxKeyCount() const;
  void SetMaxKeyCount(uint16_t max_key_count);
  page_id_t GetPageId() const;
  void SetPageId(page_id_t page_id);
  page_id_t GetNextPageId() const;
  void SetNextPageId(page_id_t next_page_id);
  bool IsLeaf() const;

protected:
  BPlusTreePageHeader header_;
};

template <typename KeyType, typename ValueType, typename Comparator>
class BPlusTreeLeafPage : public BPlusTreePage {
public:
  using MappingType = struct {
    KeyType key;
    ValueType value;
  };

  static BPlusTreeLeafPage *From(Page *page) {
    return reinterpret_cast<BPlusTreeLeafPage *>(page->GetData());
  }

  static constexpr uint16_t TEST_MAX_KEY_COUNT = 6;

  static constexpr uint16_t MAX_KEY_COUNT =
#ifdef TEST_FOR_BPLUS_TREE
      TEST_MAX_KEY_COUNT;
#else
      (PAGE_SIZE - sizeof(BPlusTreePageHeader)) / sizeof(MappingType);
#endif

  KeyType KeyAt(uint16_t index) const;
  ValueType ValueAt(uint16_t index) const;
  void SetKeyAt(uint16_t index, const KeyType &key);
  void SetValueAt(uint16_t index, const ValueType &value);

  bool Lookup(const KeyType &key, std::vector<ValueType> *value) const;
  bool Insert(const KeyType &key, const ValueType &value);
  bool Remove(const KeyType &key) = delete; //暂不实现删除功能

  bool IsFull() const { return this->GetKeyCount() >= MAX_KEY_COUNT; }
  bool Split(BPlusTreeLeafPage *new_page);

  void Init(page_id_t page_id) {
    this->SetParentPageId(INVALID_PAGE_ID);
    this->SetKeyCount(0);
    this->SetMaxKeyCount(MAX_KEY_COUNT);
    this->SetPageId(page_id);
    this->SetNextPageId(INVALID_PAGE_ID);
    this->header_.is_leaf = true;
  }

  void Print(std::ostream &os) const; // for debug

private:
  MappingType array_[1];
};

template <typename KeyType, typename ValueType, typename Comparator>
class BPlusTreeInternalPage : public BPlusTreePage {
public:
  using MappingType = struct {
    KeyType key;
    ValueType value;
  };

  static BPlusTreeInternalPage *From(Page *page) {
    return reinterpret_cast<BPlusTreeInternalPage *>(page->GetData());
  }

  static constexpr uint16_t TEST_MAX_KEY_COUNT = 6;

  static constexpr uint16_t MAX_KEY_COUNT =
#ifdef TEST_FOR_BPLUS_TREE
      TEST_MAX_KEY_COUNT;
#else
      (PAGE_SIZE - sizeof(BPlusTreePageHeader)) / sizeof(MappingType);
#endif

  KeyType KeyAt(uint16_t index) const;
  ValueType ValueAt(uint16_t index) const;
  void SetKeyAt(uint16_t index, const KeyType &key);
  void SetValueAt(uint16_t index, const ValueType &value);

  bool Insert(const KeyType &key, const ValueType &value);
  bool
  InsertAfter(const ValueType &old_value, const KeyType &new_key,
              const ValueType &new_value);  // insert new_key after old_value
  bool Remove(const KeyType &key) = delete; //暂不实现删除功能

  bool IsFull() const { return this->GetKeyCount() >= MAX_KEY_COUNT; }
  bool Split(BPlusTreeInternalPage *new_page);

  void Init(page_id_t page_id) {
    this->SetParentPageId(INVALID_PAGE_ID);
    this->SetKeyCount(0);
    this->SetMaxKeyCount(MAX_KEY_COUNT);
    this->SetPageId(page_id);
    this->SetNextPageId(INVALID_PAGE_ID);
    this->header_.is_leaf = false;
  }

  void Print(std::ostream &os) const; // for debug

private:
  MappingType array_[1];
};

} // namespace mini
