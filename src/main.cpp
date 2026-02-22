#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "binder/binder.h"
#include "catalog/catalog.h"
#include "execution/execution_context.h" // 如果你把 ctx 单独放一个头文件，按实际路径改
#include "execution/executor.h" // 这里假设包含 Executor/InsertExecutor/SelectExecutor
#include "parser/lexer.h"
#include "parser/parser.h"
#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "storage/tuple.h"

namespace mini {

// ---------------------------
// 如果你现在已经支持 CREATE TABLE，可以把 BootstrapCatalog 删除。
// 如果仍然想保留一个“默认表”，也可以继续用。
// ---------------------------
static void BootstrapCatalog(Catalog &catalog) {
  // 下面按你 Schema/Column API 调整
  auto schema = std::make_shared<Schema>();
  schema->AddColumn("col1", DataType::INTEGER);
  // 假设你已经把 VARCHAR 长度存进 TypeSpec/ColumnType 里了，这里演示用 10
  schema->AddColumn("col2", DataType::VARCHAR, 10);

  catalog.CreateTable("t", schema);
}

static void PrintPrompt() { std::cout << "mini-db> " << std::flush; }

static bool IsQuit(const std::string &line) {
  return line == "quit" || line == "exit" || line == "\\q";
}

// ---------------------------
// 你需要根据 Tuple 的真实 API 改：如何拿到 raw data
// ---------------------------
static const char *TupleDataPtr(const Tuple &t) { return t.Data(); }

static size_t DefaultWidth(const Column &c) {
  if (c.type == DataType::INTEGER)
    return 11; // -2147483648
  if (c.type == DataType::VARCHAR)
    return std::min<size_t>(c.length, 30);
  return 10;
}

static void PrintLine(const std::vector<size_t> &widths) {
  std::cout << '+';
  for (auto w : widths) {
    std::cout << std::string(w + 2, '-') << '+';
  }
  std::cout << "\n";
}

static std::string PadOrTrunc(std::string s, size_t w) {
  if (s.size() <= w)
    return s + std::string(w - s.size(), ' ');
  // 截断并加 ...
  if (w <= 3)
    return s.substr(0, w);
  return s.substr(0, w - 3) + "...";
}

static void PrintSelectHeader(const std::vector<Column> &cols,
                              const std::vector<size_t> &widths) {
  PrintLine(widths);
  std::cout << '|';
  for (size_t i = 0; i < cols.size(); i++) {
    std::cout << ' ' << PadOrTrunc(cols[i].name, widths[i]) << " |";
  }
  std::cout << "\n";
  PrintLine(widths);
}

template <class ValueOf>
static void PrintRows(const std::vector<Column> &cols,
                      const std::vector<size_t> &widths, SelectExecutor &exec,
                      ValueOf value_of) {
  Tuple t;
  size_t row_count = 0;

  while (exec.Next(&t)) {
    std::cout << '|';
    for (size_t i = 0; i < cols.size(); i++) {
      std::string v = value_of(t, i);
      std::cout << ' ' << PadOrTrunc(std::move(v), widths[i]) << " |";
    }
    std::cout << "\n";
    row_count++;
  }
  PrintLine(widths);
  std::cout << "(" << row_count << " rows)\n";
}

} // namespace mini

int main() {
  using namespace mini;

  try {
    auto disk = std::make_unique<DiskManager>("data/mini.db");
    BufferPool bpm(10, disk.get());

    Catalog catalog(&bpm);
    ExecutionContext ctx(catalog);

    BootstrapCatalog(catalog);

    std::cout << "LightDB ready. Type SQL, or 'quit'.\n";

    // ---------------------------
    // 3) REPL：SQL -> parse -> bind -> exec
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
      auto lexer = std::make_unique<Lexer>(line);
      Parser parser(std::move(lexer));
      auto stmt = parser.ParseStatement();
      if (!stmt) {
        std::cerr << "[parse error] " << parser.GetError().Message() << "\n";
        continue;
      }

      // 3.2 Binder
      Binder binder(catalog);
      auto bound = binder.BindStatement(*stmt);
      if (!bound) {
        std::cerr << "[bind error] " << binder.GetError().Message() << "\n";
        continue;
      }

      // 3.3 Executor
      // 你这里目前用 dynamic_cast + release 的方式 OK（v1 收尾完全够用）
      switch (bound->Type()) {

      case BoundStatementType::BOUND_INSERT: {
        auto *raw = dynamic_cast<BoundInsertStatement *>(bound.release());
        if (!raw) {
          std::cerr << "[exec error] bad bound stmt type\n";
          continue;
        }
        std::unique_ptr<BoundInsertStatement> ins(raw);

        // 改动：传 ctx
        InsertExecutor exec(ctx, std::move(ins));
        exec.Init();
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

        // 改动：传 ctx
        SelectExecutor exec(ctx, std::move(sel));
        exec.Init();

        auto schema = exec.GetSchema();
        const std::vector<Column> &cols = schema->GetColumns();
        std::vector<size_t> widths(cols.size());
        for (size_t i = 0; i < cols.size(); i++) {
          widths[i] = std::max(cols[i].name.size(), DefaultWidth(cols[i]));
        }
        PrintSelectHeader(cols, widths);
        PrintRows(cols, widths, exec,
                  [&schema](const Tuple &t, size_t col_idx) -> std::string {
                    return t.GetValue(schema, col_idx)->ToString();
                  });
        break;
      }

      case BoundStatementType::BOUND_CREATE_TABLE: {
        auto *raw = dynamic_cast<BoundCreateTableStatement *>(bound.release());
        if (!raw) {
          std::cerr << "[exec error] bad bound stmt type\n";
          continue;
        }
        std::unique_ptr<BoundCreateTableStatement> ct(raw);

        // 改动：传 ctx
        CreateTableExecutor exec(ctx, std::move(ct));
        exec.Init();
        while (exec.Next(nullptr)) {
        }
        std::cout << "OK (create table)\n";
        break;
      }

        // case BoundStatementType::BOUND_DELETE: {
        //   auto *raw = dynamic_cast<BoundDeleteStatement *>(bound.release());
        //   if (!raw) {
        //     std::cerr << "[exec error] bad bound stmt type\n";
        //     continue;
        //   }
        //   std::unique_ptr<BoundDeleteStatement> del(raw);

        //   // 改动：传 ctx
        //   DeleteExecutor exec(ctx, std::move(del));
        //   exec.Init();
        //   while (exec.Next(nullptr)) {
        //   }
        //   std::cout << "OK (delete)\n";
        //   break;
        // }

      default:
        std::cerr << "[exec error] unsupported statement\n";
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