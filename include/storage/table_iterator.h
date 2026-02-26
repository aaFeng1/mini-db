#pragma once
#include "common/rid.h"
#include "storage/buffer_pool.h"
#include "storage/table_heap.h"
#include "storage/tuple.h"

namespace mini {

class TableIterator {
public:
  TableIterator() = default;
  TableIterator(TableHeap *table_heap, const RID &rid, bool end = true);

  // TODO: copy?
  Tuple operator*() const;
  TableIterator &operator++();
  bool operator==(const TableIterator &other) const;
  bool operator!=(const TableIterator &other) const;

  RID GetRID() const { return rid_; }

private:
  void AdvanceToNextValid();

  TableHeap *table_heap_{nullptr};
  RID rid_{};
  bool end_{true};
};

} // namespace mini
