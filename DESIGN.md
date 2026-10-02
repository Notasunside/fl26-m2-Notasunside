# M2 Technical & Design Understanding

This file is an assessed technical-understanding artifact, not ordinary project documentation.
Answer all four questions using your own submitted implementation. Concise answers are acceptable when they are technically correct and specific.

Generic descriptions of C++ concepts or restatements of the assignment that do not identify and explain corresponding parts of your code will receive limited credit.

## 1. Polymorphism and dynamic dispatch - 1.5 points

Identify one place in your M2 implementation where runtime polymorphism occurs. Name the relevant base interface, derived implementation, and `ProcessingCore` function involved. Trace the call from `ProcessingCore` to the selected strategy implementation and explain why the derived implementation is invoked.

Then explain what would change if the relevant operation were not declared `virtual`.

//////////////////////
Q1 Answer

One place in my M2 implementation where runtime polymorphism occurs is when I use runtime polymorphism in ProcessingCore::rebuild(), which calls chunk() through an owned ChunkingStrategy pointer. The default implementation is Chunker, and my custom test supplies MarkerChunkiing. In class we learned that virtual methods are intended for polymorphic use. So, when rebuild() processes each document, the virtual call selects the chunk() implementation belonging to the actual strategy object. 
If chunk() were not virtual, the call through the base pointer would not dispatch to the derived implementation. My derived override declarations would also fail to compile, plus the base operation could not remain pure virtual using= 0.
//////////////////////

## 2. Ownership and lifetime - 1.5 points

Identify where one of the strategy objects is created, where ownership is transferred, and which object ultimately owns it. Explain how `std::unique_ptr` represents that ownership relationship and when the strategy object is destroyed.

Also explain why `ProcessingCore` is move-only and why the strategy base classes require virtual destructors.



//////////////////////
Q2 Answer
I create a strategy with std::make_uniqueMarkerChunking(counts) and transfer ownership to ProcessingCore through its constructor. The core's private Impl owns it thorugh a std::unique_ptr. This pointer symbolizes exclusive ownership and jautomatically destroys the strat when its owner is replaced/also destroyed. I made ProcessingCore move only because its unique pointers can trasnfer ownership but cannot be copied. I took inspiration from our lecture. The strategy base calsses need virtual destructors so deleting a derived strat through a base pointer runs the derived destructure as planned. 
//////////////////////


## 3. Architecture, extensibility, and M1 compatibility - 1.5 points

Explain one specific architectural decision in your M2 implementation that makes the processing system extensible while preserving M1 behavior.

Identify the classes or interfaces involved and explain both:
- how the default configuration preserves M1 behavior; and
- how a different implementation can be substituted without changing the normal `ProcessingCore` API.

Include one plausible design alternative and explain why the M2 design is preferable for this milestone. The alternative does not need to be something you actually implemented.


//////////////////////
Q3 Answer
THere are a few specific architectural changes. I decided to make ProcessingCore use the ChunkingStrategy, RetrievalStrategy, and ContextStrategy interfaces. Its default constructor supplies Chunker, RetrievalEngine, and ContextBuilder, so it preserves the M1 algorithms. I can inject different implementations through the strategy constructor while keeping the same rebuild(), search(), and build_context() API.
I suppose one alternative would be adding conditionals inside ProcessingCore to select algorithms. I prefer the M2 design because adding a strategy does not require changing the core’s processing logic.\
//////////////////////


## 4. Testing and defect reasoning - 1.5 points

Select one meaningful test from your `tests/student_tests.cpp`.

Explain:
- what M2 requirement the test validates;
- what specific implementation defect the test could detect; and
- why your test provides useful evidence beyond simply rerunning the supplied public tests.

If your test uses a custom strategy, explain how its observable behavior demonstrates that `ProcessingCore` is actually using runtime substitution.


//////////////////////
Q4 Answer
I ended up choosing test_custom_strategy_pipeline() because it validates runtime substitution and interaction between components. My custom strategies produce a "marker" chunk, a search score of 17.0, and "customcontext" text. I also check their call counters and confirm the marker chunk enters the index.
This test could detect the core ignoring an injected strategy and calling a default component instead. It adds evidence beyond the public tests by checking distinct custom behavior through the normal core API, showing that the injected implementations actually run.
//////////////////////