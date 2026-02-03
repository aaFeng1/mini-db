#pragma once
#include "binder/binder.h"
#include "binder/bound_statement.h"
#include "storage/table_iterator.h"
#include "storage/tuple.h"
#include <memory>
#include <string>

namespace mini {

class Executor {
public:
  Executor() = default;
  virtual ~Executor() = default;
  virtual void Init() = 0;
  virtual bool Next(Tuple *) = 0;

private:
};

class InsertExecutor : public Executor {
public:
  explicit InsertExecutor(
      std::unique_ptr<BoundInsertStatement> bound_insert_stmt)
      : bound_insert_stmt_(std::move(bound_insert_stmt)) {}

  ~InsertExecutor() override = default;

  void Init() override;
  bool Next(Tuple *) override;

private:
  std::unique_ptr<BoundInsertStatement> bound_insert_stmt_;
  bool done_{false};
};

class SelectExecutor : public Executor {
public:
  explicit SelectExecutor(std::unique_ptr<BoundSelectStatement> bstat)
      : bound_select_stmt_(std::move(bstat)) {}

  ~SelectExecutor() override = default;

  void Init() override;
  bool Next(Tuple *tuple) override;

private:
  std::unique_ptr<BoundSelectStatement> bound_select_stmt_;
  TableIterator table_iter_;
  TableIterator end_;
  bool inited_;
};

} // namespace mini
