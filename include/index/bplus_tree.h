#pragma once
#include "common/page.h"
#include "common/rid.h"
#include "storage/buffer_pool.h"
#include "storage/table_heap.h"
#include <vector>

namespace mini {

template <typename KeyType, typename ValueType> class BPlusTree {
public:
  explicit BPlusTree(BufferPool *buffer_pool) : buffer_pool_(buffer_pool) {}
  ~BPlusTree() = default;

  bool Insert(const KeyType &key, const ValueType &value);
  bool GetValue(const KeyType &key, std::vector<ValueType> *value);
  bool Remove(const KeyType &key);

private:
  BufferPool *buffer_pool_;
  page_id_t root_page_id_{INVALID_PAGE_ID};
};

} // namespace mini
