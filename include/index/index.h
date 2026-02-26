#pragma once
#include "catalog/schema.h"
#include "common/comparator.h"
#include "index/bplus_tree.h"
#include "storage/buffer_pool.h"
#include "storage/tuple.h"
#include <cassert>
#include <memory>

namespace mini {

class Index {
public:
  virtual ~Index() = default;
  virtual void InsertEntry(const Tuple &tuple, const RID &rid) = 0;
  virtual void DeleteEntry(const Tuple &tuple,
                           const RID &rid) = 0; // 可先不实现
  virtual bool ScanKey(const Value &key, std::vector<RID> *result) = 0;

private:
};

class BPlusTreeIndex : public Index {
public:
  BPlusTreeIndex(BufferPool *bpm, std::string index_name,
                 std::string table_name, std::shared_ptr<Schema> table_schema,
                 uint32_t key_col_id)
      : bp_(bpm), index_name_(std::move(index_name)),
        table_name_(std::move(table_name)), key_col_id_(key_col_id),
        table_schema_(table_schema), tree_(bp_) {}

  ~BPlusTreeIndex() override = default;

  void InsertEntry(const Tuple &tuple, const RID &rid) override {
    // 从 tuple 里抽 key（按 schema 解码）
    auto key_val = tuple.GetValue(table_schema_, key_col_id_);
    // 只支持 INT
    int32_t k = static_cast<IntValue *>(key_val.get())->GetValue();
    tree_.Insert(k, rid);
  }

  void DeleteEntry(const Tuple &, const RID &) override {}

  bool ScanKey(const Value &key, std::vector<RID> *result) override {
    assert(key.Type() == DataType::INTEGER);
    auto &k = static_cast<const IntValue &>(key);
    return tree_.GetValue(k.GetValue(), result);
  }

private:
  BufferPool *bp_;
  std::string index_name_;
  std::string table_name_;
  uint32_t key_col_id_;
  std::shared_ptr<Schema> table_schema_;
  BPlusTree<int32_t, RID, IntComparator> tree_;
};

} // namespace mini
