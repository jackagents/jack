// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/jack.h>
#include <gtest/gtest.h>

using namespace aos;

TEST(Allocators, ArenaGrow)
{
    /**************************************************************************
     * Growing an arena appends blocks as expected
     **************************************************************************/
    jack::ArenaAllocator arena = {};
    JACK_DEFER { arena.freeAllocator(false /*clearMemory*/); };

    const size_t ARENA_SIZE = 4 * 1024;

    for (int growCount = 0; growCount < 2; growCount++) {
        EXPECT_TRUE(arena.grow(ARENA_SIZE, ARENA_SIZE));

        jack::ArenaAllocator::Metrics metrics = arena.metrics();
        EXPECT_EQ(metrics.bytesAllocated,              0);
        EXPECT_EQ(metrics.bytesAllocatedHighWaterMark, 0);
        EXPECT_EQ(metrics.totalBytesAllocated,         0);
        EXPECT_EQ(metrics.allocations,                 0);
        EXPECT_EQ(metrics.allocationsHighWaterMark,    0);
        EXPECT_EQ(metrics.totalAllocations,            0);
        EXPECT_EQ(metrics.capacity,                    ARENA_SIZE * (growCount + 1));
        EXPECT_EQ(metrics.capacityHighWaterMark,       ARENA_SIZE * (growCount + 1));
        EXPECT_EQ(metrics.wastedSpace,                 0);
        EXPECT_EQ(metrics.wastedSpaceHighWaterMark,    0);
        EXPECT_EQ(metrics.blocks,                      growCount + 1);
        EXPECT_EQ(metrics.blocksHighWaterMark,         growCount + 1);
    }

    int blockCount = 0;
    for (const jack::ArenaAllocator::Block *block = arena.tail(); block; block = block->prev) {
        blockCount++;
    }

    EXPECT_EQ(blockCount, 2);
}

TEST(Allocators, ArenaZeroInitialisedTempMemory)
{
    /**************************************************************************
     * A temporary scope created on a zero initialised arena should revert it to
     * an unallocated state when the scope is ended.
     **************************************************************************/
    struct Foo { int bar; };

    jack::ArenaAllocator arena = {};
    JACK_DEFER { arena.freeAllocator(false /*clearMemory*/); };

    const size_t ARENA_SIZE = 4 * 1024;

    /// Begin the scope, expand the arena and allocate
    jack::ArenaAllocator::TempMemory tempScope = arena.beginTempMemory();
    {
        arena.grow(ARENA_SIZE, ARENA_SIZE);
        JACK_ALLOCATOR_ALLOC(&arena, Foo);

        /// Verify metrics reflect the allocation
        jack::ArenaAllocator::Metrics metrics = arena.metrics();
        EXPECT_GE(metrics.bytesAllocated,              sizeof(Foo));
        EXPECT_GE(metrics.bytesAllocatedHighWaterMark, sizeof(Foo));
        EXPECT_GE(metrics.totalBytesAllocated,         sizeof(Foo));
        EXPECT_EQ(metrics.allocations,                 1);
        EXPECT_EQ(metrics.allocationsHighWaterMark,    1);
        EXPECT_EQ(metrics.totalAllocations,            1);
        EXPECT_EQ(metrics.capacity,                    ARENA_SIZE);
        EXPECT_EQ(metrics.capacityHighWaterMark,       ARENA_SIZE);
        EXPECT_EQ(metrics.wastedSpace,                 0);
        EXPECT_EQ(metrics.wastedSpaceHighWaterMark,    0);
        EXPECT_EQ(metrics.blocks,                      1);
        EXPECT_EQ(metrics.blocksHighWaterMark,         1);

        /// Check arena memory blocks exist
        const jack::ArenaAllocator::Block *curr = arena.curr();
        const jack::ArenaAllocator::Block *tail = arena.tail();
        EXPECT_EQ(curr, tail);
        EXPECT_NE(curr, nullptr);
    }
    arena.endTempMemory(tempScope, false /*clearMemory*/);

    /// Check metrics are undone but the high water mark remains
    jack::ArenaAllocator::Metrics metrics = arena.metrics();
    EXPECT_EQ(metrics.bytesAllocated,              0);
    EXPECT_EQ(metrics.bytesAllocatedHighWaterMark, sizeof(Foo));
    EXPECT_EQ(metrics.totalBytesAllocated,         sizeof(Foo));
    EXPECT_EQ(metrics.allocations,                 0);
    EXPECT_EQ(metrics.allocationsHighWaterMark,    1);
    EXPECT_EQ(metrics.totalAllocations,            1);
    EXPECT_EQ(metrics.capacity,                    0);
    EXPECT_EQ(metrics.capacityHighWaterMark,       ARENA_SIZE);
    EXPECT_EQ(metrics.wastedSpace,                 0);
    EXPECT_EQ(metrics.wastedSpaceHighWaterMark,    0);
    EXPECT_EQ(metrics.blocks,                      0);
    EXPECT_EQ(metrics.blocksHighWaterMark,         1);

    /// Check arena memory blocks were reverted to a nullified state
    const jack::ArenaAllocator::Block *curr = arena.curr();
    const jack::ArenaAllocator::Block *tail = arena.tail();
    EXPECT_EQ(curr, nullptr);
    EXPECT_EQ(tail, nullptr);
}

