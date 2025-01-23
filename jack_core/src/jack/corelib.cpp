/// JACK
#include <jack/corelib.h>
#include <jack/utils.h>
#include <jack/engine.h>

/// Third Party
#include <algorithm> /// std::max

#if defined(_WIN32)
    #pragma push_macro("ERROR")
    #undef ERROR
    #if !defined(NOMINMAX)
        #define NOMINMAX
    #endif
    #define WIN32_MEAN_AND_LEAN
    #include <Windows.h>  /// VirtualAlloc
    #pragma pop_macro("ERROR")
#else
    #include <sys/mman.h> /// mmap
#endif

namespace aos::jack
{
/******************************************************************************
 * Logging
 ******************************************************************************/
#if defined(JACK_SLOW_DEBUG_CHECK_BUILD_FLAG)
const bool JACK_SLOW_DEBUG_CHECKS = true;
#else
const bool JACK_SLOW_DEBUG_CHECKS = false;
#endif

void *gJACKInstance;

/// @todo: @Hardcoded array that is not synced to the enum
static bool gLogLevel[] =
{
    /*Debug*/    false,
    /*Info*/     true,
    /*Warning*/  true,
    /*Error*/    true,
    /*Critical*/ true,
    /*Fatal*/    true,
    /*Custom*/   true,
};

void setLog(log::Severity level, bool enabled)
{
    if (JACK_CHECK(level < log::Severity::Count)) {
        gLogLevel[static_cast<uint8_t>(level)] = enabled;
    }
}

bool isLogged(log::Severity level)
{
    if (JACK_CHECK(level < log::Severity::Count)) {
        return gLogLevel[static_cast<uint8_t>(level)];
    }
    return false;
}

void log(log::Severity    level,
         std::string_view log,
         std::string_view channel,
         int              line,
         std::string_view file)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder          builder = StringBuilder(scratch.arena);

    /// \note Add date-time
    auto now     = time(nullptr);
    auto gmtTime = gmtime(&now);
    char date[24];
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", gmtTime);
    builder.append(FMT_STRING("{} "), date + 2);

    /// \note Add channel string
    switch (level) {
        case aos::log::Severity::Debug:    builder.appendRef("\033[48;5;247m\033[38;5;0mDBUG\033[0;37m"); break;
        case aos::log::Severity::Info:     builder.appendRef("\033[48;5;45m\033[38;5;0mINFO\033[0;37m"); break;
        case aos::log::Severity::Warning:  builder.appendRef("\033[48;5;214m\033[38;5;0mWARN\033[0;37m"); break;
        case aos::log::Severity::Error:    builder.appendRef("\033[48;5;1m\033[38;5;0mERRR\033[0;37m"); break;
        case aos::log::Severity::Critical: builder.appendRef("\033[48;5;9m\033[38;5;0mCRIT\033[0;37m"); break;
        case aos::log::Severity::Fatal:    builder.appendRef("\033[48;5;13m\033[38;5;0mFATL\033[0;37m"); break;
        case aos::log::Severity::Custom: {
            builder.append(FMT_STRING("\033[48;5;42m\033[38;5;0m{}\033[0;37m"), channel);
        } break;
        case aos::log::Severity::Count: break;
    }

    /// \note Add channel padding
    const size_t  DEFAULT_CHANNEL_SIZE = 4; /// DBUG/ERRR/FATL .. e.t.c
    static size_t longestChannelSize   = DEFAULT_CHANNEL_SIZE;
    size_t        channelPadding       = 0; /// How much spaces to pad the channel with
    {
        longestChannelSize = std::max(longestChannelSize, channel.size());
        if (channel.size()) {
            channelPadding = longestChannelSize - channel.size();
        } else {
            channelPadding = longestChannelSize - DEFAULT_CHANNEL_SIZE;
        }
    }
    builder.append(FMT_STRING("{:>{}}"), " ", channelPadding);

