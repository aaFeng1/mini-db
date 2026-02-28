#pragma once
#include "catalog/schema.h"
#include "index/index.h"
#include "storage/table_heap.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mini {

struct TableInfo {
  std::string name;
  std::shared_ptr<Schema> schema;
  std::shared_ptr<TableHeap> table; // 或 shared_ptr，看你是否多处持有
  int32_t table_id;                 // 可选
};

struct IndexInfo {
  std::string index_name;
  std::string table_name;
  std::unique_ptr<Schema> key_schema; // 索引键的 schema
  std::unique_ptr<Index> index;       // 索引结构
  int32_t index_id;                   // 可选
};

class Catalog {
public:
  Catalog(BufferPool *bpm) : bpm_(bpm) {}

  TableInfo *CreateTable(const std::string &name,
                         std::shared_ptr<Schema> schema);
  TableInfo *GetTable(const std::string &name);
  void ListTables();

  IndexInfo *CreateIndex(const std::string &index_name,
                         const std::string &table_name, uint32_t key_col_id);
  IndexInfo *GetIndex(const std::string &table_name,
                      const std::string &col_name);

  std::vector<std::shared_ptr<IndexInfo>> &
  GetIndexes(const std::string &table_name);
  void ListIndexes();

private:
  std::vector<IndexInfo *> &GetIndexInternal(const std::string &table_name);

  std::unordered_map<std::string, std::unique_ptr<TableInfo>> tables_;
  int32_t next_table_id_{0};
  BufferPool *bpm_; // 需要创建 TableHeap 时用（或你传 disk/bpm）

  // 方便根据表名找索引
  std::unordered_map<std::string, std::vector<std::shared_ptr<IndexInfo>>>
      table_to_indexes_;
  // TODO:???
  std::unordered_map<std::string, std::shared_ptr<IndexInfo>> indexes_;
  int32_t next_index_id_{0};
};

} // namespace mini
