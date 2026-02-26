#include "catalog/catalog.h"
#include "common/comparator.h"
#include "common/rid.h"
#include "index/bplus_tree.h"
#include "index/index.h"
#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "storage/table_heap.h"
#include <cstdint>
#include <iostream>
#include <memory>

namespace mini {

TableInfo *Catalog::CreateTable(const std::string &name,
                                std::shared_ptr<Schema> schema) {
  std::unique_ptr<TableInfo> table_info = std::make_unique<TableInfo>();
  table_info->name = name;
  table_info->schema = schema;
  table_info->table_id = next_table_id_++;
  table_info->table = std::make_shared<TableHeap>(bpm_);
  tables_[name] = std::move(table_info);
  return tables_[name].get();
}

TableInfo *Catalog::GetTable(const std::string &name) {
  auto it = tables_.find(name);
  if (it != tables_.end()) {
    return it->second.get();
  }
  return nullptr;
}

void Catalog::ListTables() {
  for (const auto &pair : tables_) {
    const TableInfo *table_info = pair.second.get();
    std::cout << "Table ID: " << table_info->table_id
              << ", Name: " << table_info->name << std::endl;
  }
}

IndexInfo *Catalog::CreateIndex(const std::string &index_name,
                                const std::string &table_name,
                                uint32_t key_col_id) {
  auto table_it = tables_.find(table_name);
  if (table_it == tables_.end()) {
    std::cerr << "Table " << table_name << " does not exist." << std::endl;
    return nullptr;
  }
  auto &table_info = table_it->second;

  // 目前只支持 INTEGER 类型的索引键
  if (table_info->schema->GetColumn(key_col_id).type != DataType::INTEGER) {
    std::cerr << "Only INTEGER columns are supported for indexing."
              << std::endl;
    return nullptr;
  }

  std::shared_ptr<IndexInfo> index_info = std::make_unique<IndexInfo>();

  // -- index_info 初始化 --
  index_info->index_name = index_name;
  index_info->table_name = table_name;
  index_info->key_schema = std::make_unique<Schema>();
  index_info->key_schema->AddColumn(
      table_info->schema->GetColumn(key_col_id).name,
      table_info->schema->GetColumn(key_col_id).type);

  index_info->index = std::make_unique<BPlusTreeIndex>(
      bpm_, index_name, table_name, table_info->schema, key_col_id);
  index_info->index_id = next_index_id_++;

  // 维护索引映射关系
  table_to_indexes_[table_name].push_back(index_info);
  indexes_[index_name] = index_info;
  return indexes_[index_name].get();
}

IndexInfo *Catalog::GetIndex(const std::string &table_name,
                             const std::string &col_name) {
  auto it = table_to_indexes_.find(table_name);
  if (it == table_to_indexes_.end()) {
    return nullptr;
  }
  for (const auto &index_info : it->second) {
    if (index_info->key_schema->GetColumn(0).name == col_name) {
      return index_info.get();
    }
  }
  return nullptr;
}

void Catalog::ListIndexes() {
  for (const auto &pair : indexes_) {
    const IndexInfo *index_info = pair.second.get();
    std::cout << "Index ID: " << index_info->index_id
              << ", Name: " << index_info->index_name
              << ", Table: " << index_info->table_name << std::endl;
  }
}

} // namespace mini