TEST(Allocators, ArenaMultipleBlocksAfterTempMemory)
{
    /**************************************************************************
     * Create multiple blocks and allocate into them followed by a temporary
     * scope and additional blocks. On ending the temporary scope, the arena
     * should revert the additional blocks generated in the scope.
     **************************************************************************/
    jack::ArenaAllocator arena = {};
    JACK_DEFER { arena.freeAllocator(false /*clearMemory*/); };

    const size_t ARENA_SIZE = 4 * 1024;

    /// Do some allocation and snap a copy of the memory blocks
    const jack::ArenaAllocator::Block *prevCurr     = nullptr;
    const jack::ArenaAllocator::Block *prevCurrNext = nullptr;
    const jack::ArenaAllocator::Block *prevTail     = nullptr;
    size_t                             prevCurrUsed = 0;
    size_t                             prevCurrSize = 0;
    size_t                             prevTailSize = 0;
    {
        /// Expand and allocate
        arena.grow(ARENA_SIZE + 1, ARENA_SIZE + 1); /// Allocate 1st block
        arena.grow(ARENA_SIZE + 2, ARENA_SIZE + 2); /// Allocate 2nd block
        arena.grow(ARENA_SIZE + 3, ARENA_SIZE + 3); /// Allocate 3rd block
        EXPECT_EQ(arena.metrics().blocks, 3);

        /// Allocate into the first block
        EXPECT_NE(JACK_ALLOCATOR_ALLOC_ARRAY(&arena, char, static_cast<size_t>(ARENA_SIZE * .75f)), nullptr);

        /// Force arena into the 2nd block
        EXPECT_NE(JACK_ALLOCATOR_ALLOC_ARRAY(&arena, char, static_cast<size_t>(ARENA_SIZE * .75f)), nullptr);

        /// Arena is now allocating from the 2nd block, previous and next
        /// pointer of the current block should be defined.
        EXPECT_NE(arena.curr(), nullptr);                  /// The current block (2nd block) should exist
        EXPECT_NE(arena.curr()->prev, nullptr);            /// The previous block (1st block) should exist
        EXPECT_NE(arena.tail(), nullptr);                  /// The tail block (3rd block) should exist
        EXPECT_EQ(arena.curr()->next, arena.tail());       /// The next block should be the tail
        EXPECT_EQ(arena.curr()->next->prev, arena.curr()); /// The next block should point prev to us
        EXPECT_EQ(arena.curr()->prev->next, arena.curr()); /// The prev block should point next to us

        /// Snap a copy of the arena memory blocks
        prevCurr     = arena.curr();
        prevCurrNext = prevCurr->next;
        prevTail     = arena.tail();
        prevCurrUsed = prevCurr->used;
        prevCurrSize = prevCurr->size;
        prevTailSize = prevTail->size;
    }

    /// Begin a temporary scope
    jack::ArenaAllocator::TempMemory tempScope = arena.beginTempMemory();
    {
        /// Force arena into the 3rd block
        EXPECT_NE(JACK_ALLOCATOR_ALLOC_ARRAY(&arena, char, static_cast<size_t>(ARENA_SIZE * .75f)), nullptr);

        /// Arena is now allocating from the 3rd block, previous is defined but
        /// not next.
        EXPECT_NE(arena.curr(), nullptr);                  /// The current block (3rd block) should exist
        EXPECT_NE(arena.curr()->prev, nullptr);            /// The previous block (2nd block) should exist
        EXPECT_EQ(arena.curr(), arena.tail());             /// The current block and tail block are the same
        EXPECT_EQ(arena.curr()->next, nullptr);            /// We are the tail, so next block is not defined
        EXPECT_EQ(arena.curr()->prev->next, arena.curr()); /// The prev block should point next to us

        arena.grow(ARENA_SIZE, ARENA_SIZE); /// Allocate 4th block
        EXPECT_EQ(arena.metrics().blocks, 4);

    }
    arena.endTempMemory(tempScope, false /*clearMemory*/);

    /// Verify ending the scope reverted the new blocks and allocations
    /// generated in the scope.
    {
        /// Arena should now be allocating from the 2nd block, previous and next
        /// pointer of the current block should be defined.
        EXPECT_NE(arena.curr(), nullptr);                  /// The current block (2nd block) should exist
        EXPECT_NE(arena.curr()->prev, nullptr);            /// The previous block (1st block) should exist
        EXPECT_NE(arena.tail(), nullptr);                  /// The tail block (3rd block) should exist
        EXPECT_EQ(arena.curr()->next, arena.tail());       /// The next block should be the tail
        EXPECT_EQ(arena.curr()->next->prev, arena.curr()); /// The next block should point prev to us
        EXPECT_EQ(arena.curr()->prev->next, arena.curr()); /// The prev block should point next to us

        /// Verify the current and tail blocks are the same as what we
        /// snapshotted.
        const jack::ArenaAllocator::Block *curr = arena.curr();
        const jack::ArenaAllocator::Block *tail = arena.tail();
        EXPECT_EQ(curr, prevCurr);
        EXPECT_EQ(curr->next, prevCurrNext);
        EXPECT_EQ(curr->used, prevCurrUsed);
        EXPECT_EQ(curr->size, prevCurrSize);
        EXPECT_EQ(tail, prevTail);
        EXPECT_EQ(tail->used, 0);
        EXPECT_EQ(tail->size, prevTailSize);
    }

    /// Check some of the metrics
    jack::ArenaAllocator::Metrics metrics = arena.metrics();
    EXPECT_EQ(metrics.bytesAllocated,              (ARENA_SIZE * .75f) * 2);
    EXPECT_GT(metrics.bytesAllocatedHighWaterMark, (ARENA_SIZE * .75f) * 2);
    EXPECT_GT(metrics.totalBytesAllocated,         (ARENA_SIZE * .75f) * 2);
    EXPECT_EQ(metrics.allocations,                 2);
    EXPECT_EQ(metrics.allocationsHighWaterMark,    3);
    EXPECT_EQ(metrics.totalAllocations,            3);
    EXPECT_EQ(metrics.capacity,                    (ARENA_SIZE * 3) + 1 + 2 + 3);
    EXPECT_GT(metrics.capacityHighWaterMark,       metrics.capacity);
    EXPECT_GT(metrics.wastedSpace,                 0) << "We wasted space when we forced the arena into the 2nd block";
    EXPECT_GT(metrics.wastedSpaceHighWaterMark,    0);
    EXPECT_EQ(metrics.blocks,                      3);
    EXPECT_EQ(metrics.blocksHighWaterMark,         4);
}

TEST(Allocators, ArenaAutomaticGrow)
{
    /**************************************************************************
     * A zero initialised arena should automatically grow on allocation
     **************************************************************************/
    jack::ArenaAllocator arena = {};
    JACK_DEFER { arena.freeAllocator(false /*clearMemory*/); };

    /// Arena should automatically grow a block
    EXPECT_NE(JACK_ALLOCATOR_ALLOC_ARRAY(&arena, char, 4 * 1024), nullptr);

    EXPECT_NE(arena.curr(), nullptr);         /// The current block should exist
    EXPECT_EQ(arena.curr()->prev, nullptr); /// The previous block should not exist
    EXPECT_NE(arena.tail(), nullptr);         /// The tail block should exist
    EXPECT_EQ(arena.curr(), arena.tail());    /// The current and tail should be the same
}
