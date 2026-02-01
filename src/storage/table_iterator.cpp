#include "storage/table_iterator.h"
#include "storage/table_heap.h"

#include "common/rid.h"

namespace mini {

TableIterator::TableIterator(TableHeap *table_heap, const RID &rid, bool end)
    : table_heap_(table_heap), rid_(rid), end_(end) {}

Tuple TableIterator::operator*() const {
  Tuple tuple;
  table_heap_->GetTuple(rid_, &tuple);
  return tuple;
}

TableIterator &TableIterator::operator++() {
  if (end_) {
    return *this;
  }

  AdvanceToNextValid();
  return *this;
}

bool TableIterator::operator==(const TableIterator &other) const {
  if (end_ && other.end_) {
    return true;
  }
  return table_heap_ == other.table_heap_ &&
         rid_.page_id == other.rid_.page_id &&
         rid_.slot_id == other.rid_.slot_id && end_ == other.end_;
}

bool TableIterator::operator!=(const TableIterator &other) const {
  return !(*this == other);
}

void TableIterator::AdvanceToNextValid() {
  // 获取当前页
  BufferPool *buffer_pool = table_heap_->GetBufferPool();
  page_id_t current_page_id = rid_.page_id;
  auto page = buffer_pool->FetchPage(current_page_id);
  auto table_page = TablePage::From(page->GetData());

  uint16_t slot_count = table_page->GetSlotCount();
  uint16_t next_slot_id = rid_.slot_id + 1;

  // 查找下一个有效的槽
  while (true) {
    while (next_slot_id < slot_count) {
      if (!table_page->IsDeleted(next_slot_id)) {
        rid_.slot_id = next_slot_id;
        buffer_pool->UnpinPage(current_page_id, false);
        return;
      }
      next_slot_id++;
    }

    // 移动到下一页
    page_id_t next_page_id = table_page->GetNextPageId();
    buffer_pool->UnpinPage(current_page_id, false);
    if (next_page_id == INVALID_PAGE_ID) {
      end_ = true;
      return;
    }

    page = buffer_pool->FetchPage(next_page_id);
    table_page = TablePage::From(page->GetData());
    current_page_id = next_page_id;
    rid_.page_id = current_page_id;
    slot_count = table_page->GetSlotCount();
    next_slot_id = 0;
  }
}

}; // namespace mini