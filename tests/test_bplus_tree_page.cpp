#include "common/comparator.h"
#include "common/page.h"
#include "common/rid.h"
#include "index/bplus_tree_page.h"
#include <cstdint>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

using namespace mini;

class BPlusTreePageTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(BPlusTreePageTest, BasicStruct) {
  Page page;
  auto *leaf_page =
      BPlusTreeLeafPage<int32_t, page_id_t, mini::IntComparator>::From(&page);
  leaf_page->Init();
  EXPECT_TRUE(leaf_page->IsLeaf());
  std::cout << "Max key count in leaf page: " << leaf_page->GetMaxKeyCount()
            << std::endl;
  auto *internal_page =
      BPlusTreeInternalPage<int32_t, RID, mini::IntComparator>::From(&page);
  internal_page->Init();
  EXPECT_FALSE(internal_page->IsLeaf());
  std::cout << "Max key count in internal page: "
            << internal_page->GetMaxKeyCount() << std::endl;
}

// ------------------------- Leaf Page Tests -------------------------

TEST_F(BPlusTreePageTest, BasicLeafPageInsert) {
  Page page;
  auto *leaf_page =
      BPlusTreeLeafPage<int32_t, RID, mini::IntComparator>::From(&page);
  leaf_page->Init();
  for (int i = 0; i < 339; ++i) {
    EXPECT_TRUE(leaf_page->Insert(i, RID{i, static_cast<uint16_t>(i)}));
  }
  EXPECT_EQ(leaf_page->GetKeyCount(), 339);
  for (int i = 0; i < 339; ++i) {
    EXPECT_EQ(leaf_page->KeyAt(i), i);
    EXPECT_EQ(leaf_page->ValueAt(i).page_id, i);
    EXPECT_EQ(leaf_page->ValueAt(i).slot_id, i);
  }
  // is full
  EXPECT_TRUE(leaf_page->IsFull());
  EXPECT_FALSE(leaf_page->Insert(0, RID{0, 0}));
}

// same key, different value
TEST_F(BPlusTreePageTest, DuplicateKeyLeafPageInsert) {
  Page page;
  auto *leaf_page =
      BPlusTreeLeafPage<int32_t, RID, mini::IntComparator>::From(&page);
  leaf_page->Init();
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(leaf_page->Insert(2, RID{i, static_cast<uint16_t>(i)}));
  }
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(leaf_page->Insert(1, RID{i, static_cast<uint16_t>(i)}));
  }
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(leaf_page->Insert(3, RID{i, static_cast<uint16_t>(i)}));
  }
  EXPECT_EQ(leaf_page->GetKeyCount(), 30);
  std::vector<RID> values;
  EXPECT_TRUE(leaf_page->Lookup(1, &values));
  EXPECT_EQ(values.size(), 10);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(values[i].page_id, 9 - i);
    EXPECT_EQ(values[i].slot_id, static_cast<uint16_t>(9 - i));
  }

  values.clear();
  EXPECT_TRUE(leaf_page->Lookup(2, &values));
  EXPECT_EQ(values.size(), 10);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(values[i].page_id, 9 - i);
    EXPECT_EQ(values[i].slot_id, static_cast<uint16_t>(9 - i));
  }

  values.clear();
  EXPECT_TRUE(leaf_page->Lookup(3, &values));
  EXPECT_EQ(values.size(), 10);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(values[i].page_id, 9 - i);
    EXPECT_EQ(values[i].slot_id, static_cast<uint16_t>(9 - i));
  }
}

// spilt leaf page
TEST_F(BPlusTreePageTest, SplitLeafPage) {
  Page page1, page2;
  auto *leaf_page1 =
      BPlusTreeLeafPage<int32_t, RID, mini::IntComparator>::From(&page1);
  auto *leaf_page2 =
      BPlusTreeLeafPage<int32_t, RID, mini::IntComparator>::From(&page2);
  leaf_page1->Init();
  leaf_page2->Init();
  for (int i = 0; i < 339; ++i) {
    EXPECT_TRUE(leaf_page1->Insert(i, RID{i, static_cast<uint16_t>(i)}));
  }
  EXPECT_TRUE(leaf_page1->Split(leaf_page2));
  EXPECT_EQ(leaf_page1->GetKeyCount(), 170);
  EXPECT_EQ(leaf_page2->GetKeyCount(), 169);
  for (int i = 0; i < 170; ++i) {
    EXPECT_EQ(leaf_page1->KeyAt(i), i);
    EXPECT_EQ(leaf_page1->ValueAt(i).page_id, i);
    EXPECT_EQ(leaf_page1->ValueAt(i).slot_id, i);
  }
  for (int i = 0; i < 169; ++i) {
    EXPECT_EQ(leaf_page2->KeyAt(i), i + 170);
    EXPECT_EQ(leaf_page2->ValueAt(i).page_id, i + 170);
    EXPECT_EQ(leaf_page2->ValueAt(i).slot_id, static_cast<uint16_t>(i + 170));
  }
}