    /// \note Create a string slice pointing to the file portion of the file
    /// path, e.g. "main.cpp" of /foo/bar/main.cpp
    std::string_view logLocation          = {};
    static size_t    longestLogPrefixSize = 0;
    size_t           logPadding           = 0; /// How much spaces to pad before emitting log contents
    {
        for (size_t fileIndex = file.size() - 1; fileIndex < file.size(); fileIndex--) {
            if (file[fileIndex] == '/' || file[fileIndex] == '\\') {
                logLocation = std::string_view(file.data() + (fileIndex + 1), file.size() - (fileIndex + 1));
                break;
            }
        }

        if (logLocation.empty()) {
            logLocation = file;
        }

        longestLogPrefixSize = std::max(longestLogPrefixSize, logLocation.size());
        logPadding           = longestLogPrefixSize - logLocation.size() + 1;
    }
    builder.append(FMT_STRING("{}:{:04}{:>{}}"), logLocation, line, " ", logPadding);

    /// \todo Not thread safe, pull engine label from the instance
    std::string_view engineLabel = {};
    if (gJACKInstance) {
        const Engine *engine = static_cast<Engine *>(gJACKInstance);
        engineLabel          = engine->name();
    }
    builder.append(FMT_STRING("{}{}{}{}"), engineLabel.size() ? " [" : "", engineLabel, engineLabel.size() ? "] " : "", log);

    std::string_view finalLine = builder.toStringArena(scratch.arena);
    fmt::println(stderr, FMT_STRING("{}"), finalLine);
}

/******************************************************************************
 * Allocator
 ******************************************************************************/
HeapAllocator globalHeapAllocator;

/******************************************************************************
 * Allocation Metadata
 ******************************************************************************/
static std::string dumpAllocation(JACK_CALL_SITE_ARGS size_t size, const void* ptr)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("[ptr=0x{:p}, size={}"), ptr, size);

    #ifdef JACK_ALLOCATION_TRACKING
    builder.append(FMT_STRING(", func={}, file={}, line={}"), func, file, line);
    #endif /// JACK_ALLOCATION_TRACKING

    builder.appendRef("]");
    std::string result = builder.toString();
    return result;
}

static std::string dumpAllocationRecord(const AllocationRecord& record, const void* ptr)
{
    std::string result = dumpAllocation(
            #ifdef JACK_ALLOCATION_TRACKING
            record.file,
            record.func,
            record.line,
            #endif /// JACK_ALLOCATION_TRACKING
            record.size,
            ptr);
    return result;
}

void AllocationTracker::add(JACK_CALL_SITE_ARGS void* ptr, size_t size)
{
    if (!ptr) {
        return;
    }

    AllocationEntry* entry = find(ptr);
    if (entry) {
        if (!entry->freed) {
            JACK_FATAL(
                "Error the record is marked as still allocated but the same pointer was regiven as "
                "new, there's a missing JACK_DELETE in the codebase {}", dumpAllocation(JACK_CALL_SITE_INPUT size, ptr));

            JACK_FATAL("Previously allocated at {}", dumpAllocationRecord(entry->record, ptr));
        }
    } else {
        entry = &m_allocations[ptr];
    }

    entry->record.size = size;
    #ifdef JACK_ALLOCATION_TRACKING
    entry->record.file = file;
    entry->record.func = func;
    entry->record.line = line;
    #endif /// JACK_ALLOCATION_TRACKING
    entry->freedRecord = {};
    entry->freed       = false;
}

std::string AllocationTracker::stats(uint64_t uptimeInMs) const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    auto                   builder = StringBuilder(scratch.arena);

    builder.appendRef("\n====== Memory Trace Table: Active Allocations ======\n");

    size_t bytesLeaked = 0;
    size_t ptrsLeaked  = 0;
    for (auto it : m_allocations) {
        const void *           ptr   = it.first;
        const AllocationEntry &entry = it.second;
        if (!entry.freed) {
            bytesLeaked += entry.record.size;
            ptrsLeaked++;
            builder.append("{}\n", dumpAllocationRecord(entry.record, ptr));
        }
    }

    char byteString[512];
    readableByteSizeString(byteString, sizeof(byteString), bytesLeaked);

    size_t          totalAllocations = m_allocations.size();
    FixedString<64> duration         = readableDurationStringFromMs(uptimeInMs);

    builder.append(FMT_STRING("\n{} active allocations ({}). {} allocations in the program lifetime ({}"), ptrsLeaked, byteString, totalAllocations, duration.view());
    if (uptimeInMs > 0) {
        double uptimeInS            = uptimeInMs / 1000.0;
        double allocationsPerSecond = static_cast<double>(totalAllocations) / uptimeInS;
        builder.append(FMT_STRING(", {} allocs/s"), allocationsPerSecond);
    }

    builder.appendRef(")\n");
    std::string result = builder.toString();
    return result;
}

