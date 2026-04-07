#include <gtest/gtest.h>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "helixdb/storage/pager.hpp"
#include "helixdb/btree/bplushtree.hpp"
#include "helixdb/btree/btree_cursor.hpp"
#include <random>

using namespace helixdb;

class HelixDBAcidTest : public ::testing::Test {
protected:
    const std::string db_path = "acid_test.db";
    const std::string wal_path = "acid_test.db.wal";

    void SetUp() override {
        CleanFiles();
    }

    void TearDown() override {
        CleanFiles();
    }

    void CleanFiles() {
        std::filesystem::remove(db_path);
        std::filesystem::remove(wal_path);
    }
};

TEST_F(HelixDBAcidTest, TestAtomicityAndDurability) {
    {
        storage::Pager pager(db_path);
        bplushtree::BPlushTree tree(pager);

        tree.insert(10, 1000);
        tree.insert(20, 2000);
    }

    {
        storage::Pager pager(db_path);
        bplushtree::BPlushTree tree(pager);

        uint32_t value;
        EXPECT_TRUE(tree.find(10, value));
        EXPECT_EQ(value, 1000);
        EXPECT_TRUE(tree.find(20, value));
        EXPECT_EQ(value, 2000);
    }
}

TEST_F(HelixDBAcidTest, TestStructuralConsistency) {
    storage::Pager pager(db_path);
    bplushtree::BPlushTree tree(pager);

    std::vector<uint64_t> keys;
    for (uint64_t i = 1; i <= 200; ++i) {
        keys.push_back(i);
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(keys.begin(), keys.end(), g);

    for (uint64_t k : keys) {
        tree.insert(k, static_cast<uint32_t>(k * 10));
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
    storage::Pager pager(db_path);
    bplushtree::BPlushTree tree(pager);

    tree.insert(50, 500);

    auto cursor = tree.lower_bound(50);
    EXPECT_EQ(cursor.value(), 500);

    tree.insert(50, 999);

    EXPECT_EQ(cursor.value(), 999);
}

TEST_F(HelixDBAcidTest, TestSpaceManagementConsistency) {
    storage::Pager pager(db_path);
    
    uint32_t p1 = pager.allocate_page();
    uint32_t p2 = pager.allocate_page();
    
    pager.free_page(p1);
    
    uint32_t p3 = pager.allocate_page();
    EXPECT_EQ(p1, p3);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}