// spilt leaf page with less keys
TEST_F(BPlusTreePageTest, SplitLeafPageWithLessKeys) {
  Page page1, page2;
  auto *leaf_page1 =
      BPlusTreeLeafPage<int32_t, RID, mini::IntComparator>::From(&page1);
  auto *leaf_page2 =
      BPlusTreeLeafPage<int32_t, RID, mini::IntComparator>::From(&page2);
  leaf_page1->Init();
  leaf_page2->Init();
  EXPECT_TRUE(leaf_page1->Insert(1, RID{1, static_cast<uint16_t>(1)}));
  EXPECT_FALSE(leaf_page1->Split(leaf_page2)); // 不够分裂
}

// ------------------------- Internal Page Tests -------------------------

TEST_F(BPlusTreePageTest, BasicInternalPageInsert) {
  Page page;
  auto *internal_page =
      BPlusTreeInternalPage<int32_t, page_id_t, mini::IntComparator>::From(
          &page);
  internal_page->Init();
  for (int i = 0; i < 509; ++i) {
    EXPECT_TRUE(internal_page->Insert(i, i + 1000));
  }
  EXPECT_EQ(internal_page->GetKeyCount(), 509);
  for (int i = 0; i < 509; ++i) {
    EXPECT_EQ(internal_page->KeyAt(i), i);
    EXPECT_EQ(internal_page->ValueAt(i), i + 1000);
  }
  // is full
  EXPECT_TRUE(internal_page->IsFull());
  EXPECT_FALSE(internal_page->Insert(0, 1000));
}

TEST_F(BPlusTreePageTest, SplitInternalPage) {
  Page page1, page2;
  auto *internal_page1 =
      BPlusTreeInternalPage<int32_t, page_id_t, mini::IntComparator>::From(
          &page1);
  auto *internal_page2 =
      BPlusTreeInternalPage<int32_t, page_id_t, mini::IntComparator>::From(
          &page2);
  internal_page1->Init();
  internal_page2->Init();
  for (int i = 0; i < 509; ++i) {
    EXPECT_TRUE(internal_page1->Insert(i, i + 1000));
  }
  EXPECT_TRUE(internal_page1->Split(internal_page2));
  EXPECT_EQ(internal_page1->GetKeyCount(), 255);
  EXPECT_EQ(internal_page2->GetKeyCount(), 254);
  for (int i = 0; i < 255; ++i) {
    EXPECT_EQ(internal_page1->KeyAt(i), i);
    EXPECT_EQ(internal_page1->ValueAt(i), i + 1000);
  }
  for (int i = 0; i < 254; ++i) {
    EXPECT_EQ(internal_page2->KeyAt(i), i + 255);
    EXPECT_EQ(internal_page2->ValueAt(i), i + 1255);
  }
}

// spilt internal page with less keys
TEST_F(BPlusTreePageTest, SplitInternalPageWithLessKeys) {
  Page page1, page2;
  auto *internal_page1 =
      BPlusTreeInternalPage<int32_t, page_id_t, mini::IntComparator>::From(
          &page1);
  auto *internal_page2 =
      BPlusTreeInternalPage<int32_t, page_id_t, mini::IntComparator>::From(
          &page2);
  internal_page1->Init();
  internal_page2->Init();
  EXPECT_TRUE(internal_page1->Insert(1, 1001));
  EXPECT_FALSE(internal_page1->Split(internal_page2)); // 不够分裂
  EXPECT_TRUE(internal_page1->Insert(2, 1002));
  EXPECT_TRUE(internal_page1->Split(internal_page2));

  EXPECT_EQ(internal_page1->GetKeyCount(), 1);
  EXPECT_EQ(internal_page2->GetKeyCount(), 1);
  EXPECT_EQ(internal_page1->KeyAt(0), 1);
  EXPECT_EQ(internal_page1->ValueAt(0), 1001);
  EXPECT_EQ(internal_page2->KeyAt(0), 2);
  EXPECT_EQ(internal_page2->ValueAt(0), 1002);
}