AllocationEntry* AllocationTracker::find(const void* ptr)
{
    AllocationEntry* result = nullptr;
    if (!ptr) {
        return nullptr;
    }

    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        result = &(it->second);
    }

    return result;
}

bool AllocationTracker::markPointerFree(JACK_CALL_SITE_ARGS const void* ptr)
{
    if (!ptr) {
        return false;
    }

    AllocationEntry *entry = find(ptr);
    bool result = markEntryFree(JACK_CALL_SITE_INPUT entry, ptr);
    return result;
}

bool AllocationTracker::markEntryFree(JACK_CALL_SITE_ARGS AllocationEntry* entry, const void* ptr)
{
    bool result = false;
    if (!entry) {
        JACK_FATAL("Pointer was freed that was not allocated by the allocator {}", dumpAllocation(JACK_CALL_SITE_INPUT 0 /*size*/, ptr));
        return result;
    }

    if (!ptr) {
        return result;
    }

    if (entry->freed) {
        JACK_FATAL("Double free detected at {}", dumpAllocation(JACK_CALL_SITE_INPUT entry->record.size, ptr));
        JACK_FATAL("Previously allocated at {}", dumpAllocationRecord(entry->record, ptr));
        JACK_FATAL("Previously freed at {}", dumpAllocationRecord(entry->freedRecord, ptr));
    } else {
        result                  = true;
        entry->freed            = true;
        entry->freedRecord.size = entry->record.size;
        #ifdef JACK_ALLOCATION_TRACKING
        entry->freedRecord.file = file;
        entry->freedRecord.func = func;
        entry->freedRecord.line = line;
        #endif /// JACK_ALLOCATION_TRACKING
    }

    return result;
}

/******************************************************************************
 * HeapAllocator
 ******************************************************************************/
static void updateAllocatorMetrics(AllocatorMetrics& metrics, size_t size)
{
    metrics.allocations++;
    metrics.totalAllocations++;

    metrics.bytesAllocated += size;
    metrics.totalBytesAllocated += size;

    metrics.bytesAllocatedHighWaterMark = std::max(metrics.bytesAllocatedHighWaterMark, metrics.bytesAllocated);
    metrics.allocationsHighWaterMark    = std::max(metrics.allocationsHighWaterMark, metrics.allocations);
}

void HeapAllocator::addAllocationMetadata(JACK_CALL_SITE_ARGS void* ptr, size_t allocationSize)
{
    if (!ptr) {
        return;
    }

    updateAllocatorMetrics(m_metrics, allocationSize);

    /// Forward the call site information from the original caller
    m_allocationTracker.add(JACK_CALL_SITE_INPUT ptr, allocationSize);
}

void HeapAllocator::deleteInternal(JACK_CALL_SITE_ARGS const void* ptr)
{
    TracyFree(ptr);
    AllocationEntry *entry = m_allocationTracker.find(ptr);
    if (!entry) { return; }

    m_allocationTracker.markEntryFree(JACK_CALL_SITE_INPUT entry, ptr);
    if (entry->record.size > m_metrics.bytesAllocated) {
        JACK_FATAL("A pointer is being freed that exceeds the recorded number of active bytes in the allocator [freeSize={}, activeSize={}]{}", entry->record.size, m_metrics.bytesAllocated, m_allocationTracker.stats(0));
        JACK_ASSERT(m_metrics.bytesAllocated >= entry->record.size);
    } else {
        JACK_ASSERT_MSG(m_metrics.allocations > 0, "Internal error: The allocation count has gone out of sync and we're trying to delete an allocation that did not come from this allocator.");
        m_metrics.allocations--;
        m_metrics.bytesAllocated -= entry->record.size;

        if (m_metrics.allocations == 0) {
            if (m_metrics.bytesAllocated != 0) {
                std::string stats = m_allocationTracker.stats(0);
                JACK_FATAL("Internal error: dumping allocation table {}", stats);
            }
            JACK_ASSERT_MSG(m_metrics.bytesAllocated == 0, "Internal error: 0 allocations means no bytes should be allocated");
        }
    }
}

