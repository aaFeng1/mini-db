#include <cassert>
#include <cstring>
#include <iostream>

#include "../src/storage/disk_manager.h"

int main() {
  mini::DiskManager dm("mini.db");

  mini::Page out;
  const char *msg = "page-0";
  std::memcpy(out.data.data(), msg, std::strlen(msg));
  dm.WritePage(0, out);

  msg = "page-100";
  std::memcpy(out.data.data(), msg, std::strlen(msg));
  dm.WritePage(100, out);

  mini::Page in;
  dm.ReadPage(0, in);
  //   std::cout << "read: " << reinterpret_cast<const char *>(in.data.data())
  //             << "\n";
  assert(strcmp(reinterpret_cast<const char *>(in.data.data()), "page-0") == 0);
  dm.ReadPage(100, in);
  //   std::cout << "read: " << reinterpret_cast<const char *>(in.data.data())
  //             << "\n";
  assert(strcmp(reinterpret_cast<const char *>(in.data.data()), "page-100") ==
         0);

  std::cout << "ALL TESTS PASSED" << std::endl;

  return 0;
}
