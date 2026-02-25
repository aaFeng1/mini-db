#include "index/bplus_tree.h"
#include "common/comparator.h"
#include "common/page.h"
#include "index/bplus_tree_page.h"
#include <iostream>
#include <stdexcept>

namespace mini {

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTree<KeyType, ValueType, Comparator>::Insert(const KeyType &key,
                                                       const ValueType &value) {
  if (root_page_id_ == INVALID_PAGE_ID) {
    //树为空，创建一个新的页作为根节点
    auto newpage = buffer_pool_->NewPageGuarded(&root_page_id_);
    auto page = newpage.GetPage();
    auto leaf = BPlusTreeLeafPage<KeyType, RID, Comparator>::From(page);
    leaf->Init(root_page_id_);
  }
  page_id_t new_page_id;
  KeyType new_key;
  if (InsertDown(root_page_id_, key, value, &new_page_id, &new_key)) {
    //根节点分裂了，创建一个新的根节点
    page_id_t father_page_id;
    auto father_pageguard = buffer_pool_->NewPageGuarded(&father_page_id);
    auto father_page =
        BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(
            father_pageguard.GetPage());
    father_page->Init(father_page_id);
    MergeNewPages(root_page_id_, new_page_id, father_page_id);

    root_page_id_ = father_page_id;
  }
  return true;
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTree<KeyType, ValueType, Comparator>::GetValue(
    const KeyType &key, std::vector<ValueType> *value) {
  if (root_page_id_ == INVALID_PAGE_ID) {
    return false;
  }
  page_id_t nowpid = root_page_id_;
  auto page = buffer_pool_->FetchPage(nowpid);
  auto dummy = BPlusTreePage::From(page);
  while (!dummy->IsLeaf()) {
    auto internal =
        BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(page);
    page_id_t child_page_id = internal->ValueAt(internal->GetKeyCount() - 1);
    for (uint16_t i = 1; i < internal->GetKeyCount(); ++i) {
      if (Comparator{}(key, internal->KeyAt(i)) <= 0) {
        child_page_id = internal->ValueAt(i - 1);
        break;
      }
    }
    buffer_pool_->UnpinPage(nowpid, false);
    nowpid = child_page_id;
    page = buffer_pool_->FetchPage(nowpid);
    dummy = BPlusTreePage::From(page);
  }

  auto leaf = BPlusTreeLeafPage<KeyType, RID, Comparator>::From(page);
  bool foundbigger = false;
  while (true) {
    for (uint16_t i = 0; i < leaf->GetKeyCount(); ++i) {
      if (Comparator{}(key, leaf->KeyAt(i)) == 0) {
        value->push_back(leaf->ValueAt(i));
      } else if (Comparator{}(key, leaf->KeyAt(i)) < 0) {
        foundbigger = true;
        break;
      }
    }
    // 现在的情况是要么因为遇到更大的key而退出循环
    if (foundbigger) {
      break;
    }
    // 要么是当前页的key遍历完了
    if (leaf->GetNextPageId() == INVALID_PAGE_ID) {
      // 如果没法继续加载新页
      break;
    }
    // 否则就加载新页
    page_id_t next_page_id = leaf->GetNextPageId();
    buffer_pool_->UnpinPage(nowpid, false);
    nowpid = next_page_id;
    page = buffer_pool_->FetchPage(nowpid);
    leaf = BPlusTreeLeafPage<KeyType, RID, Comparator>::From(page);
  }
  buffer_pool_->UnpinPage(nowpid, false);
  return !value->empty();
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTree<KeyType, ValueType, Comparator>::Remove(const KeyType &key) {
  return false;
}

template <typename KeyType, typename ValueType, typename Comparator>
void BPlusTree<KeyType, ValueType, Comparator>::Print(std::ostream &os) const {
  os << "-----B+tree struct-----" << std::endl;
  if (root_page_id_ == INVALID_PAGE_ID) {
    os << "Empty tree" << std::endl;
    os << "----------" << std::endl;
    return;
  }
  std::vector<page_id_t> current_level{root_page_id_};
  int level = 0;
  while (!current_level.empty()) {
    std::vector<page_id_t> next_level;
    os << "----Level " << level++ << ":----" << std::endl;
    for (auto page_id : current_level) {
      auto page = buffer_pool_->FetchPage(page_id);
      auto dummy = BPlusTreePage::From(page);
      if (dummy->IsLeaf()) {
        auto leaf = BPlusTreeLeafPage<KeyType, RID, Comparator>::From(page);
        leaf->Print(os);
      } else {
        auto internal =
            BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(page);
        internal->Print(os);
        for (uint16_t i = 0; i < internal->GetKeyCount(); ++i) {
          next_level.push_back(internal->ValueAt(i));
        }
      }
      buffer_pool_->UnpinPage(page_id, false);
    }
    current_level = std::move(next_level);
  }
  os << "----------" << std::endl;
}

template <typename KeyType, typename ValueType, typename Comparator>
bool BPlusTree<KeyType, ValueType, Comparator>::InsertDown(
    page_id_t page_id, const KeyType &key, const ValueType &value,
    page_id_t *new_page_id, KeyType *new_key) {
  //辅助递归函数，功能为将kv插入到以p为根的树中
  //若出现与p同级的分裂，也就是p在此次插入后满了分裂
  //修改pidret为新页页号，返回true，
  //否则返回false
  auto pageguard = buffer_pool_->FetchPageGuarded(page_id);
  auto dummy = BPlusTreePage::From(pageguard.GetPage());
  // leaf page
  if (dummy->IsLeaf()) {
    auto leaf =
        BPlusTreeLeafPage<KeyType, RID, Comparator>::From(pageguard.GetPage());
    if (!leaf->Insert(key, value)) {
      throw std::runtime_error("leaf is not full but insert failed");
    }
    pageguard.SetDirty();

    if (leaf->IsFull()) {
      auto newpage = buffer_pool_->NewPageGuarded(new_page_id);
      auto new_leaf =
          BPlusTreeLeafPage<KeyType, RID, Comparator>::From(newpage.GetPage());
      new_leaf->Init(*new_page_id);
      leaf->Split(new_leaf);
      *new_key = new_leaf->KeyAt(0);
      return true;
    }
    return false;
  }

  // internal page
  auto internal = BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(
      pageguard.GetPage());
  //找到要插入的子树
  page_id_t child_page_id = internal->ValueAt(internal->GetKeyCount() - 1);
  for (uint16_t i = 1; i < internal->GetKeyCount(); ++i) {
    if (Comparator{}(key, internal->KeyAt(i)) < 0) {
      child_page_id = internal->ValueAt(i - 1);
      break;
    }
  }
  page_id_t new_child_page_id;
  KeyType new_child_key;
  if (InsertDown(child_page_id, key, value, &new_child_page_id,
                 &new_child_key)) {
    // 子树分裂了，插入新的key和page_id
    // NOTE: 不是insert
    if (!internal->InsertAfter(child_page_id, new_child_key,
                               new_child_page_id)) {
      throw std::runtime_error("internal is not full but insert failed");
    }
    pageguard.SetDirty();
  }

  if (internal->IsFull()) {
    auto newpage = buffer_pool_->NewPageGuarded(new_page_id);
    auto new_internal =
        BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(
            newpage.GetPage());
    new_internal->Init(*new_page_id);
    internal->Split(new_internal);
    *new_key = new_internal->KeyAt(0);
    return true;
  }
  return false;
}

template <typename KeyType, typename ValueType, typename Comparator>
void BPlusTree<KeyType, ValueType, Comparator>::MergeNewPages(
    page_id_t left_pid, page_id_t right_pid, page_id_t parent_pid) {
  auto left_pageguard = buffer_pool_->FetchPageGuarded(left_pid);
  auto right_pageguard = buffer_pool_->FetchPageGuarded(right_pid);
  auto parent_pageguard = buffer_pool_->FetchPageGuarded(parent_pid);

  auto dummy = BPlusTreePage::From(left_pageguard.GetPage());
  if (dummy->IsLeaf()) {
    auto left_leaf = BPlusTreeLeafPage<KeyType, RID, Comparator>::From(
        left_pageguard.GetPage());
    auto right_leaf = BPlusTreeLeafPage<KeyType, RID, Comparator>::From(
        right_pageguard.GetPage());
    auto parent = BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(
        parent_pageguard.GetPage());

    parent->Insert(left_leaf->KeyAt(0), left_pid);
    parent->Insert(right_leaf->KeyAt(0), right_pid);
    // right_leaf->SetNextPageId(left_leaf->GetNextPageId());
    // left_leaf->SetNextPageId(right_pid);
    // left_pageguard.SetDirty();
    // right_pageguard.SetDirty();
  } else {
    auto left_leaf =
        BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(
            left_pageguard.GetPage());
    auto right_leaf =
        BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(
            right_pageguard.GetPage());
    auto parent = BPlusTreeInternalPage<KeyType, page_id_t, Comparator>::From(
        parent_pageguard.GetPage());

    parent->Insert(left_leaf->KeyAt(0), left_pid);
    parent->Insert(right_leaf->KeyAt(0), right_pid);
  }
  parent_pageguard.SetDirty();
}
template class BPlusTree<int32_t, RID, mini::IntComparator>;

} // namespace mini
