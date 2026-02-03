#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>

#include "binder/binder.h"
#include "catalog/catalog.h"
#include "execution/executor.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "storage/table_heap.h"
#include "storage/tuple.h"
#include <sstream>
#include <string>

// src/main.cpp
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 下面这些 include 你按你项目真实路径改

// 如果你需要存储层初始化（DiskManager/BufferPool）也在这里 include
// #include "storage/disk_manager.h"
// #include "storage/buffer_pool_manager.h"

namespace mini {

// ---------------------------
// 你需要根据自己项目改动的“初始化建表”部分
// ---------------------------

// 例子：建一个表 t，有两列：int 和 char[16] (固定长字符串)
// 你项目里 Column/Schema 的构造函数、TableInfo 的字段名可能不同：按需改。
static void BootstrapCatalog(Catalog &catalog) {
  auto schema = std::make_shared<Schema>();
  schema->AddColumn("col1", DataType::INTEGER);
  schema->AddColumn("col2", DataType::VARCHAR, 10);
  catalog.CreateTable("t", schema);
}

// ---------------------------
// 输出：把 Tuple 的 bytes 打印出来
// v1 没有类型系统/解码层的话，先按“定长布局”打印也行
// ---------------------------
static void PrintSelectHeader() {
  std::cout << "+----------+------------------+\n";
  std::cout << "| col1(int)| col2(str[16])    |\n";
  std::cout << "+----------+------------------+\n";
}

static void PrintSelectFooter() {
  std::cout << "+----------+------------------+\n";
}

// 你需要根据 Tuple/Schema 的真实 API 改：如何拿到 tuple 的 raw bytes
// 假设你有：tuple.GetData() / tuple.Data() / tuple.GetDataPtr() 等
static const char *TupleDataPtr(const Tuple &t) { return t.Data(); }

static void PrintTupleAsV1Row(const Tuple &t) {
  // v1 演示：按 (int32 @ offset0) + (char[16] @ offset4) 打印
  const char *p = TupleDataPtr(t);
  if (!p) {
    std::cout << "(tuple decode not wired)\n";
    return;
  }

  int32_t a = 0;
  std::memcpy(&a, p + 0, sizeof(int32_t));

  char s[17];
  std::memset(s, 0, sizeof(s));
  std::memcpy(s, p + 4, 16);

  std::cout << std::right;
  std::cout << "| " << std::setw(8) << a << " | ";
  std::cout << std::left << std::setw(16) << s << " |\n";
}

// ---------------------------
// REPL 辅助
// ---------------------------
static void PrintPrompt() { std::cout << "mini-db> " << std::flush; }

static bool IsQuit(const std::string &line) {
  return line == "quit" || line == "exit" || line == "\\q";
}

} // namespace mini

int main() {
  using namespace mini;

  try {
    // ---------------------------
    // 1) 初始化存储层（如果你需要）
    // ---------------------------
    // v1 如果你 Catalog 里创建 TableHeap 需要 Disk/Buffer，就在这里初始化
    //
    BufferPool bpm(/*pool_size=*/10, new DiskManager("data/mini.db"));
    Catalog catalog(&bpm); // 或 Catalog(bpm)

    // ---------------------------
    // 2) 手动建表（因为 v1 不支持 CREATE）
    // ---------------------------
    BootstrapCatalog(catalog);

    std::cout << "LightDB v1 (minimal) ready. Type SQL, or 'quit'.\n";

    // ---------------------------
    // 3) REPL：读 SQL -> parse -> bind -> exec
    // ---------------------------
    std::string line;
    while (true) {
      PrintPrompt();
      if (!std::getline(std::cin, line))
        break;
      if (line.empty())
        continue;
      if (IsQuit(line))
        break;

      // 3.1 Parser
      std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(line);
      Parser parser(std::move(lexer));
      auto stmt = parser.ParseStatement();
      if (!stmt) {
        std::cerr << "[parse error] " << parser.GetError().Message()
                  << "\n"; // TODO: 按你 Parser 错误接口改
        continue;
      }

      // 3.2 Binder
      Binder binder(catalog);
      auto bound = binder.BindStatement(*stmt);
      if (!bound) {
        std::cerr << "[bind error] " << binder.GetError().Message()
                  << "\n"; // TODO: 按你 Binder 错误接口改
        continue;
      }

      // 3.3 Executor
      switch (bound->Type()) {
      case BoundStatementType::BOUND_INSERT: {
        // 你如果 executor 构造函数收
        // unique_ptr<BoundInsertStatement>，这里需要转换 推荐：Binder
        // 直接返回具体的 bound stmt（或提供 helper）
        //
        // 这里先用 dynamic_cast 做演示（v1 OK）
        auto *raw = dynamic_cast<BoundInsertStatement *>(bound.release());
        if (!raw) {
          std::cerr << "[exec error] bad bound stmt type\n";
          continue;
        }

        std::unique_ptr<BoundInsertStatement> ins(raw);

        InsertExecutor exec(std::move(ins));
        exec.Init();
        // 如果你把 INSERT 设计成 Next 执行一次，就 while；否则 Init 已插入也行
        while (exec.Next(nullptr)) {
        }

        std::cout << "OK (insert)\n";
        break;
      }

      case BoundStatementType::BOUND_SELECT: {
        auto *raw = dynamic_cast<BoundSelectStatement *>(bound.release());
        if (!raw) {
          std::cerr << "[exec error] bad bound stmt type\n";
          continue;
        }

        std::unique_ptr<BoundSelectStatement> sel(raw);

        SelectExecutor exec(std::move(sel));
        exec.Init();

        PrintSelectHeader();
        Tuple out;
        // 如果你的 SelectExecutor::Next 是 Next(Tuple*), 用下面这行：
        while (exec.Next(&out)) {
          PrintTupleAsV1Row(out);
        }
        PrintSelectFooter();
        break;
      }

      default:
        std::cerr << "[exec error] unsupported statement in v1\n";
        break;
      }
    }

    std::cout << "bye.\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "fatal: " << e.what() << "\n";
    return 1;
  }
}
