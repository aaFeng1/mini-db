#include "index/bplus_tree_page.h"
#include "common/comparator.h"
#include <cstdint>
#include <iostream>

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
void BPlusTreeLeafPage<KeyType, ValueType, Comparator>::SetKeyAt(
    uint16_t index, const KeyType &key) {
  array_[index].key = key;
}
template <typename KeyType, typename ValueType, typename Comparator>
void BPlusTreeLeafPage<KeyType, ValueType, Comparator>::SetValueAt(
    uint16_t index, const ValueType &value) {
  array_[index].value = value;
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTreeLeafPage<KeyType, ValueType, Comparator>::Lookup(
    const KeyType &key, std::vector<ValueType> *value) const {
  bool flag = false;
  uint16_t left = 0, right = this->GetKeyCount() - 1;
  while (left <= right) {
    uint16_t mid = left + (right - left) / 2;
    if (Comparator{}(key, array_[mid].key) < 0) {
      right = mid - 1;
    } else if (Comparator{}(key, array_[mid].key) > 0) {
      left = mid + 1;
    } else {
      // 找到了一个匹配的key，向两边遍历
      uint16_t i = mid;
      while (true) {
        if (Comparator{}(key, array_[i].key) != 0)
          break;
        value->push_back(array_[i].value);
        if (i == left)
          break; // 关键：到边界就停
        --i;
      }
      i = mid + 1;
      while (i <= right && Comparator{}(key, array_[i].key) == 0) {
        value->push_back(array_[i].value);
        flag = true;
        i++;
      }
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

  new_page->SetNextPageId(this->GetNextPageId());
  this->SetNextPageId(new_page->GetPageId());
  return true;
}

template <typename KeyType, typename ValueType, typename Comparator>
void BPlusTreeLeafPage<KeyType, ValueType, Comparator>::Print(
    std::ostream &os) const {
  os << "Leaf Page: " << this->GetPageId() << " -> " << this->GetNextPageId()
     << std::endl;
  for (uint16_t i = 0; i < this->GetKeyCount() && i < 5; ++i) {
    os << "key: " << array_[i].key << " value: " << array_[i].value.page_id
       << ":" << array_[i].value.slot_id << std::endl;
  }
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
void BPlusTreeInternalPage<KeyType, ValueType, Comparator>::SetKeyAt(
    uint16_t index, const KeyType &key) {
  array_[index].key = key;
}
template <typename KeyType, typename ValueType, typename Comparator>
void BPlusTreeInternalPage<KeyType, ValueType, Comparator>::SetValueAt(
    uint16_t index, const ValueType &value) {
  array_[index].value = value;
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
bool BPlusTreeInternalPage<KeyType, ValueType, Comparator>::InsertAfter(
    const ValueType &old_value, const KeyType &new_key,
    const ValueType &new_value) {
  uint16_t key_count = this->GetKeyCount();
  uint16_t index = 0;
  for (; index < key_count; ++index) {
    if (array_[index].value == old_value) {
      break;
    }
  }
  if (index >= key_count) {
    return false;
  }
  if (this->IsFull()) {
    return false;
  }
  for (uint16_t i = key_count; i > index + 1; --i) {
    array_[i] = array_[i - 1];
  }
  array_[index + 1].key = new_key;
  array_[index + 1].value = new_value;
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

template <typename KeyType, typename ValueType, typename Comparator>
void BPlusTreeInternalPage<KeyType, ValueType, Comparator>::Print(
    std::ostream &os) const {
  os << "Internal Page: " << this->GetPageId() << std::endl;
  for (uint16_t i = 0; i < this->GetKeyCount(); ++i) {
    os << "key: " << array_[i].key << " pid: " << array_[i].value << std::endl;
  }
}

template class BPlusTreeLeafPage<int32_t, RID, mini::IntComparator>;
template class BPlusTreeInternalPage<int32_t, page_id_t, mini::IntComparator>;

} // namespace mini