ArenaAllocator::TempMemoryScope::TempMemoryScope(ArenaAllocator& arena)
: m_arena(arena)
, m_tempMemory(arena.beginTempMemory())
{
}

ArenaAllocator::TempMemoryScope::~TempMemoryScope()
{
    m_arena.endTempMemory(m_tempMemory, /*clearMemory*/ false);
}

void *ArenaAllocator::allocate(JACK_CALL_SITE_ARGS size_t size, uint8_t alignment)
{
    ZoneScoped;
    #ifdef JACK_ALLOCATION_TRACKING
    /// \todo Use the call site arguments? For arenas I'm not sure this make
    /// a whole lot of sense.
    (void)file; (void)func; (void)line;
    #endif /// JACK_ALLOCATION_TRACKING

    JACK_ASSERT_MSG(alignment > 0,                        "Alignment can not be zero");
    JACK_ASSERT_MSG(((alignment & (alignment - 1)) == 0), "Require power of 2 alignment");
    size_t maxSize = size + (alignment - 1);

    /// Get the next free memory block to allocate from
    while (!m_curr || (m_curr->used + maxSize) > m_curr->size) {
        if (m_curr && m_curr->next) {
            /// Advance to the next memory block that was prior pre-allocated
            m_metrics.wastedSpace += (m_curr->size - m_curr->used);
            m_metrics.wastedSpaceHighWaterMark = std::max(m_metrics.wastedSpaceHighWaterMark, m_metrics.wastedSpace);
            m_curr = m_curr->next;
        } else {
            /// No more memory blocks left in the arena, grow and try again
            size_t allocationSize = std::max(maxSize, m_minBlockSize);
            if (!grow(allocationSize, allocationSize)) { return nullptr; }
        }
    }

    /// Calculate the offset required to correctly align the allocation
    const uintptr_t targetAddress = reinterpret_cast<uintptr_t>(m_curr->memory) + m_curr->used;
    const size_t alignmentOffset  = (alignment - (targetAddress & (alignment - 1))) & (alignment - 1);
    const size_t requiredSize     = size + alignmentOffset;

    /// Allocate memory from the arena
    void *unalignedResult = m_curr->memory + m_curr->used;
    void *result          = nullptr;

    /// Ensure the OS backs the allocation with some physical pages
    JACK_ASSERT_MSG(m_curr->committed >= m_curr->used, "Internal error: Can't use more memory than is committed by the OS");
    bool committed = true;
    {
        size_t commitSpace = m_curr->committed - m_curr->used;
        if (commitSpace < requiredSize) {
            size_t commitRequirement = requiredSize - commitSpace;
            committed                = virtualCommit(unalignedResult, commitRequirement);
            m_curr->committed    += commitRequirement;
            m_metrics.commitSize += commitRequirement;
        }
    }

    /// Divvy out the pointer
    if (committed) {
        JACK_ASSERT_MSG(m_curr->used < m_curr->committed, "Internal error: We just commited memory, or ensured that we have enough commit space ");
        result        = static_cast<char *>(unalignedResult) + alignmentOffset;
        m_curr->used += requiredSize;
        JACK_ASSERT_MSG(m_curr->used      <= m_curr->committed, "Internal error: Can't use more memory than is committed by the OS");
        JACK_ASSERT_MSG(m_curr->committed <= m_curr->size, "Internal error: Committed memory is out of bounds");
        JACK_ASSERT_MSG(requiredSize <= maxSize, "Internal error: Alignment calculation is botched");
        JACK_ASSERT_MSG((reinterpret_cast<uintptr_t>(result) & (alignment - 1)) == 0, "Internal error: Pointer alignment to power of two failed");

        /// Update allocator/arena metrics
        updateAllocatorMetrics(m_metrics, requiredSize);
    }

    return result;
}

static size_t allocationSizeForArenaBlock(const ArenaAllocator::Block* block)
{
    size_t result = sizeof(*block) + block->size;
    return result;
}

