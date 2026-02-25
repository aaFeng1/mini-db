#pragma once
#include "common/page.h"
#include "common/rid.h"
#include "storage/buffer_pool.h"
#include "storage/table_heap.h"
#include <vector>

namespace mini {

template <typename KeyType, typename ValueType, typename Comparator>
class BPlusTree {
public:
  explicit BPlusTree(BufferPool *buffer_pool) : buffer_pool_(buffer_pool) {}
  ~BPlusTree() = default;

  bool Insert(const KeyType &key, const ValueType &value);
  bool GetValue(const KeyType &key, std::vector<ValueType> *value);
  bool Remove(const KeyType &key);

  void Print(std::ostream &os) const; // for debug

private:
  bool InsertDown(page_id_t page_id, const KeyType &key, const ValueType &value,
                  page_id_t *new_page_id, KeyType *new_key);

  void MergeNewPages(page_id_t left_page_id, page_id_t right_page_id,
                     page_id_t parent_page_id);

  BufferPool *buffer_pool_;
  page_id_t root_page_id_{INVALID_PAGE_ID};
};

} // namespace mini
