#pragma once
#include "catalog/schema.h"
#include "storage/table_heap.h"
#include <memory>
#include <string>

namespace mini {

struct TableInfo {
  std::string name;
  std::shared_ptr<Schema> schema;
  std::shared_ptr<TableHeap> table; // 或 shared_ptr，看你是否多处持有
  int32_t table_id;                 // 可选
};

class Catalog {
public:
  Catalog(BufferPool *bpm) : bpm_(bpm) {}

  TableInfo *CreateTable(const std::string &name,
                         std::shared_ptr<Schema> schema);
  TableInfo *GetTable(const std::string &name);
  void ListTables();

private:
  std::unordered_map<std::string, std::unique_ptr<TableInfo>> tables_;
  int32_t next_table_id_{0};
  BufferPool *bpm_; // 需要创建 TableHeap 时用（或你传 disk/bpm）
};

} // namespace mini
