#include <chrono>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "binder/binder.h"
#include "catalog/catalog.h"
#include "execution/execution_context.h"
#include "execution/executor.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "storage/tuple.h"

namespace mini {

// 默认表
static void BootstrapCatalog(Catalog &catalog) {
  // 下面按你 Schema/Column API 调整
  auto schema = std::make_shared<Schema>();
  schema->AddColumn("col1", DataType::INTEGER);

  schema->AddColumn("col2", DataType::INTEGER);

  catalog.CreateTable("t", schema);

  std::vector<int> keys(100000);
  for (int j = 0; j < 10; ++j) {

    for (int i = 0; i < 10000; ++i) {
      keys[j * 10000 + i] = i;
    }
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(keys.begin(), keys.end(), gen);

  auto table_info = catalog.GetTable("t");
  for (int key : keys) {
    Tuple tuple;
    char *buf = tuple.Resize(schema->GetTupleLength());
    int32_t tmp = key;
    memcpy(buf, &tmp, sizeof(int32_t));
    memcpy(buf + sizeof(int32_t), &tmp, sizeof(int32_t));
    table_info->table->InsertTuple(tuple);
  }
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
    if (row_count <= 10) {
      std::cout << '|';
      for (size_t i = 0; i < cols.size(); i++) {
        std::string v = value_of(t, i);
        std::cout << ' ' << PadOrTrunc(std::move(v), widths[i]) << " |";
      }
      std::cout << "\n";
    } else if (row_count == 11) {
      std::cout << "| " << PadOrTrunc("...", widths[0]) << " |";
      for (size_t i = 1; i < cols.size(); i++) {
        std::cout << ' ' << PadOrTrunc("...", widths[i]) << " |";
      }
      std::cout << "\n";
    }
    row_count++;
  }
  PrintLine(widths);
  std::cout << "(" << row_count << " rows)\n";
}

} // namespace mini

int main() {
  using namespace mini;

  try {
    std::filesystem::remove("data/mini.db");
    auto disk = std::make_unique<DiskManager>("data/mini.db");

    BufferPool bpm(1000, disk.get());

    Catalog catalog(&bpm);
    ExecutionContext ctx(catalog);

    BootstrapCatalog(catalog);

    std::cout << "MiniDB ready. Type SQL, or 'quit'.\n";

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

        auto schema = exec.GetSchema();
        const std::vector<Column> &cols = schema->GetColumns();
        std::vector<size_t> widths(cols.size());
        for (size_t i = 0; i < cols.size(); i++) {
          widths[i] = std::max(cols[i].name.size(), DefaultWidth(cols[i]));
        }
        auto start = std::chrono::steady_clock::now();
        exec.Init();
        PrintSelectHeader(cols, widths);
        PrintRows(cols, widths, exec,
                  [&schema](const Tuple &t, size_t col_idx) -> std::string {
                    return t.GetValue(schema, col_idx)->ToString();
                  });
        auto end = std::chrono::steady_clock::now();
        auto duration = end - start;
        double ms = std::chrono::duration<double, std::milli>(duration).count();
        std::cout << "(time: " << ms << " ms)\n";
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

      case BoundStatementType::BOUND_CREATE_INDEX: {
        auto *raw = dynamic_cast<BoundCreateIndexStatement *>(bound.release());
        if (!raw) {
          std::cerr << "[exec error] bad bound stmt type\n";
          continue;
        }
        std::unique_ptr<BoundCreateIndexStatement> ci(raw);

        // 改动：传 ctx
        CreateIndexExecutor exec(ctx, std::move(ci));
        exec.Init();
        while (exec.Next(nullptr)) {
        }
        std::cout << "OK (create index)\n";
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