#include <cstring>
#include <iostream>

#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "storage/table_heap.h"
#include <sstream>
#include <string>

using namespace mini;

void ExecuteSQL(const std::string &sql, TableHeap &table_heap) {
  std::istringstream iss(sql);
  std::string cmd;
  iss >> cmd;

  if (cmd == "insert") {
    int id, value;
    if (!(iss >> id >> value)) {
      std::cerr << "syntax error: insert <id> <value>\n";
      return;
    }

    Row row{id, value};
    RID rid = table_heap.InsertRow(row);

    std::cout << "inserted: page=" << rid.page_id << " slot=" << rid.slot_id
              << "\n";
    return;
  }

  if (cmd == "select") {
    table_heap.Scan([](const RID &rid, const Row &row) {
      std::cout << "page=" << rid.page_id << " slot=" << rid.slot_id
                << " | id=" << row.id << " value=" << row.value << "\n";
    });
    return;
  }

  std::cerr << "unknown command: " << cmd << "\n";
}

int main() {
  DiskManager dm("mini.db");
  BufferPool buffer_pool(100, &dm);
  TableHeap table(&buffer_pool);

  std::string line;
  while (true) {
    std::cout << "mini-db> ";
    if (!std::getline(std::cin, line)) {
      break;
    }
    if (line == "exit" || line == "quit") {
      break;
    }
    ExecuteSQL(line, table);
  }
  return 0;
}
