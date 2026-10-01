
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

struct Counts {
    int chunk_calls = 0;
    int retrieve_calls = 0;
    int context_calls = 0;
    int destroyed = 0;
};

class MarkerChunkiing final : public aiws::ChunkingStrategy {
    Counts& counts;

    public:
    explicit MarkerChunkiing(Counts& counts) : counts(counts) {}
    ~MarkerChunkiing() override { ++counts.destroyed; }
    std::vector<aiws::Chunk> chunk(const aiws::Document& doc ,std::size_t order ) const override {
        ++counts.chunk_calls;
       return { {   doc.id() + "#marker", doc.id(), order, 0, "marker", 1, 0, doc.text().size()
        }};
    }
};

class MarkerRetrieval final : public aiws::RetrievalStrategy {
    Counts& counts;

public:
    explicit MarkerRetrieval(Counts& c) : counts(c) {}
    ~MarkerRetrieval() override {  ++counts.destroyed;  }

    std::vector<aiws::SearchResult> search(const std::string&,int k, const std::vector<aiws::Chunk>& chunks, const aiws::CorpusIndex& index) 
    const override    {
        ++counts.retrieve_calls;
        if (    k <= 0 || chunks.empty()    ) return {};
            const auto& c = chunks.front();
        // was custom chunk indexed?
        if (    index.term_frequency("marker", c.id) != 1   ) return {};
        return {{ c.id, c.document_id, c.sequence, c.text, 17.0, 1  }};
    }
};

class MarkerContext final : public aiws::ContextStrategy { Counts& counts;

public:
    explicit MarkerContext(Counts& c) : counts(c) {}

    ~MarkerContext() override { ++counts.destroyed; }
    std::vector<aiws::ContextItem> build(
        const std::vector<aiws::SearchResult>& ranked,
        std::size_t budget) const override {
        ++counts.context_calls;
        if (    ranked.empty() || budget == 0   ) return {};

            const auto& r = ranked.front();

        return {{
            r.chunk_id, r.document_id, r.chunk_sequence, "customcontext", 1, r.score, false
        }};
    }
};

aiws::Workspace sample_workspace() {
    aiws::Workspace ws;
    ws.add_document(aiws::Document{ "first", "", "Alpha beta beta"  }   );
    ws.add_document(aiws::Document{ "yo mama", "", "Gamma alpha" });
    return ws;
}

///////////////////////////////

//Tests Below
///////////////////////////////

///////////////////////////////


void test_default_m1_behavior() {
    // Check that the default core still chunks, indexes, searches,
    // and builds context using the M1 behavior. //

    aiws::ProcessingCore core;
    core.rebuild( sample_workspace() );

    check(core.chunk_count() ==2, "default chunk count" );
    check(core.term_frequency("beta", "first#0") == 2,  "default term freq "); 
    check(core.document_frequency("alpha") == 2 , "default document freq" );
        const auto results = core.search("beta" , 2);
    check(results.size()== 1 &&   results[0].document_id =="first",  "default retrieval finds matching document");
        const auto context = core.build_context("beta", 2 , 2);
    // check(context.size() ==  1 &&  context[0].text== "alpha beta " && context[0].token_count == 2 && context[0].truncated, "default context respects token budget");
        check(context.size() ==  1 &&  context[0].text== "alpha beta" && context[0].token_count == 2 && context[0].truncated, "default context respects token budget");
    check(core.search( "", 2).empty() , " empty quer");
    check(core.search("beta" , 0).empty(), "0 results requested ");
    check(core.build_context("beta", 2, 0).empty(),  "zero context budget" );
}

void test_custom_strategy_pipeline() {
     // 

    Counts counts;
    //std::make_unique from <memory>
    aiws::ProcessingCore core(  std::make_unique<MarkerChunkiing>(counts), std::make_unique<MarkerRetrieval>(counts),std::make_unique<MarkerContext>(counts));
    core.rebuild(sample_workspace());

    check( counts.chunk_calls == 2 &&core.chunk_count() == 2 &&   core.chunks()[0].id == "first#marker",  "custom chunker called" );
    check(core.term_frequency("marker", "first#marker") ==1 ,  "custom chunks enter the index");
        const auto results = core.search("unrelated", 1);
    check(counts.retrieve_calls == 1 &&  results.size()== 1 &&  results[0].score == 17.0, "custom retrieval called");
    //check(counts.retrieval_calls == 1 &&  results.size()== 1 &&  results[0].score == 17.0, "custom retrieval called");
        const auto context = core.build_context("unrelated" , 1, 2);
    check(counts.retrieve_calls == 2 && counts.context_calls == 1 &&  context.size() == 1 &&  context[0].text == "customcontext" &&   context[0].score == 17.0, 
     "custom context receives custom ranking");
}

