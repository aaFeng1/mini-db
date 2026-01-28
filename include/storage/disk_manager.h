#pragma once
#include "common/page.h"
#include <string>

namespace mini {

class DiskManager {
public:
  explicit DiskManager(const std::string &file_path);
  ~DiskManager();

  void WritePage(page_id_t page_id, const Page &page);
  void ReadPage(page_id_t page_id, Page &page);

private:
  std::string file_path_;
  int fd_{-1};

  static long long OffsetOf(page_id_t page_id);
};

} // namespace mini
