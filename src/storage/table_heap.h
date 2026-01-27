#pragma once
#include "buffer_pool.h"
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

struct RID {
  int32_t page_id;
  uint16_t slot_id;
};

struct Row {
  int32_t id;
  int32_t value;
};

struct TablePageHeader {
  int32_t next_page_id;
  uint16_t num_slots;
};

class TablePage {
public:
  static TablePage *From(char *data) {
    return reinterpret_cast<TablePage *>(data);
  }
  static const TablePage *From(const char *data) {
    return reinterpret_cast<const TablePage *>(data);
  }

  void init() {
    header_.next_page_id = -1;
    header_.num_slots = 0;
  }

  page_id_t GetNextPageId() const { return header_.next_page_id; }
  void SetNextPageId(page_id_t pid) { header_.next_page_id = pid; }

  uint32_t GetNumSlots() const { return header_.num_slots; }

  bool HasSpaceForOneRow() const {
    uint32_t n = header_.num_slots;
    size_t need = sizeof(TablePageHeader) + (n + 1) * sizeof(Row);
    return need <= PAGE_SIZE;
  }

  uint16_t InsertRow(const Row &row) {
    if (!HasSpaceForOneRow()) {
      throw std::runtime_error("page full");
    }
    uint16_t slot = header_.num_slots;
    std::memcpy(GetRowPtr(slot), &row, sizeof(row));
    header_.num_slots++;
    return slot;
  }

  Row GetRow(uint16_t slot) const {
    if (slot >= header_.num_slots) {
      throw std::runtime_error("slot out of range");
    }
    Row row;
    std::memcpy(&row, GetRowPtr(slot), sizeof(Row));
    return row;
  }

private:
  char *GetRowPtr(uint16_t slot) {
    return reinterpret_cast<char *>(this) + sizeof(TablePageHeader) +
           slot * sizeof(Row);
  }
  const char *GetRowPtr(uint16_t slot) const {
    return reinterpret_cast<const char *>(this) + sizeof(TablePageHeader) +
           slot * sizeof(Row);
  }
  TablePageHeader header_;
};

class TableHeap {
public:
  explicit TableHeap(BufferPool *buffer_pool);

  // 插入一条记录，返回它的物理位置
  RID InsertRow(const Row &row);

  // 根据 RID 精确读取
  Row GetRow(const RID &rid);

  // 顺序扫描整张表
  void Scan(std::function<void(const RID &, const Row &)> callback);

private:
  BufferPool *buffer_pool_;

  // 表的页链表
  int32_t first_page_id_;
  int32_t last_page_id_;

  // （可选）保护插入和扩容
  std::mutex latch_;
};

} // namespace mini