bool ArenaAllocator::grow(size_t size, size_t commitSize)
{
    if (!JACK_CHECK(commitSize <= size)) {
        commitSize = size;
    }

    /// Allocate ourselves a memory block
    size_t allocationSize = sizeof(Block) + size;
    void * buffer         = virtualReserve(allocationSize, false /*commit*/);
    m_metrics.sysCalls++;

    /// Check for allocation failure
    if (!buffer) {
        return false;
    }

    /// Ensure the OS can allocate us the physical pages for the block we will
    /// initialise inline in the buffer we just reserved.
    if (!virtualCommit(buffer, sizeof(Block) + commitSize)) {
        return false;
    }
    m_metrics.sysCalls++;

    /// Setup the memory block
    auto *block       = new (buffer) Block;
    block->memory     = static_cast<char *>(buffer) + sizeof(*block);
    block->size       = allocationSize - sizeof(*block);
    block->prev       = m_tail;
    block->committed += commitSize;

    if (m_tail) {
        m_tail->next = block;
    }

    m_tail = block;

    if (!m_curr) {
        m_curr = m_tail;
    }

    /// Update metrics
    m_metrics.capacity += block->size;
    m_metrics.blocks++;

    m_metrics.blocksHighWaterMark   = std::max(m_metrics.blocksHighWaterMark, m_metrics.blocks);
    m_metrics.capacityHighWaterMark = std::max(m_metrics.capacityHighWaterMark, m_metrics.capacity);
    m_metrics.commitSize           += commitSize;
    return true;
}

static void freeArenaBlocksUntil(ArenaAllocator* arena, ArenaAllocator::Block** arenaBlocks, const ArenaAllocator::Block* until, bool clearMemory)
{
    while ((*arenaBlocks) != until) {
        if (clearMemory) {
            memset((*arenaBlocks)->memory, 0xAA, (*arenaBlocks)->used);
        }
        ArenaAllocator::Block *prev = (*arenaBlocks)->prev;
        virtualRelease((*arenaBlocks), allocationSizeForArenaBlock(*arenaBlocks));
        arena->m_metrics.sysCalls++;
        (*arenaBlocks) = prev;
    }
}

void ArenaAllocator::freeAllocator(bool clearMemory)
{
    freeArenaBlocksUntil(this, &m_tail, nullptr, clearMemory);

    m_curr                   = nullptr;
    m_tail                   = nullptr;
    m_metrics.bytesAllocated = 0;
    m_metrics.allocations    = 0;
    m_metrics.capacity       = 0;
    m_metrics.wastedSpace    = 0;
    m_metrics.blocks         = 0;
}

void ArenaAllocator::reset(bool clearMemory)
{
    m_metrics.wastedSpace    = 0;
    m_metrics.allocations    = 0;
    m_metrics.bytesAllocated = 0;
    for (Block *block = m_tail; block; block = block->prev) {
        if (clearMemory) {
            memset(block->memory, 0xAA, block->used);
        }
        block->used = 0;
        if (!block->prev) {
            m_curr = block;
        }
    }
}

bool ArenaAllocator::owns(const void* ptr) const
{
    bool result = false;
    for (const Block *block = tail(); block && !result; block = block->prev) {
        const void *begin = block->memory;
        const void *end   = static_cast<const char *>(begin) + block->size;
        result            = (ptr >= begin && ptr <= end);
    }
    return result;
}

ArenaAllocator::TempMemory ArenaAllocator::beginTempMemory()
{
    /// Take a snapshot of the arena
    TempMemory result     = {};
    result.tail           = m_tail;
    result.curr           = m_curr;
    result.bytesAllocated = m_metrics.bytesAllocated;
    result.allocations    = m_metrics.allocations;
    result.capacity       = m_metrics.capacity;
    result.wastedSpace    = m_metrics.wastedSpace;
    result.blocks         = m_metrics.blocks;
    if (result.curr) {
        result.currNext = result.curr->next;
        result.currUsed = result.curr->used;
    }

    return result;
}

