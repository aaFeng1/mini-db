#pragma once
#include "binder/binder.h"
#include "binder/bound_statement.h"
#include "execution/execution_context.h"
#include "storage/table_iterator.h"
#include "storage/tuple.h"
#include <memory>
#include <string>

namespace mini {

class Executor {
public:
  Executor(ExecutionContext &context) : context_(context) {}
  virtual ~Executor() = default;
  virtual void Init() = 0;
  virtual bool Next(Tuple *) = 0;

protected:
  ExecutionContext &Context() { return context_; }

private:
  ExecutionContext &context_;
};

class InsertExecutor : public Executor {
public:
  explicit InsertExecutor(
      ExecutionContext &context,
      std::unique_ptr<BoundInsertStatement> bound_insert_stmt)
      : Executor(context), bound_insert_stmt_(std::move(bound_insert_stmt)) {}

  ~InsertExecutor() override = default;

  void Init() override;
  bool Next(Tuple *) override;

private:
  std::unique_ptr<BoundInsertStatement> bound_insert_stmt_;
  bool done_{false};
};

class SelectExecutor : public Executor {
public:
  explicit SelectExecutor(ExecutionContext &context,
                          std::unique_ptr<BoundSelectStatement> bstat)
      : Executor(context), bound_select_stmt_(std::move(bstat)) {}

  ~SelectExecutor() override = default;

  void Init() override;
  bool Next(Tuple *tuple) override;

private:
  std::unique_ptr<BoundSelectStatement> bound_select_stmt_;
  TableIterator table_iter_;
  TableIterator end_;
  bool inited_;
};

class CreateTableExecutor : public Executor {
public:
  explicit CreateTableExecutor(
      ExecutionContext &context,
      std::unique_ptr<BoundCreateTableStatement> bound_create_table_stmt)
      : Executor(context),
        bound_create_table_stmt_(std::move(bound_create_table_stmt)) {}

  ~CreateTableExecutor() override = default;

  void Init() override;
  bool Next(Tuple *) override;

private:
  std::unique_ptr<BoundCreateTableStatement> bound_create_table_stmt_;
  bool done_{false};
};

} // namespace mini
