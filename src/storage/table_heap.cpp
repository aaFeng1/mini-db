#include "storage/table_heap.h"
#include <cstdint>
#include <stdexcept>

namespace mini {

TableHeap::TableHeap(BufferPool *buffer_pool) : buffer_pool_(buffer_pool) {
  // TODO:bad design
  first_page_id_ = last_page_id_ = 0;
  Page *p0 = buffer_pool_->FetchPage(0);
  TablePage *tp = TablePage::From(p0->GetData());
  tp->init();
  buffer_pool_->UnpinPage(0, true);
}

RID TableHeap::InsertRow(const Row &row) {
  Page *page = buffer_pool_->FetchPage(last_page_id_);
  TablePage *tp = TablePage::From(page->GetData());

  if (tp->HasSpaceForOneRow()) {
    uint16_t slot = tp->InsertRow(row);
    buffer_pool_->UnpinPage(last_page_id_, true);
    return RID{last_page_id_, slot};
  }

  int32_t newpageId = last_page_id_ + 1;
  Page *newpage = buffer_pool_->FetchPage(newpageId);
  TablePage *newtp = TablePage::From(newpage->GetData());

  newtp->init();
  tp->SetNextPageId(newpageId);
  uint16_t slot = newtp->InsertRow(row);

  buffer_pool_->UnpinPage(last_page_id_, true);
  buffer_pool_->UnpinPage(newpageId, true);

  last_page_id_ = newpageId;

  return RID{newpageId, slot};
}

Row TableHeap::GetRow(const RID &rid) {
  if (rid.page_id < first_page_id_ || rid.page_id > last_page_id_) {
    throw std::runtime_error("out of page range");
  }

  Page *page = buffer_pool_->FetchPage(rid.page_id);
  TablePage *tp = TablePage::From(page->GetData());

  auto num_slots = tp->GetNumSlots();
  if (rid.slot_id >= num_slots) {
    throw std::runtime_error("out of slot range");
  }

  Row row = tp->GetRow(rid.slot_id);

  buffer_pool_->UnpinPage(rid.page_id, false);

  return row;
}

void TableHeap::Scan(std::function<void(const RID &, const Row &)> callback) {
  int32_t pid = first_page_id_;

  while (pid != -1) { // -1 表示 INVALID_PAGE_ID（按你 v0 约定）
    Page *page = buffer_pool_->FetchPage(pid);
    TablePage *tp = TablePage::From(page->GetData());

    uint32_t num_slots = tp->GetNumSlots();
    for (uint32_t slot = 0; slot < num_slots; slot++) {
      Row row = tp->GetRow(slot);
      RID rid{pid, static_cast<uint16_t>(slot)};

      // ★ 关键：调用 callback
      callback(rid, row);
    }

    int32_t next = tp->GetNextPageId();
    buffer_pool_->UnpinPage(pid, false);
    pid = next;
  }
}

} // namespace mini