void ArenaAllocator::endTempMemory(TempMemory temp, bool clearMemory)
{
    /// Free the tail blocks until the scope's tail block
    freeArenaBlocksUntil(this, &m_tail, temp.tail, clearMemory);

    /// Zero all the blocks between the tail and the temp's current blok
    for (Block *block = m_tail; block != temp.curr; block = block->prev) {
        block->used = 0;
    }

    /// Revert the curr/tail block to the temp
    if (m_tail) {
        m_tail->used = 0;
        m_tail->next = nullptr;
    }

    m_curr = temp.curr;
    if (m_curr) {
        JACK_ASSERT(m_tail);
        m_curr->used = temp.currUsed;
        m_curr->next = temp.currNext;
    }

    /// Restore metrics
    m_metrics.bytesAllocated = temp.bytesAllocated;
    m_metrics.allocations    = temp.allocations;
    m_metrics.capacity       = temp.capacity;
    m_metrics.wastedSpace    = temp.wastedSpace;
    m_metrics.blocks         = temp.blocks;
}

ThreadScratchAllocator::ThreadScratchAllocator(ArenaAllocator& arena)
: arena(arena)
, tempMemory(arena.beginTempMemory())
{
}

ThreadScratchAllocator::~ThreadScratchAllocator()
{
    arena.endTempMemory(tempMemory, /*clearMemory*/ false);
}

ThreadScratchAllocator getThreadScratchAllocator(const ArenaAllocator* other)
{
    thread_local ArenaAllocator threadArenas[2] = {};
    ArenaAllocator& arena                       = &threadArenas[0] == other ? threadArenas[1] : threadArenas[0];
    if (!arena.m_curr) {
        arena.grow(JACK_KILOBYTES(64), /*commitSize*/ JACK_KILOBYTES(64));
    }

    return ThreadScratchAllocator(arena);
}

/******************************************************************************
 * ChunkAllocator
 ******************************************************************************/
/// Atomically perform max(src, newValue) on a size_t value
static void atomicMaxUSize(std::atomic<size_t>& src, size_t newValue)
{
    for (bool done = false; !done; ) {
        size_t srcValue = src.load();
        done            = srcValue < newValue ? src.compare_exchange_strong(srcValue, /*expected*/
                                                                            newValue, /*desired*/
                                                                            std::memory_order_acq_rel)
                                              : true;
    }
}

void *ChunkAllocator::allocate(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA)
{
    ZoneScoped;
    JACK_ASSERT_MSG((m_chunkSize >= sizeof(FreeList)), "The chunk size must be big enough to construct a free list entry");
    JACK_ASSERT_MSG((m_alignment > 0), "Alignment cannot be 0");
    JACK_ASSERT_MSG(((m_alignment & (m_alignment - 1)) == 0), "Require power of two alignment");

    /// Pull a chunk from the free list atomically using an atomic CAS
    void *result = nullptr;
    for (bool done = false; !done; ) {
        auto *expected = m_freeList.load(std::memory_order_acquire);
        if (expected) {
            auto *desired = expected->m_next;
            if (expected == expected->m_next) {
                JACK_ASSERT(!"Cyclic free list detected");
            }
            done = m_freeList.compare_exchange_strong(expected, desired, std::memory_order_acq_rel);

            if (done) {
                /// Explicitly end the chunk's object lifetime because of strict
                /// aliasing rules.
                result = new (expected) char[m_chunkSize];
            }
        } else {
            break;
        }
    }

    /// If no chunk is available, allocate one using our
    if (!result) {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        result = m_arena.allocate(JACK_CALL_SITE_INPUT m_chunkSize, m_alignment);
        JACK_ASSERT_MSG((reinterpret_cast<uintptr_t>(result) & (m_alignment - 1)) == 0, "Internal error: Pointer did not get aligned correctly");

        /// Successfully allocated, update the arena's chunk counts
        if (result) {
            size_t chunkCount = m_metrics.m_chunks.fetch_add(1) + 1;
            atomicMaxUSize(m_metrics.m_chunksHighWaterMark, chunkCount);
        }
    }

    /// Update the number of handed out chunks in the allocator
    if (result) {
        size_t usedCount = m_metrics.m_usedChunks.fetch_add(1) + 1;
        atomicMaxUSize(m_metrics.m_usedChunksHighWaterMark, usedCount);
    }

    #ifndef NDEBUG
    if (result) {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        if (!m_arena.owns(result)) {
            JACK_ASSERT(!"Internal error: Allocated chunk didn't originate from allocator");
        }
    }
    #endif

    return result;
}

