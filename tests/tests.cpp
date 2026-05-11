#include <gtest/gtest.h>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "helixdb/storage/pager.hpp"
#include "helixdb/btree/bplustree.hpp"
#include "helixdb/btree/btree_cursor.hpp"
#include <random>

using namespace helixdb;

class HelixDBTest : public ::testing::Test {
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

    static void sequentialInsertHelper(bplustree::BPlusTree& tree, int total) {
        for (uint64_t i = 1; i <= total; ++i) {
            ASSERT_NO_THROW(tree.insert(i, static_cast<uint32_t>(i * 10)))
                << "Failed during sequential insert at key: " << i;
        }
    }

    static void sequentialVerifyHelper(bplustree::BPlusTree& tree, int total) {
        for (uint64_t i = 1; i <= total; ++i) {
            uint32_t value = 0;
            bool found = tree.find(i, value);
            ASSERT_TRUE(found) << "Key " << i << " lost after tree growth!";
            EXPECT_EQ(value, static_cast<uint32_t>(i * 10));
        }
    }
};

TEST_F(HelixDBTest, TestAtomicityAndDurability) {
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

TEST_F(HelixDBTest, TestStructuralConsistency) {
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

TEST_F(HelixDBTest, TestBufferPoolIsolation) {
    storage::Pager pager(dbPath());
    bplustree::BPlusTree tree(pager);

    tree.insert(50, 500);

    auto cursor = tree.lowerBound(50);
    EXPECT_EQ(cursor.value(), 500);

    tree.insert(50, 999);

    EXPECT_EQ(cursor.value(), 999);
}

TEST_F(HelixDBTest, TestSpaceManagementConsistency) {
    storage::Pager pager(dbPath());
    
    uint32_t pageId1 = pager.allocatePage();
    uint32_t pageId2 = pager.allocatePage();
    
    pager.freePage(pageId1);
    
    uint32_t pageId3 = pager.allocatePage();
    EXPECT_EQ(pageId1, pageId3);
}

TEST_F(HelixDBTest, SequentialInsertStress) {
    storage::Pager pager(dbPath());
    bplustree::BPlusTree tree(pager);

    const int TOTAL_INSERTS = 5000;
    
    sequentialInsertHelper(tree, TOTAL_INSERTS);
    sequentialVerifyHelper(tree, TOTAL_INSERTS);
}

TEST_F(HelixDBTest, RandomInsertStress) {
    storage::Pager pager(dbPath());
    bplustree::BPlusTree tree(pager);

    const int TOTAL_INSERTS = 3000;
    std::vector<uint64_t> keys;
    for (uint64_t i = 1; i <= TOTAL_INSERTS; ++i) {
        keys.push_back(i);
    }

    std::random_device randDevice;
    std::mt19937 generator(randDevice()); 
    std::shuffle(keys.begin(), keys.end(), generator);

    for (uint64_t keyVal : keys) {
        tree.insert(keyVal, static_cast<uint32_t>(keyVal + 100));
    }

    for (uint64_t keyVal : keys) {
        uint32_t value = 0;
        ASSERT_TRUE(tree.find(keyVal, value)) << "Randomly inserted key " << keyVal << " is missing!";
        EXPECT_EQ(value, static_cast<uint32_t>(keyVal + 100));
    }
}

TEST_F(HelixDBTest, ByteBoundarySplit) {
    storage::Pager pager(dbPath());
    bplustree::BPlusTree tree(pager);

    const int split_threshold = 350; 

    for (uint64_t i = 1; i <= split_threshold; ++i) {
        tree.insert(i, i);
    }

    uint32_t val = 0;
    EXPECT_TRUE(tree.find(1, val));
    EXPECT_TRUE(tree.find(split_threshold, val));
    
    auto cursor = tree.begin();
    int seen = 0;
    while(cursor.valid()) {
        seen++;
        cursor.next();
    }
    EXPECT_EQ(seen, split_threshold);
}

auto main(int argc, char **argv) -> int {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}