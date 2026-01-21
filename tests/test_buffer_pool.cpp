#include <cassert>
#include <cstdio> // std::remove
#include <cstring>
#include <iostream>
#include <string>

#include "../src/common/page.h"
#include "../src/storage/buffer_pool.h"
#include "../src/storage/disk_manager.h"

static void FillPage(mini::Page &p, const std::string &s) {
  std::memset(p.data.data(), 0, mini::PAGE_SIZE);
  std::memcpy(p.data.data(), s.data(),
              std::min(s.size(), (size_t)mini::PAGE_SIZE));
}

static std::string ReadPagePrefix(const mini::Page &p, size_t n = 64) {
  n = std::min(n, (size_t)mini::PAGE_SIZE);
  return std::string(reinterpret_cast<const char *>(p.data.data()),
                     reinterpret_cast<const char *>(p.data.data()) + n);
}

static void TestPinProtection() {
  const char *db = "test_bp_pin.db";
  std::remove(db);

  mini::DiskManager dm(db);
  mini::BufferPool bp(1, &dm);

  mini::Page *p1 = bp.FetchPage(1);
  assert(p1 != nullptr);

  // 不 Unpin，让它一直 pin 着
  mini::Page *p2 = bp.FetchPage(2);
  assert(p2 == nullptr && "pool_size=1 and page 1 pinned, should not evict");

  std::cout << "[PASS] TestPinProtection\n";
}

static void TestDirtyEvictWriteBack() {
  const char *db = "test_bp_dirty.db";
  std::remove(db);

  mini::DiskManager dm(db);
  mini::BufferPool bp(1, &dm);

  // 1) load page 1, modify, mark dirty
  mini::Page *p1 = bp.FetchPage(1);
  assert(p1 != nullptr);
  FillPage(*p1, "page-1-dirty-data");
  assert(bp.UnpinPage(1, true));

  // 2) fetch page 2 => must evict page 1 (dirty writeback expected)
  mini::Page *p2 = bp.FetchPage(2);
  assert(p2 != nullptr);
  assert(bp.UnpinPage(2, false));

  // 3) read page 1 directly from disk, verify content persisted
  mini::Page check;
  dm.ReadPage(1, check);
  auto prefix = ReadPagePrefix(check, 32);
  assert(prefix.find("page-1-dirty-data") != std::string::npos);

  std::cout << "[PASS] TestDirtyEvictWriteBack\n";
}

static void TestEvictMappingCorrectness() {
  const char *db = "test_bp_map.db";
  std::remove(db);

  mini::DiskManager dm(db);
  mini::BufferPool bp(1, &dm);

  // Prepare page 1 on disk
  {
    mini::Page tmp;
    FillPage(tmp, "ONE");
    dm.WritePage(1, tmp);
  }

  // Fetch page 1
  mini::Page *p1 = bp.FetchPage(1);
  assert(p1 != nullptr);
  assert(ReadPagePrefix(*p1, 8).find("ONE") != std::string::npos);
  assert(bp.UnpinPage(1, false));

  // Fetch page 2 => evict page 1
  mini::Page *p2 = bp.FetchPage(2);
  assert(p2 != nullptr);
  FillPage(*p2, "TWO");
  assert(bp.UnpinPage(2, true));

  // Fetch page 1 again => must not "hit wrong mapping"
  mini::Page *p1_again = bp.FetchPage(1);
  assert(p1_again != nullptr);

  auto s = ReadPagePrefix(*p1_again, 8);
  assert(s.find("ONE") != std::string::npos &&
         "mapping wrong: returned page 2 content for page 1?");
  assert(bp.UnpinPage(1, false));

  std::cout << "[PASS] TestEvictMappingCorrectness\n";
}

int main() {
  TestPinProtection();
  TestDirtyEvictWriteBack();
  TestEvictMappingCorrectness();
  return 0;
}