void ChunkAllocator::give(JACK_CALL_SITE_ARGS void* ptr, bool clearMemory)
{
    #ifdef JACK_ALLOCATION_TRACKING
    (void)file; (void)func; (void)line; /// \todo Use the call site arguments?
    #endif /// JACK_ALLOCATION_TRACKING

    if (!ptr) {
        return;
    }

    JACK_ASSERT_MSG(((reinterpret_cast<uintptr_t>(ptr) & (m_alignment - 1)) == 0),
                    "The pointer returned is incorrect because it is not aligned correctly");

    #ifndef NDEBUG
    {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        if (!JACK_CHECK(m_arena.owns(ptr))) {
            JACK_ERROR("The returned pointer was not allocated from this allocator {}", dumpAllocation(JACK_CALL_SITE_INPUT 0, ptr));
            return;
        }
    }
    #endif

    if (clearMemory) {
        memset(ptr, 0xAA, m_chunkSize);
    }

    /// \note On memory returned by the user it may possibly have been used
    /// to construct an object. We can reuse this pointer as a Chunk
    /// by using placement new onto the memory again which formally ends the
    /// previous object lifetime and starts a Chunk object's lifetime, i.e.
    /// the optimizer will not mess with this as it is not UB.
    ///
    /// This is permitted by the C++11 standard in section 3.8.4 page
    /// 66, see:
    /// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf
    /// https://en.cppreference.com/w/cpp/language/lifetime
    ///
    /// A program may end the lifetime of any object by reusing the storage
    /// which the object occupies or by explicitly calling the destructor
    /// for an object of a class type with a non-trivial destructor.
    auto *desired = new (ptr) FreeList;

    /// Return the chunk back to the free list atomically using an atomic CAS
    for (bool swapped = false; !swapped; ) {
        desired->m_next = m_freeList.load(std::memory_order_acquire);
        if (desired == desired->m_next) {
            JACK_ASSERT(!"Cyclic free list entry detected");
        }
        swapped = m_freeList.compare_exchange_strong(desired->m_next, desired, std::memory_order_acq_rel);
    }

    /// Update the number of chunks being used
    size_t prevUsedChunks = m_metrics.m_usedChunks.fetch_sub(1);
    (void)prevUsedChunks;
    JACK_ASSERT_MSG(prevUsedChunks > 0,
                    "Allocator is trying to return the same chunk multiple "
                    "times such that the number of used chunks has become invalid");
}

void ChunkAllocator::freeAllocator(bool clearMemory)
{
    m_freeList.store(nullptr, std::memory_order_release);

    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    m_arena.freeAllocator(clearMemory);
}

void *virtualReserve(size_t size, bool commit)
{
    ZoneScoped;
    #if defined(_WIN32)
    unsigned long flags = MEM_RESERVE;
    if (commit) {
        flags |= MEM_COMMIT;
    }
    void *result = ::VirtualAlloc(nullptr, size, flags, PAGE_READWRITE);
    #else
    unsigned flags = PROT_NONE;
    if (commit) {
        flags = PROT_READ | PROT_WRITE;
    }
    void *result = mmap(nullptr, size, flags, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        result = nullptr;
    }
    #endif
    TracyAlloc(result, size);
    return result;
}

bool virtualCommit(void* ptr, size_t size)
{
    ZoneScoped;
    #if defined(_WIN32)
    bool result = ::VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
    #else
    /// All pointers returned by mmap are aligned to page boundaries so aligning
    /// this should always be valid so long as our assumption that the page size
    /// is 4kb is correct.
    const size_t page_size      = JACK_KILOBYTES(4);
    auto         ptrAddress     = reinterpret_cast<uintptr_t>(ptr);
    size_t       offset         = ptrAddress & (page_size - 1);
    void *       ptrPageAligned = static_cast<char *>(ptr) - offset;
    bool         result         = mprotect(ptrPageAligned, size, PROT_READ | PROT_WRITE) == 0;
    #endif
    return result;
}