void test_null_strategies() {


    for (int missing = 0; missing < 3; ++missing) {
        Counts counts;
        bool rejected = false;

        try {

            // let status = age >= 18 ? "Adult" : "Minor";
            // // If age >= 18 is true -> "Adult", else -> "Minor"
            aiws::ProcessingCore core(
                missing == 0?  nullptr  : std::make_unique<MarkerChunkiing>(counts),
                missing == 1 ? nullptr  : std::make_unique<MarkerRetrieval>(counts),
                missing == 2? nullptr: std::make_unique<MarkerContext>(counts)
            );

            // <stdexcept> std::invalid_argument and catch
            //https://cplusplus.com/doc/tutorial/exceptions/

        } catch (const std::invalid_argument&) {  rejected = true;}
            check(rejected, "null strategy throws invalid argument" );
            check(counts.destroyed ==2, "the rejected config releases our supplied strategies" );
    }
}

void test_failed_rebuild_preserves_corpus() {
      

    aiws::ProcessingCore core;
    core.rebuild(sample_workspace());

    aiws::Workspace duplicate;
    duplicate.add_document(aiws::Document{ "same", "", "new words"  });
    duplicate.add_document(aiws::Document{  "same", "", "moar words"  });
        bool rejected = false;
    try {core.rebuild(duplicate); } catch (const std::invalid_argument&) {rejected = true; }

    check(rejected, "it rejected duplicate ids");
    check( core.chunk_count()== 2 && core.chunks()[0].id == "first#0",  "failed rebuild preserveed chunks" );
    check( core.term_frequency("beta", "first#0") ==2,  "failed rebuild preserves index" );
        const auto results = core.search(   "beta" , 1 );
    check( results.size() == 1 &&  results[0].document_id == "first",   "old corpus still was searchable" );
}

void test_move_and_lifetime() {
    // #include <type_traits>  // for std::is_copy_constructible_v, etc.
    //https://en.cppreference.com/cpp/header/type_traits
    //-- https://stackoverflow.com/questions/1647895/what-does-static-assert-do-and-what-would-you-use-it-for
    // from link: static_assert is used to make assertions at compile time. /
    //When the static assertion fails, the program simply doesn't compile.
    static_assert( !std::is_copy_constructible_v<aiws::ProcessingCore>  );
    static_assert(   !std::is_copy_assignable_v<aiws::ProcessingCore>   );
    static_assert(  std::is_nothrow_move_constructible_v<aiws::ProcessingCore>  );
    static_assert(   std::is_nothrow_move_assignable_v<aiws::ProcessingCore>    );
    Counts counts;

    {
        aiws::ProcessingCore original(   std::make_unique<MarkerChunkiing>(counts), std::make_unique<MarkerRetrieval>(counts), std::make_unique<MarkerContext>(counts)  );
        original.rebuild(sample_workspace());
        aiws::ProcessingCore moved(std::move(original));

            const auto results =moved.search("unrelated", 1);
        check( results.size() == 1 && results[0].score == 17.0 ,  "move constr keeps strategies and corpus" );
        aiws::ProcessingCore assigned;

        // <utility> std::move
            assigned = std::move( moved );
         const auto context =  assigned.build_context("unrelated", 1,2);

        check(context.size() == 1 && context[0].text == "customcontext", " the move assignment keeps pipeline ");
        check( counts.destroyed ==  0,  "moves don't destroy transferred strategies" );
    }
    // are the strategies destroyed when last core reaches out of scope?
    check(  counts.destroyed == 3, " each derived strategy destroyed exactly once"   );
}



int main() {
    // TODO: Add your own M2 tests here.

   try {
        test_default_m1_behavior();
        test_custom_strategy_pipeline();
        test_null_strategies();
        test_failed_rebuild_preserves_corpus();
        test_move_and_lifetime();
    } catch (const std::exception& error) 
    {
        std::cerr << "Unexpected exception: " << error.what() << '\n';
            return 1;
    }

    if (failures != 0) return 1;
    std::cout << "M2 student tests passed\n";
        return 0;
}
