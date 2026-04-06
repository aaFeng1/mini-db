#pragma once

#include "common/page.h"
#include <cstdint>
constexpr int MAX_COLUMN_NUM = 5;
constexpr int MAX_TABLE_NUM = 5;

namespace mini {

struct ColumnMeta {
  char name[32];
  uint32_t type;
  uint32_t len;
};

struct TableMeta {
  uint32_t table_id;
  char table_name[32];
  page_id_t first_page_id;
  page_id_t last_page_id;
  uint32_t column_count;
  ColumnMeta columns[MAX_COLUMN_NUM];
};

struct DBMetaPage {
  page_id_t next_page_id;
  uint32_t table_count;
  TableMeta tables[MAX_TABLE_NUM];
};

} // namespace mini
