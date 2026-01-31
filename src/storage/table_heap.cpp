#include "storage/table_heap.h"
#include "common/page_guard.h"
#include <cstdint>
#include <stdexcept>

namespace mini {

void TablePage::Init() {
  header_.next_page_id = -1;
  header_.num_slots = 0;
  header_.free_space_ptr = PAGE_SIZE;
}

bool TablePage::InsertTuple(const char *tuple_data, uint16_t tuple_size,
                            uint16_t *out_slot_id) {
  size_t need = sizeof(Slot) + tuple_size;
  if (GetFreeSpace() < need) {
    return false;
  }
  // insert slot
  uint16_t new_offset = header_.free_space_ptr - tuple_size;
  char *page_data = reinterpret_cast<char *>(this);
  std::memcpy(page_data + new_offset, tuple_data, tuple_size);
  header_.free_space_ptr = new_offset;

  Slot *new_slot = SlotAt(page_data, header_.num_slots);
  new_slot->offset = new_offset;
  new_slot->size = tuple_size;
  new_slot->is_deleted = 0;
  header_.num_slots += 1;

  *out_slot_id = header_.num_slots - 1;
  return true;
}

bool TablePage::GetTuple(uint16_t slot_id, const char **out_data,
                         uint16_t *out_size) const {
  if (slot_id >= header_.num_slots)
    return false;
  const char *page_data = reinterpret_cast<const char *>(this);
  const Slot *slot = SlotAt(page_data, slot_id);
  if (slot->is_deleted)
    return false;
  *out_data = page_data + slot->offset;
  *out_size = slot->size;
  return true;
}

bool TablePage::MarkDelete(uint16_t slot_id) {
  if (slot_id >= header_.num_slots)
    return false;
  char *page_data = reinterpret_cast<char *>(this);
  Slot *slot = SlotAt(page_data, slot_id);
  slot->is_deleted = 1;
  return true;
}

bool TablePage::IsDeleted(uint16_t slot_id) const {
  // 判断 slot 是否被标记为删除
  if (slot_id >= header_.num_slots)
    return true;
  const char *page_data = reinterpret_cast<const char *>(this);
  const Slot *slots_ = SlotAt(page_data, slot_id);
  if (slots_->is_deleted)
    return true;
  return false;
}

uint16_t TablePage::GetFreeSpace() const {
  return header_.free_space_ptr - sizeof(header_) -
         sizeof(Slot) * header_.num_slots;
}

uint16_t TablePage::GetSlotCount() const { return header_.num_slots; }

TableHeap::TableHeap(BufferPool *buffer_pool) : buffer_pool_(buffer_pool) {
  // TODO:bad design
  first_page_id_ = last_page_id_ = 0;
  Page *p0 = buffer_pool_->FetchPage(0);
  TablePage *tp = TablePage::From(p0->GetData());
  tp->Init();
  buffer_pool_->UnpinPage(0, true);
}

RID TableHeap::InsertTuple(const Tuple &tuple) {
  PageGuard pg = buffer_pool_->FetchPageGuarded(last_page_id_);
  TablePage *tp = pg.GetPage()->As<TablePage>();
  uint16_t out_slot_id;
  if (tp->InsertTuple(tuple.Data(), tuple.Size(), &out_slot_id)) {
    pg.SetDirty();
    return RID{last_page_id_, out_slot_id};
  }
  // need new page
  // TODO: add page allocation in BufferPool
  PageGuard pgNex = buffer_pool_->FetchPageGuarded(last_page_id_ + 1);
  // TODO: maybe is nullptr
  TablePage *tpNex = pgNex.GetPage()->As<TablePage>();
  tpNex->Init();
  tp->SetNextPageId(last_page_id_ + 1);
  last_page_id_ += 1;
  pg.SetDirty();
  pgNex.SetDirty();
  if (tpNex->InsertTuple(tuple.Data(), tuple.Size(), &out_slot_id)) {
    return RID{last_page_id_, out_slot_id};
  }
  throw std::runtime_error("InsertTuple failed even after new page allocated");
}

bool TableHeap::GetTuple(const RID &rid, Tuple *out) {
  PageGuard pg = buffer_pool_->FetchPageGuarded(rid.page_id);
  // TODO: maybe nullptr
  TablePage *tp = pg.GetPage()->As<TablePage>();
  const char *data;
  uint16_t size;
  if (!tp->GetTuple(rid.slot_id, &data, &size)) {
    return false;
  }
  out->SetData(data, size);
  return true;
}

TableIterator TableHeap::Begin() { return TableIterator(); }

} // namespace mini
