#pragma once
#include "catalog/catalog.h"

namespace mini {

class ExecutionContext {
public:
  ExecutionContext(Catalog &catalog) : catalog_(catalog) {}
  ~ExecutionContext() = default;

  Catalog &GetCatalog() { return catalog_; }

private:
  Catalog &catalog_;
};

} // namespace mini
