#include "helixdb/storage/pager.hpp"
#include "helixdb/schema/schema_manager.hpp"

#include <iostream>

using namespace helixdb;

static int passed = 0;
static int failed = 0;

#define SECTION(name) std::cout << "\n=== " << name << " ===\n"
#define EXPECT(cond, msg) \
do { \
    if (cond) { std::cout << "  [PASS] " << msg << "\n"; ++passed; } \
    else      { std::cout << "  [FAIL] " << msg << "\n"; ++failed; } \
} while(0)

void test_leaf_overflow() {
    SECTION("LeafNode layout overflow");
    std::remove("overflow_test.db");
    std::remove("overflow_test.db.wal");

    storage::Pager pager("overflow_test.db");
    schema::SchemaManager manager(pager);

    schema::TableSchema s {"t", { {"id", schema::ColumnType::INT} } };
    manager.create_table(s);
    auto& t = manager.get_table("t");

    std::cout << "Inserting 20 rows." << std::endl;
    for (int i = 0; i <= 20; ++i) t.insert(i, { std::to_string(i) });

    bool ok = true;
    for (int i = 1; i <= 20; ++i) {
        if (!t.select(i)) {
            ok = false;
            break;
        }
    }

    EXPECT(ok, "All 20 rows found after inserting.");
}

void test_split() {
    SECTION("Internal node split and parent pointer update.");
    std::remove("split_test.db");
    std::remove("split_test.db.wal");

    storage::Pager pager("split_test.db");
    schema::SchemaManager manager(pager);

    schema::TableSchema s {"t", { {"id", schema::ColumnType::INT} } };
    manager.create_table(s);
    auto& t = manager.get_table("t");

    const int N = 200;
    std::cout << "  Inserting " << N << " rows to force splits..." << std::endl;
    for (int i = 1; i <= N; ++i) {
        t.insert(i, { std::to_string(i) });
    }

    int found = 0;
    for (int i = 1; i <= N; ++i) {
        if (t.select(i)) ++found;
    }
    EXPECT(found == N, std::to_string(found) + "/" + std::to_string(N) + " rows found after splits");
}

void test_multi_table_isolation() {
    SECTION("Multi table root page collision.");
    std::remove("multi_test.db");
    std::remove("multi_test.db.wal");

    storage::Pager pager("multi_test.db");
    schema::SchemaManager mgr(pager);

    schema::TableSchema users  { "users",  { {"id", schema::ColumnType::INT} } };
    schema::TableSchema orders { "orders", { {"id", schema::ColumnType::INT} } };

    mgr.create_table(users);
    auto& u = mgr.get_table("users");

    u.insert(1, {"1"});
    u.insert(2, {"2"});

    mgr.create_table(orders);
    auto& o = mgr.get_table("orders");

    o.insert(100, {"100"});

    EXPECT(u.select(1), "users row 1 still accessible after orders table created");
    EXPECT(u.select(2), "users row 2 still accessible after orders table created");

    EXPECT(o.select(100), "orders row 100 accessible");

    EXPECT(!o.select(1), "orders tree does not contain users key 1");
}

void test_wal_recovery() {
    SECTION("WAL recovery.");
    std::remove("wal_test.db");
    std::remove("wal_test.db.wal");

    {
        storage::Pager pager("wal_test.db");
        schema::SchemaManager mgr(pager);

        schema::TableSchema s { "t", { {"id", schema::ColumnType::INT} } };
        mgr.create_table(s);
        auto& t = mgr.get_table("t");

        t.insert(10, {"10"});
        t.insert(20, {"20"});
        t.insert(30, {"30"});
    }

    {
        storage::Pager pager2("wal_test.db");
        schema::SchemaManager mgr2(pager2);

        schema::TableSchema s { "t", { {"id", schema::ColumnType::INT} } };
        mgr2.create_table(s);
        auto& t2 = mgr2.get_table("t");

        EXPECT(t2.select(10), "row 10 survives reopen");
        EXPECT(t2.select(20), "row 20 survives reopen");
        EXPECT(t2.select(30), "row 30 survives reopen");
    }
}

int main() {
    // test_leaf_overflow();
    // test_split();
    // test_multi_table_isolation();
    test_wal_recovery();

    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;

    for (const char* f : {
        "overflow_test.db", "overflow_test.db.wal",
        "split_test.db", "split_test.db.wal"
    }) std::remove(f);

    return failed > 0 ? 1 : 0;
}