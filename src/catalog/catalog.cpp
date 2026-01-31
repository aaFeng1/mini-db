#include "catalog/catalog.h"
#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "storage/table_heap.h"
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

} // namespace mini
