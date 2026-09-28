
// Student-written M2 tests
//
// Add your own tests to this file. Your tests are part of the submitted work
// and are evaluated under Student-Written Testing & Validation.
//
// Do not modify tests/public_tests.cpp.
//
// Your tests should exercise important M2 behavior beyond the supplied public
// tests. Consider default compatibility, custom strategies, runtime dispatch,
// invalid configuration, ownership/move behavior, and component interactions.

#include "aiws/processing_core.hpp"
#include <iostream>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <utility>
#include <type_traits>


namespace {
    int failures = 0;
    void check(bool condition, const char* label) { if (!condition) {
        std::cerr << "Womp womp fail: " << label << '\n'; ++failures; }
    }}

void test_default_m1_behavior() {
    // Check that the default core still chunks, indexes, searches,
    // and builds context using the M1 behavior. //
}

void test_custom_strategy_pipeline() {
    // Inject custom chunking, retrieval, and context strategies.
    // Check that their distinctive results appear through ProcessingCore //
}

void test_null_strategies() {
    // Pass nullptr in each constructor position and check that
    // each configuration throws std::invalid_argument //
}

void test_failed_rebuild_preserves_corpus() {
    // Build a valid corpus, then rebuild with duplicate document IDs.
    // Check that the exception leaves the original corpus usable //
}

void test_move_and_lifetime() {
    // Move a configured core and check that its strategies still work.
    // Check that a derived strategy is destroyed exactly once. //
}



int main() {
    // TODO: Add your own M2 tests here.

    test_default_m1_behavior();
    test_custom_strategy_pipeline();
    test_null_strategies();
    test_failed_rebuild_preserves_corpus();
    test_move_and_lifetime();
    
    return 0;
}