void virtualDecommit(void* ptr, size_t size)
{
    ZoneScoped;
    #if defined(_WIN32)
    ::VirtualFree(ptr, size, MEM_DECOMMIT);
    #else
    mprotect(ptr, size, PROT_NONE);

    /// \note Previously we used MADV_FREE which releases the pages from memory
    /// lazily when the operating system is under memory pressure. We target
    /// Ubuntu 16.04 which comes with Linux kernel 4.4 which predates the
    /// introduction of this flag.
    ///
    /// We fall back to MADV_DONTNEED which immediately frees the backing page.
    /// This has the side benefit that memory usage of the process is more
    /// accurately reflected in real time statistics tools.
    madvise(ptr, size, MADV_DONTNEED);
    #endif
}

void virtualRelease(void* ptr, size_t size)
{
    ZoneScoped;
    #if defined(_WIN32)
    BOOL result = ::VirtualFree(ptr, 0, MEM_RELEASE);
    (void)result; (void)size;
    #else
    munmap(ptr, size);
    #endif
    TracyFree(ptr);
}

/******************************************************************************
 * Strings
 ******************************************************************************/
/// \note C++ does not have starts_with and ends_with until C++20 so we have our
/// own implementations ...
bool stringStartsWith(std::string_view src, std::string_view prefix)
{
    std::string_view substring;
    if (src.size() >= prefix.size()) {
        substring = std::string_view(src.data(), prefix.size());
    }
    bool result = substring == prefix;
    return result;
}

bool stringEndsWith(std::string_view src, std::string_view suffix)
{
    std::string_view substring;
    if (src.size() >= suffix.size()) {
        substring = std::string_view(src.data() + src.size(), suffix.size());
    }
    bool result = substring == suffix;
    return result;
}

std::string_view stringTrimWhitespace(std::string_view src)
{
    std::string_view result = src;
    while (result.size() && (result.front() == '\n' || result.front() == '\r' || result.front() == '\t' || result.front() == ' ')) {
        result = result.substr(1);
    }

    while (result.size() && (result.back() == '\n' || result.back() == '\r' || result.back() == '\t' || result.back() == ' ')) {
        result = result.substr(0, result.size() - 1);
    }
    return result;
}

std::string_view stringCopy(ArenaAllocator& arena, std::string_view string)
{
    std::string_view result = {};
    if (string.size()) {
        char *buffer = JACK_ALLOCATOR_ALLOC_ARRAY(&arena, char, string.size() + 1 /*null-terminator*/);
        if (buffer) {
            result = std::string_view(buffer, string.size());
            memcpy(buffer, string.data(), string.size());
        }
    }
    return result;
}

/******************************************************************************
 * StringBuilder
 ******************************************************************************/
bool StringBuilder::appendRef(std::string_view string)
{
    if (string.empty() || string.size() <= 0) {
        return false;
    }

    Link *link = JACK_ALLOCATOR_ALLOC(&m_arena, Link);
    if (!link) {
        return false;
    }

    link->string = string;
    link->next   = NULL;

    if (m_head)
        m_tail->next = link;
    else
        m_head = link;

    m_tail = link;
    m_count++;
    m_stringSize += string.size();
    return true;
}

bool StringBuilder::appendCopy(std::string_view string)
{
    std::string_view copy = stringCopy(m_arena, string);
    bool    result        = appendRef(copy);
    return result;
}

std::string_view StringBuilder::toStringArena(ArenaAllocator& arena) const
{
    std::string_view result = {};
    if (m_stringSize <= 0 || m_count <= 0) {
        return result;
    }

    char *buffer       = JACK_ALLOCATOR_ALLOC_ARRAY(&arena, char, m_stringSize + 1);
    size_t bufferCount = 0;
    if (!buffer) {
        return result;
    }

    for (Link *link = m_head; link; link = link->next) {
        memcpy(buffer + bufferCount, link->string.data(), link->string.size());
        bufferCount += link->string.size();
    }

    result = std::string_view(buffer, bufferCount);
    JACK_ASSERT(result.size() == m_stringSize);
    return result;
}

std::string StringBuilder::toString() const
{
    std::string result = {};
    result.reserve(m_stringSize);
    for (Link *link = m_head; link; link = link->next) {
        result += link->string;
    }
    JACK_ASSERT(result.size() == m_stringSize);
    return result;
}
} /// namespace aos::jack
