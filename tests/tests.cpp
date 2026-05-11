#include <gtest/gtest.h>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "helixdb/storage/pager.hpp"
#include "helixdb/btree/bplustree.hpp"
#include "helixdb/btree/btree_cursor.hpp"
#include <random>

using namespace helixdb;

class HelixDBAcidTest : public ::testing::Test {
private:
    const std::string db_path_ = "acid_test.db";
    const std::string wal_path_ = "acid_test.db.wal";

protected:
    [[nodiscard]] auto dbPath() const -> const std::string& { return db_path_; }

    void SetUp() override {
        cleanFiles();
    }
    
    void TearDown() override {
        cleanFiles();
    }

    void cleanFiles() {
        std::filesystem::remove(db_path_);
        std::filesystem::remove(wal_path_);
    }
};

TEST_F(HelixDBAcidTest, TestAtomicityAndDurability) {
    {
        storage::Pager pager(dbPath());
        bplustree::BPlusTree tree(pager);

        tree.insert(10, 1000);
        tree.insert(20, 2000);
    }

    {
        storage::Pager pager(dbPath());
        bplustree::BPlusTree tree(pager);

        uint32_t value = 0;
        EXPECT_TRUE(tree.find(10, value));
        EXPECT_EQ(value, 1000);
        EXPECT_TRUE(tree.find(20, value));
        EXPECT_EQ(value, 2000);
    }
}

TEST_F(HelixDBAcidTest, TestStructuralConsistency) {
    storage::Pager pager(dbPath());
    bplustree::BPlusTree tree(pager);

    std::vector<uint64_t> keys;
    for (uint64_t i = 1; i <= 200; ++i) {
        keys.push_back(i);
    }
    
    std::random_device randDevice;
    std::mt19937 generator(randDevice());
    std::shuffle(keys.begin(), keys.end(), generator);

    for (uint64_t keyVal : keys) {
        tree.insert(keyVal, static_cast<uint32_t>(keyVal * 10));
    }

    auto cursor = tree.begin();
    uint64_t expected_key = 1;
    int count = 0;

    while (cursor.valid()) {
        EXPECT_EQ(cursor.key(), expected_key);
        EXPECT_EQ(cursor.value(), expected_key * 10);
        
        cursor.next();
        expected_key++;
        count++;
    }

    EXPECT_EQ(count, 200);
}

TEST_F(HelixDBAcidTest, TestBufferPoolIsolation) {
    storage::Pager pager(dbPath());
    bplustree::BPlusTree tree(pager);

    tree.insert(50, 500);

    auto cursor = tree.lowerBound(50);
    EXPECT_EQ(cursor.value(), 500);

    tree.insert(50, 999);

    EXPECT_EQ(cursor.value(), 999);
}

TEST_F(HelixDBAcidTest, TestSpaceManagementConsistency) {
    storage::Pager pager(dbPath());
    
    uint32_t pageId1 = pager.allocatePage();
    uint32_t pageId2 = pager.allocatePage();
    
    pager.freePage(pageId1);
    
    uint32_t pageId3 = pager.allocatePage();
    EXPECT_EQ(pageId1, pageId3);
}

auto main(int argc, char **argv) -> int {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}