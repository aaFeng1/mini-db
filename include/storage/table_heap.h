#pragma once
#include "common/rid.h"
#include "storage/buffer_pool.h"
#include "storage/tuple.h"
#include <cstdint>
#include <cstring>
#include <functional>
#include <mutex>
#include <stdexcept>

// v0 assumptions:
// - single table
// - page ids are contiguous starting from 0
// - no page reuse
// - no crash recovery
// - single-threaded

namespace mini {

struct TablePageHeader {
  int32_t next_page_id;
  uint16_t num_slots;      // 0 means no rows
  uint16_t free_space_ptr; // PAGE_SIZE - free_space_ptr = free space left
};

struct Slot {
  uint16_t offset;     // record bytes 的起始偏移（相对页首）
  uint16_t size;       // record 长度
  uint8_t is_deleted;  // v1 先做逻辑删除
  uint8_t reserved[3]; // 对齐
};

class TablePage {
public:
  static TablePage *From(char *data) {
    return reinterpret_cast<TablePage *>(data);
  }
  static const TablePage *From(const char *data) {
    return reinterpret_cast<const TablePage *>(data);
  }
  static Slot *SlotAt(char *data, uint16_t slot_id) {
    return reinterpret_cast<Slot *>(data + sizeof(TablePageHeader) +
                                    slot_id * sizeof(Slot));
  }
  static const Slot *SlotAt(const char *data, uint16_t slot_id) {
    return reinterpret_cast<const Slot *>(data + sizeof(TablePageHeader) +
                                          slot_id * sizeof(Slot));
  }

  void Init(); // 初始化 header

  bool InsertTuple(const char *tuple_data, uint16_t tuple_size,
                   uint16_t *out_slot_id);

  bool GetTuple(uint16_t slot_id, const char **out_data,
                uint16_t *out_size) const;

  bool MarkDelete(uint16_t slot_id); // 逻辑删除
  bool IsDeleted(uint16_t slot_id) const;

  uint16_t GetFreeSpace() const;
  uint16_t GetSlotCount() const;

  page_id_t GetNextPageId() const { return header_.next_page_id; }
  void SetNextPageId(page_id_t pid) { header_.next_page_id = pid; }

  uint32_t GetNumSlots() const { return header_.num_slots; }

private:
  TablePageHeader header_;
};

class TableIterator {
public:
private:
  // TODO
};

class TableHeap {
public:
  explicit TableHeap(BufferPool *buffer_pool);

  RID InsertTuple(const Tuple &tuple);
  bool GetTuple(const RID &rid, Tuple *out);
  TableIterator Begin();

private:
  BufferPool *buffer_pool_;

  // 表的页链表
  int32_t first_page_id_;
  int32_t last_page_id_;

  // （可选）保护插入和扩容
  std::mutex latch_;
};

} // namespace mini
