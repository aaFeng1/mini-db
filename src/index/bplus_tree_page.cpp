#include "index/bplus_tree_page.h"
#include "common/comparator.h"
#include <cstdint>

namespace mini {

BPlusTreePage *BPlusTreePage::From(Page *page) {
  return reinterpret_cast<BPlusTreePage *>(page->GetData());
}

page_id_t BPlusTreePage::GetParentPageId() const {
  return header_.parent_page_id;
}

void BPlusTreePage::SetParentPageId(page_id_t parent_page_id) {
  header_.parent_page_id = parent_page_id;
}

uint16_t BPlusTreePage::GetKeyCount() const { return header_.key_count; }

void BPlusTreePage::SetKeyCount(uint16_t key_count) {
  header_.key_count = key_count;
}

uint16_t BPlusTreePage::GetMaxKeyCount() const { return header_.max_key_count; }

void BPlusTreePage::SetMaxKeyCount(uint16_t max_key_count) {
  header_.max_key_count = max_key_count;
}

page_id_t BPlusTreePage::GetPageId() const { return header_.page_id; }

void BPlusTreePage::SetPageId(page_id_t page_id) { header_.page_id = page_id; }

page_id_t BPlusTreePage::GetNextPageId() const { return header_.next_page_id; }

void BPlusTreePage::SetNextPageId(page_id_t next_page_id) {
  header_.next_page_id = next_page_id;
}

bool BPlusTreePage::IsLeaf() const { return header_.is_leaf; }

// ------------------------------BPlusTreeLeafPage--------------------------------
template <typename KeyType, typename ValueType, typename Comparator>
KeyType
BPlusTreeLeafPage<KeyType, ValueType, Comparator>::KeyAt(uint16_t index) const {
  return array_[index].key;
}
template <typename KeyType, typename ValueType, typename Comparator>
ValueType BPlusTreeLeafPage<KeyType, ValueType, Comparator>::ValueAt(
    uint16_t index) const {
  return array_[index].value;
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTreeLeafPage<KeyType, ValueType, Comparator>::Lookup(
    const KeyType &key, std::vector<ValueType> *value) const {
  bool flag = false;
  for (uint16_t i = 0; i < this->GetKeyCount(); ++i) {
    if (Comparator{}(array_[i].key, key) == 0) {
      value->push_back(this->ValueAt(i));
      flag = true;
    } else if (Comparator{}(array_[i].key, key) > 0) {
      break;
    }
  }
  return flag;
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTreeLeafPage<KeyType, ValueType, Comparator>::Insert(
    const KeyType &key, const ValueType &value) {
  if (this->IsFull()) {
    return false;
  }
  uint16_t key_count = this->GetKeyCount();
  uint16_t index = 0;
  while (index < key_count && Comparator{}(array_[index].key, key) < 0) {
    index++;
  }
  for (uint16_t i = key_count; i > index; --i) {
    array_[i] = array_[i - 1];
  }
  array_[index].key = key;
  array_[index].value = value;
  this->SetKeyCount(key_count + 1);
  return true;
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTreeLeafPage<KeyType, ValueType, Comparator>::Split(
    BPlusTreeLeafPage *new_page) {
  uint16_t key_count = this->GetKeyCount();
  if (key_count < 2) {
    return false; // 不够分裂
  }
  uint16_t half_count = key_count / 2;
  for (uint16_t i = 0; i < half_count; ++i) {
    new_page->array_[i] = array_[key_count - half_count + i];
  }
  this->SetKeyCount(key_count - half_count);
  new_page->SetKeyCount(half_count);
  return true;
}

// ---------------------------BPlusTreeInternalPage-------------------------------
template <typename KeyType, typename ValueType, typename Comparator>
KeyType BPlusTreeInternalPage<KeyType, ValueType, Comparator>::KeyAt(
    uint16_t index) const {
  return array_[index].key;
}

template <typename KeyType, typename ValueType, typename Comparator>
ValueType BPlusTreeInternalPage<KeyType, ValueType, Comparator>::ValueAt(
    uint16_t index) const {
  return array_[index].value;
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTreeInternalPage<KeyType, ValueType, Comparator>::Insert(
    const KeyType &key, const ValueType &value) {
  if (this->IsFull()) {
    return false;
  }
  uint16_t key_count = this->GetKeyCount();
  uint16_t index = 0;
  while (index < key_count && Comparator{}(array_[index].key, key) < 0) {
    index++;
  }
  for (uint16_t i = key_count; i > index; --i) {
    array_[i] = array_[i - 1];
  }
  array_[index].key = key;
  array_[index].value = value;
  this->SetKeyCount(key_count + 1);
  return true;
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTreeInternalPage<KeyType, ValueType, Comparator>::Split(
    BPlusTreeInternalPage *new_page) {
  uint16_t key_count = this->GetKeyCount();
  if (key_count < 2) {
    return false; // 不够分裂
  }
  uint16_t half_count = key_count / 2;
  for (uint16_t i = 0; i < half_count; ++i) {
    new_page->array_[i] = array_[key_count - half_count + i];
  }
  this->SetKeyCount(key_count - half_count);
  new_page->SetKeyCount(half_count);
  return true;
}

template class BPlusTreeLeafPage<int32_t, RID, mini::IntComparator>;
template class BPlusTreeInternalPage<int32_t, page_id_t, mini::IntComparator>;

} // namespace mini
