#ifndef JACK_CORE_LIB_H
#define JACK_CORE_LIB_H
/******************************************************************************
 * @file corelib.h
 * @brief The core programming primitives that is generally expected to be
 * available across the entire codebase and projects using JACK. This file's
 * dependencies should be kept to the absolute minimum to avoid dependency
 * creep.
 ******************************************************************************/

/// Third Party
#include <array>                           /// Conversion from std::array into JACK containers
#include <cstring>                         /// std::memmove
#include <vector>                          /// Conversion from STL containers into JACK containers
#include <fmt/core.h>                      /// String formatting primitives
#include <tracy/Tracy.hpp>                 /// Profiling
#include <unordered_map>                   /// Allocation table
#include <mutex>                           /// Thread-safe event allocator/metrics
#include <atomic>                          /// Thread-safe event allocator

using namespace std::string_view_literals; /// Allow use of ""sv literals

namespace aos::log
{
enum class Severity : uint8_t
{
    Debug,
    Info,
    Warning,
    Error,
    Critical,
    Fatal,
    Custom,
    Count,
};
} // namespace aos::log

/******************************************************************************
 * Break to debugger macro
 ******************************************************************************/
/// Macro to break to the debugger
#if !defined(JACK_DEBUG_BREAK)
    #if defined(_MSC_VER) || defined(__MINGW32__)
        #define JACK_DEBUG_BREAK __debugbreak()
    #elif defined(__clang__) || defined(__GNUC__)
        #include <signal.h>
        #define JACK_DEBUG_BREAK raise(SIGTRAP)
    #else
        #define JACK_DEBUG_BREAK assert(!"Debug break")
    #endif
#endif

namespace aos::jack
{
#if !defined(JACK_DUMP_STACK_TRACE)
    #if defined(JACK_WITH_STACK_TRACE)
    #define JACK_DUMP_STACK_TRACE cpptrace::print()
    #else
    #define JACK_DUMP_STACK_TRACE
    #endif
#endif

/// Preprocessor token pasting work-around macro
#define JACK_STRINGIFY2(token) #token
#define JACK_STRINGIFY(token) JACK_STRINGIFY2(token)

/******************************************************************************
 * Assert Macros
 ******************************************************************************/
#if !defined(JACK_ASSERT_FMT)
    #define JACK_ASSERT_FMT(expr, fmt, ...)                                                                                                                     \
        do {                                                                                                                                                    \
            if (!(expr)) {                                                                                                                                      \
                aos::jack::ThreadScratchAllocator scratch_ = aos::jack::getThreadScratchAllocator(nullptr);                                                     \
                std::string_view log                       = aos::jack::fmtAllocInternal(scratch_.arena, FMT_STRING(fmt), __VA_ARGS__);                         \
                JACK_CRITICAL("Assertion triggered: expression was '" #expr "' [file='" __FILE__ ":" JACK_STRINGIFY(__LINE__) "']\n{}", log.size() ? log : ""); \
                JACK_DUMP_STACK_TRACE;                                                                                                                          \
                JACK_DEBUG_BREAK;                                                                                                                               \
            }                                                                                                                                                   \
        } while (0)
#endif

#if !defined(JACK_ASSERT_MSG)
    #define JACK_ASSERT_MSG(expr, msg) JACK_ASSERT_FMT(expr, "{}", msg)
#endif

#if !defined(JACK_ASSERT)
    #define JACK_ASSERT(expr) JACK_ASSERT_FMT(expr, "", "")
#endif

#if !defined(JACK_INVALID_CODE_PATH)
    /// \note Tests trigger some invalid code paths to check the core handles them
    /// appropriately. In those cases we do not trigger our traps.
    #if defined(JACK_WITH_TESTS)
    #define JACK_INVALID_CODE_PATH
    #else
    #define JACK_INVALID_CODE_PATH JACK_ASSERT_MSG(false, "Invalid code path"sv)
    #endif
#endif

/******************************************************************************
 * Check macro
 ******************************************************************************/
/// Check the expression trapping in debug, whilst in release- trapping is
/// removed and the expression is evaluated as if it were a normal 'if' branch.
///
/// This allows handling of the condition gracefully when compiled out but traps
/// to notify the developer in builds when it's compiled in.
#if 0
    bool flag = true;
    if (JACK_CHECK(flag)) {
        // This branch will execute!
    } else {
        // Prints "Flag was false!"
    }
#endif

#if !defined(JACK_CHECK)
    #if defined(NDEBUG)
        #define JACK_CHECK(expr) (expr)
    #else
        #define JACK_CHECK(expr) ((expr) ? true : (JACK_DEBUG_BREAK, false))
    #endif
#endif

/******************************************************************************
 * Printing Macros
 ******************************************************************************/
/// Print a std::string into C-style variable argument printing functions using
/// "%.*s"
#if 0
    std::string string = "Hello world";
    printf("%.*s\n", JACK_STRING_FMT(string)); /// Prints "Hello world"
#endif
#define JACK_STRING_FMT(string) static_cast<int>((string).size()), (string).data()


/******************************************************************************
 * @brief Enum to denote the status of a job or task in JACK generally.
 ******************************************************************************/
enum class FinishState
{
    NOT_YET,
    FAILED,
    DROPPED,
    SUCCESS,
};

/******************************************************************************
 * Typedefs
 ******************************************************************************/
using float32 = float;
using float64 = double;

/******************************************************************************
 * Strings
 ******************************************************************************/
bool             stringStartsWith(std::string_view src, std::string_view prefix);
bool             stringEndsWith(std::string_view src, std::string_view suffix);
std::string_view stringTrimWhitespace(std::string_view src);
std::string_view stringCopy(class ArenaAllocator& arena, std::string_view string);

/// Use this macro to format a string. This macro uses the FMT_STRING macro to
/// enable compile time checks on the given format string. The 'Alloc' macro
/// allocates the string from an arena and the other macro uses the default
/// STL allocator.
#define JACK_FMT_ALLOC(arena, fmtLiteral, ...) \
    aos::jack::fmtAllocInternal(arena, FMT_STRING(fmtLiteral), ## __VA_ARGS__)

#define JACK_FMT(fmtLiteral, ...) \
    fmt::format(FMT_STRING(fmtLiteral), ## __VA_ARGS__)

template <typename... T>
std::string_view fmtAllocInternal(class ArenaAllocator& arena, fmt::format_string<T...> fmt, const T&... args);

/******************************************************************************
 * Logging
 ******************************************************************************/
extern const bool  JACK_SLOW_DEBUG_CHECKS;
extern       void *gJACKInstance;

void setLog(log::Severity level, bool enabled);
bool isLogged(log::Severity level);
void printBar(std::string_view title, int colour = 44);
void log(log::Severity level, std::string_view log, std::string_view channel, int line, std::string_view file);

template <typename... T>
void logFmt(log::Severity level, std::string_view channel, int line, std::string_view file, fmt::format_string<T...> fmt, T &&...args);

/******************************************************************************
 * Logging Macros
 ******************************************************************************/
#if !defined(JACK_LOG)
    #define JACK_LOG(level, channel, fmt, ...) do { \
            if (aos::jack::isLogged(level)) { \
                aos::jack::logFmt(level, channel, __LINE__, __FILE__, FMT_STRING(fmt), __VA_ARGS__); \
            } \
        } while (0)
#endif

/// \note Compiling with '-pedantic' on GCC will complain about
///
/// ISO C++11 requires at least one argument for the "..." in a variadic macro [-Werror]
///
/// We compile in C++17 which means libfmt does not do compile time string
/// checks. In our macros, we surround out string literals by 'FMT_STRING()' to
/// enforce compile time checks in C++17. Hence we need to separate the 'fmt'
/// parameters from the variadic list.
///
/// Because of this, if we call 'JACK_FATAL("your fatal message")' then GCC
/// complains that the variadic macro needs at-least one argument. For these
/// situations we provide a 2nd macro e.g. 'JACK_FATAL_MSG' which is the next
/// best alternative.
///
/// When we switch to C++23 compile time checks are enabled by default in
/// libfmt, this macro can then be resimplified to
///
/// #define JACK_FATAL(...) JACK_LOG(aos::log::Severity::Fatal, "", __VA_ARGS__)
///
/// Since all invocations of logging macros will have atleast one argument, the
/// log message itself then this warning will go away.
#define JACK_FATAL(fmt, ...)      JACK_LOG(aos::log::Severity::Fatal,    "",    fmt, __VA_ARGS__)
#define JACK_CRITICAL(fmt, ...)   JACK_LOG(aos::log::Severity::Critical, "",    fmt, __VA_ARGS__)
#define JACK_ERROR(fmt, ...)      JACK_LOG(aos::log::Severity::Error,    "",    fmt, __VA_ARGS__)
#define JACK_WARNING(fmt, ...)    JACK_LOG(aos::log::Severity::Warning,  "",    fmt, __VA_ARGS__)
#define JACK_DEBUG(fmt, ...)      JACK_LOG(aos::log::Severity::Debug,    "",    fmt, __VA_ARGS__)
#define JACK_INFO(fmt, ...)       JACK_LOG(aos::log::Severity::Info,     "",    fmt, __VA_ARGS__)
#define JACK_BUS(fmt, ...)        JACK_LOG(aos::log::Severity::Custom,   "BUS", fmt, __VA_ARGS__)

#define JACK_FATAL_MSG(string)    JACK_FATAL("{}", string)
#define JACK_CRITICAL_MSG(string) JACK_CRITICAL("{}", string)
#define JACK_ERROR_MSG(string)    JACK_ERROR("{}", string)
#define JACK_WARNING_MSG(string)  JACK_WARNING("{}", string)
#define JACK_DEBUG_MSG(string)    JACK_DEBUG("{}", string)
#define JACK_INFO_MSG(string)     JACK_INFO("{}", string)
#define JACK_BUS_MSG(string)      JACK_BUS("{}", string)

#define JACK_WARNING_ONCE(fmt, ...)                          \
    for (static bool once_ = false; !once_; once_ = true) {  \
        JACK_WARNING(fmt, __VA_ARGS__);                      \
    }

#define JACK_WARNING_ONCE_MSG(string) JACK_WARNING_ONCE("{}", string)

/******************************************************************************
 * Allocators
 ******************************************************************************/
/// \todo Future improvements
/// - Store the stack trace of allocations for improved tracing UX
/// - Change the mutex around the trace table using atomic instructions
/// - Add guard pages around allocations to trigger page faults on out of bounds
///   accesses/writes

#ifndef JACK_ALLOCATOR_CLEAR_MEMORY
    #ifdef NDEBUG
        #define JACK_ALLOCATOR_CLEAR_MEMORY false
    #else
        #define JACK_ALLOCATOR_CLEAR_MEMORY true
    #endif
#endif

/******************************************************************************
 * Debug Call Site Macros
 * When JACK_ALLOCATION_TRACKING is not defined, no call site information is tracked
 * with allocations which slims down call stack for allocating calls.
 ******************************************************************************/
#ifdef JACK_ALLOCATION_TRACKING
    #define JACK_CALL_SITE_ARGS char const *file, char const *func, int line,
    #define JACK_CALL_SITE_ARGS_NO_TAIL_COMMA char const *file, char const *func, int line

    #define JACK_CALL_SITE_INPUT file, func, line,
    #define JACK_CALL_SITE_INPUT_NO_TAIL_COMMA file, func, line

    #define JACK_CALL_SITE __FILE__, __func__, __LINE__,
    #define JACK_CALL_SITE_NO_TAIL_COMMA __FILE__, __func__, __LINE__
#else
    #define JACK_CALL_SITE_ARGS
    #define JACK_CALL_SITE_ARGS_NO_TAIL_COMMA

    #define JACK_CALL_SITE_INPUT
    #define JACK_CALL_SITE_INPUT_NO_TAIL_COMMA

    #define JACK_CALL_SITE
    #define JACK_CALL_SITE_NO_TAIL_COMMA
#endif

/******************************************************************************
 * Allocation Macros
 * Use these macros for allocating calls in JACK to ensure allocations get
 * instrumented when JACK_ALLOCATION_TRACKING is defined
 *
 * *_NEW variants:          Allocate a single object, forwarding arguments to
 *                          the object's constructor
 *
 * *_ALLOC variants:        Allocate and default construct a single object.
 *                          \note This is required since passing zero arguments
 *                          for variadic macros is disallowed in GCC, hence we
 *                          can't use *_NEW with zero arguments.
 *
 * *_DELETE variants:       Allocating using NEW/ALLOC must be deallocated with
 *                          a DELETE call.
 *
 * *_DELETE_ARRAY variants: Allocating using NEW_ARRAY/ALLOC_ARRAY must be
 *                          deallocated with a DELETE_ARRAY call.
 *
 ******************************************************************************/
#define JACK_NEW(Type, ...) JACK_ALLOCATOR_NEW(&aos::jack::globalHeapAllocator, Type, __VA_ARGS__)
#define JACK_ALLOC(Type) JACK_ALLOCATOR_ALLOC(&aos::jack::globalHeapAllocator, Type)
#define JACK_DELETE(ptr) JACK_ALLOCATOR_DELETE(&aos::jack::globalHeapAllocator, ptr)

#define JACK_ALLOC_ARRAY(Type, count) JACK_ALLOCATOR_ALLOC_ARRAY(&aos::jack::globalHeapAllocator, Type, count)
#define JACK_DELETE_ARRAY(ptr) JACK_ALLOCATOR_DELETE_ARRAY(&aos::jack::globalHeapAllocator, ptr)

#define JACK_ALLOCATOR_NEW(allocator, Type, ...) (allocator)->newInternal<Type>(JACK_CALL_SITE __VA_ARGS__)
#define JACK_ALLOCATOR_ALLOC(allocator, Type) (allocator)->newInternal<Type>(JACK_CALL_SITE_NO_TAIL_COMMA)
#define JACK_ALLOCATOR_DELETE(allocator, ptr) do { (allocator)->deleteInternal(JACK_CALL_SITE ptr); delete ptr; } while(0)

#define JACK_ALLOCATOR_ALLOC_ARRAY(allocator, Type, count) (allocator)->newArrayInternal<Type>(JACK_CALL_SITE count)
#define JACK_ALLOCATOR_DELETE_ARRAY(allocator, ptr) do { (allocator)->deleteInternal(JACK_CALL_SITE ptr); delete[] ptr; } while(0)

#define JACK_CHUNK_ALLOCATOR_GIVE(allocator, Type, ptr, clearMemory) do { (ptr)->~Type(); (allocator)->give(JACK_CALL_SITE ptr, clearMemory); } while(0)

/// \note Special cloning macro that uses the 'clone' member function of the
/// object that logs the call site to the allocation tracker.
#define JACK_CLONE_BDI_OBJECT(object) ((object) ? (object)->clone(JACK_CALL_SITE_NO_TAIL_COMMA) : nullptr)

/******************************************************************************
 * Smart Pointer Macros
 * Use these macros for smart pointers to ensure they get tracked by our memory
 * subsystem.
 ******************************************************************************/
#define JACK_UNIQUE_PTR(Type) std::unique_ptr<Type, void (*)(Type *)>

/// \note The init functions construct a smart pointer by taking ownership
/// of 'ptr'
#define JACK_INIT_UNIQUE_PTR(Type, ptr) \
    std::unique_ptr<Type, void (*)(Type *)>(ptr, [](Type *deletePtr) { JACK_DELETE(deletePtr); })

#define JACK_INIT_SHARED_PTR(Type, ptr)                                         \
    std::shared_ptr<Type>(ptr, [](Type *deletePtr) { JACK_DELETE(deletePtr); })

/******************************************************************************
 * Memory Size Macros
 ******************************************************************************/
#define JACK_KILOBYTES(val) (1024ULL * (val))
#define JACK_MEGABYTES(val) (1024ULL * JACK_KILOBYTES(val))
#define JACK_GIGABYTES(val) (1024ULL * JACK_MEGABYTES(val))

/******************************************************************************
 * Allocation Metadata
 ******************************************************************************/
struct AllocationRecord
{
    size_t      size; /// The size of the allocation
    #ifdef JACK_ALLOCATION_TRACKING
    const char* file; /// The file that the allocation was made
    const char* func; /// The function that the allocation was made
    int         line; /// The line number of the file the allocation was made
    #endif /// JACK_ALLOCATION_TRACKING
};

struct AllocationEntry
{
    AllocationRecord freedRecord; /// The location that the allocation was freed (only if freed is true)
    AllocationRecord record;      /// The location that the allocation was done at
    bool             freed;       /// True if the allocation associated with this record is freed
};

struct AllocationTracker
{
    /// Add an allocation to the allocation table
    void             add(JACK_CALL_SITE_ARGS void *ptr, size_t size);

    /// A string with a dump of all allocations in the tracker
    /// @param uptimeInMs (Optional) The total running duration that the
    /// allocation tracker will use to report the rate of allocations per
    /// second. Pass in 0 if not applicable.
    std::string      stats(uint64_t uptimeInMs) const;

    /// Delete all the currently tracked allocations
    void             clear() { m_allocations.clear(); }

    /// Find the allocation record for the pointer
    AllocationEntry* find(const void* ptr);

    /// @return True if the arguments were valid and the record metadata was
    /// able to be set freed.
    bool             markPointerFree(JACK_CALL_SITE_ARGS const void *ptr);

    /// @return True if the arguments were valid and the record metadata was
    /// able to be set freed.
    bool             markEntryFree(JACK_CALL_SITE_ARGS AllocationEntry* entry, const void* ptr);

    /// The allocation table
    std::unordered_map<const void*, AllocationEntry> m_allocations;
};

struct AllocatorMetrics /// The usage metrics of an allocator
{
    /// The number of bytes currently allocated
    size_t bytesAllocated = 0;

    /// The high water mark of bytes allocated
    size_t bytesAllocatedHighWaterMark = 0;

    /// The total number of bytes allocated cumulative
    size_t totalBytesAllocated = 0;

    /// The number of current allocations
    size_t allocations = 0;

    /// The high water mark of current allocations
    size_t allocationsHighWaterMark = 0;

    /// The total number of allocations cumulative
    size_t totalAllocations = 0;
};

/******************************************************************************
 * HeapAllocator
 * A allocator that allocates memory from the system i.e. new() and is useful
 * for dynamic object allocations that has a lifetime that is not well-defined.
 *
 * This allocator is thread safe.
 ******************************************************************************/
class HeapAllocator
{
public:
    /// @return The current usage metrics of the allocator
    const AllocatorMetrics& metrics() const { return m_metrics; }

    /// Track the given pointer by creating the allocation metadata record for it
    void addAllocationMetadata(JACK_CALL_SITE_ARGS void* ptr, size_t allocationSize);

    /// NOTE: I've decided to *not* override placement new since declaring that
    /// symbol globally can leak into other projects which is a bunch of
    /// maintenance work and needing to be a C/C++ language lawyer which can
    /// be avoided by specifically hand instrumenting our code.
    ///
    /// Irrespective of overriding placement new, we still want macros to
    /// easily fill in the CALL_SITE arguments where the allocation is requested
    /// anyway and we hide this behind macros for the user so  placement new
    /// would not allow instrumenting our allocations any simpler.
    ///
    /// CALL_SITE arguments are also compiled out to reduce the function call
    /// overhead if the allocation tracking macro is not defined.

    /// Allocate an array of objects, use the allocation macros to ensure the
    /// call site arguments are passed in appropriately.
    template <typename T>
    T* newArrayInternal(JACK_CALL_SITE_ARGS size_t count)
    {
        auto* result = new T[count];
        TracyAlloc(result, sizeof(T) * count);
        addAllocationMetadata(JACK_CALL_SITE_INPUT result, sizeof(T) * count);
        return result;
    }

    /// Allocate an object, use the allocation macros to ensure the call
    /// site arguments are passed in appropriately.
    template <typename T, typename... Args>
    T* newInternal(JACK_CALL_SITE_ARGS Args &&...args)
    {
        auto* result = new T(std::forward<Args>(args)...);
        TracyAlloc(result, sizeof(T));
        addAllocationMetadata(JACK_CALL_SITE_INPUT result, sizeof(T));
        return result;
    }

    /// Delete an object from the allocation metadata records, use the
    /// allocation macros to ensure the call site arguments are passed in
    /// appropriately.
    void deleteInternal(JACK_CALL_SITE_ARGS const void *ptr);

    /// The allocation database
    AllocationTracker m_allocationTracker = {};

private:
    /// The usage metrics of the allocator
    AllocatorMetrics m_metrics = {};
};

/******************************************************************************
 * ArenaAllocator
 * An allocator that allocates from a fixed block of memory. This implementation
 * is a growing allocator and additional blocks of memory are chained into
 * a linked-list of blocks of memory when the allocator runs out of space.
 *
 * Arena's can not free individual pointers, only fixed blocks of memory at
 * a time. It is useful for allocations with a well defined lifetime or
 * short-lived allocations. By reusing the fixed memory block we avoid calls
 * into the kernel, minimise cache misses and reduce memory fragmentation.
 *
 * This allocator is thread safe using a recursive mutex.
 ******************************************************************************/
class ArenaAllocator
{
public:
    struct Block
    {
        Block* prev      = nullptr; /// Next memory block in the linked list
        Block* next      = nullptr; /// Previous memory block in the linked list
        size_t committed = 0;       /// The number of bytes backed by physical pages
        size_t used      = 0;       /// The number of bytes used up in the block. Always <= commit size.
        size_t size      = 0;       /// The size of the block in bytes
        char*  memory    = nullptr; /// The backing memory of the block
    };

    /// Captures a snapshot of the arena for reverting when the scope is ended.
    struct TempMemory
    {
        /// Blocks
        Block* tail;
        Block* curr;
        Block* currNext;
        size_t currUsed;

        /// Metrics
        size_t   bytesAllocated;
        size_t   allocations;
        size_t   space;
        size_t   capacity;
        size_t   wastedSpace;
        uint32_t blocks;
    };

    struct Metrics : public AllocatorMetrics
    {
        /// The total amount of memory committed from the OS
        size_t   commitSize = 0;

        /// The number of times the arena had to call the OS layer (i.e. to
        /// reserve memory or commit memory from the OS).
        size_t   sysCalls = 0;

        /// The current total allocating capacity in bytes of the arena
        size_t   capacity = 0;

        /// The high water mark of capacity
        size_t   capacityHighWaterMark = 0;

        /// The amount of bytes that are left over in all the blocks because
        /// a requested allocation required appending a new block to the arena.
        size_t   wastedSpace = 0;

        /// The high water mark of wasted space
        size_t   wastedSpaceHighWaterMark = 0;

        /// The number of memory blocks currently in the arena
        uint32_t blocks = 0;

        /// The high water mark of blocks
        uint32_t blocksHighWaterMark = 0;
    };

    class TempMemoryScope
    {
    public:
        TempMemoryScope(ArenaAllocator& arena);
        ~TempMemoryScope();
        ArenaAllocator& m_arena;
        TempMemory      m_tempMemory;
    };

    /// Allocate raw bytes from the arena.
    /// @param size The size in bytes to allocate from the arena
    /// @param alignment A power of two value to which the returned pointer will
    /// be aligned to (i.e. 1, 2, 4, 8, 16 ... e.t.c)
    /// @return The aligned allocation, a null pointer if allocation failed.
    void* allocate(JACK_CALL_SITE_ARGS size_t size, uint8_t alignment);

    /// Grow the allocating capacity of the arena
    /// @param size The amount of bytes to reserve from OS for the the capacity
    /// of the arena
    /// @param commitSize The amount of bytes to commit as physical pages from the
    /// OS for the arena
    /// @return True if growing succeeded, false if system is out of memory.
    bool grow(size_t size, size_t commitSize);

    /// Free all the memory occupied by the arena, all pointers handed out are
    /// invalidated after this call.
    void freeAllocator(bool clearMemory);

    /// Reset all arena block usage to 0 and set the arena to allocate from the
    /// head of the memory block list.
    void reset(bool clearMemory);

    /// @return True if the pointer was allocated from the arena, false if
    /// otherwise.
    bool owns(const void *ptr) const;

    /// Begin an allocation scope where all allocations are undone between the
    /// begin and end scope call. Useful for short lived, highly defined
    /// allocations life-times. Freeing the arena invalidates the scope
    /// generated by this function.
    TempMemory beginTempMemory();

    /// End a temporary scope generated by the beginTemporaryScope call.
    void endTempMemory(TempMemory temp, bool clearMemory);

    /// Create a scope object that begins a temp memory object. On scope exit,
    /// the object destructs ending the temp memory scope which releases memory
    /// allocated in the contained scope.
    [[nodiscard]] TempMemoryScope tempMemoryScope() { return TempMemoryScope(*this); }

    /// @return The current usage metrics of the allocator
    const Metrics& metrics() const { return m_metrics; }

    /// @return The tail block the arena will allocate from of the arena
    const Block* tail() const { return m_tail; }

    /// @return The current block the arena is allocating out of the arena
    const Block* curr() const { return m_curr; }

    /// Allocate an array of objects, use the allocation macros to ensure the
    /// call site arguments are passed in appropriately.
    template <typename T>
    T* newArrayInternal(JACK_CALL_SITE_ARGS size_t count)
    {
        auto* raw    = allocate(JACK_CALL_SITE_INPUT sizeof(T) * count, alignof(T));
        T*    result = raw ? new (raw) T[count] : nullptr;
        return result;
    }

    /// Allocate an object, use the allocation macros to ensure the call
    /// site arguments are passed in appropriately.
    template <typename T, typename... Args>
    T* newInternal(JACK_CALL_SITE_ARGS Args &&...args)
    {
        auto *raw    = allocate(JACK_CALL_SITE_INPUT sizeof(T), alignof(T));
        auto *result = raw ? new (raw) T(std::forward<Args>(args)...) : nullptr;
        return result;
    }

    /// The size in bytes to grow the arena when allocation is requested and
    /// there's insufficient memory for allocation. This parameter is ignored if
    /// the grow function is manually called.
    size_t m_minBlockSize = JACK_MEGABYTES(4);

    /// The usage metrics of the allocator
    Metrics m_metrics = {};

    /// The current block the arena is allocating from
    Block* m_curr = nullptr;

    /// The last block in the linked list of blocks that the arena can allocate
    /// from.
    Block* m_tail = nullptr;
};

struct ArenaLink
{
    ArenaAllocator arena;
    ArenaLink      *next;
};

/******************************************************************************
 * A thread local arena that is used for short-lived allocations within the
 * same scope lexical scope. On scope exit the memory allocated from the arena
 * is released.
 ******************************************************************************/
class ThreadScratchAllocator
{
public:
    ThreadScratchAllocator(ArenaAllocator& arena);
    ~ThreadScratchAllocator();
    ArenaAllocator&            arena;
    ArenaAllocator::TempMemory tempMemory;
};

/// Retrieve the arena allocator for the current thread. These are stored in
/// thread local storage so in a multithreaded context does not exhibit race
/// conditions between threads. Memory allocated from these allocators should
/// not be shared between threads irrespective without synchronisation.
///
/// @param[in] Pass in the arena if any currently in the lexical scope. This
/// prevents returning the same scratch allocator twice. For example if you pass
/// a scratch arena into a function and then request the scratch allocator again
/// in the function, the inner scratch allocator could undo allocations from the
/// outer scope.
ThreadScratchAllocator getThreadScratchAllocator(const ArenaAllocator *otherArena);

/******************************************************************************
 * Chunk Allocator
 *
 * The chunk allocator stores chunks as a linked-list of buffers of chunk
 * size. To maintain the linked-list, each chunk has a header that is stored
 * inline in the chunk's buffer and is referred to as the free list. The free
 * list header contains a pointer to the next free list entry.
 *
 * When the allocator hands out chunks, we pull from the free-list if there
 * are any chunks or otherwise dynamically grow the allocator with new
 * chunks.
 *
 * Each free list entry is type-punned back to a byte buffer that has the
 * size of a chunk and can be overwritten by the user that requested the
 * allocation.
 *
 * When the chunk is returned, the free list header is reinitialised onto the
 * chunk and re-appended back onto the free list allowing re-use of the memory.
 *
 * This allocator is thread safe by using CAS on the free list implemented as
 * a linked list.
 ******************************************************************************/
class ChunkAllocator
{
public:
    struct FreeList { FreeList *m_next; };

    struct Metrics
    {
        /// The current number of chunks currently allocated
        std::atomic<size_t> m_usedChunks{0};

        /// The high water mark of used chunks
        std::atomic<size_t> m_usedChunksHighWaterMark{0};

        /// The number of chunks in the allocator
        std::atomic<size_t> m_chunks{0};

        /// The high water mark of chunks in the allocator
        std::atomic<size_t> m_chunksHighWaterMark{0};
    };

    /// @return A chunk from the allocator that has a size of chunkSize
    void* allocate(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA);

    /// @return An object allocated from a chunk given by the allocator. Null
    /// pointer if 'T' is incompatible with the chunks (wrong alignment/larger
    /// than the chunk size) or if allocating memory failed.
    template <typename T, typename... Args>
    T* newInternal(JACK_CALL_SITE_ARGS Args &&...args)
    {
        JACK_ASSERT_MSG(m_chunkSize >= sizeof(T),
                        "The object to allocate is larger than the chunk size of the allocator");

        JACK_ASSERT_MSG(m_alignment >= alignof(T),
                        "The object to allocate has alignment larger than the alignment of the allocator");

        T* result = nullptr;
        if (m_chunkSize >= sizeof(T) && m_alignment >= alignof(T)) {
            void* raw = allocate(JACK_CALL_SITE_INPUT_NO_TAIL_COMMA);
            result    = raw ? new (raw) T(std::forward<Args>(args)...) : nullptr;
        }

        return result;
    }

    /// Return a chunk back to the allocator, all references to this pointer are
    /// now invalidated.
    void give(JACK_CALL_SITE_ARGS void* ptr, bool clearMemory);

    /// Free all the chunks in the allocator, all references to the chunks are
    /// now invalidated.
    void freeAllocator(bool clearMemory);

    /// The backing allocator of all the chunks in this allocator
    ArenaAllocator m_arena = {};

    /// Make allocations from the arena thread safe
    /// \note Currently some tests run in a thread to update an agent's
    /// beliefset which will access the arena from multiple threads.
    mutable std::recursive_mutex m_mutex;

    /// The vacant chunks availiable to be handed out by the allocator
    std::atomic<FreeList*> m_freeList{nullptr};

    /// The usage metrics of the allocator
    Metrics m_metrics = {};

    /// The alignment value in which chunks will be aligned to
    /// This value must be set once before the allocator is used by using the
    /// constructor or after default initialisation.
    uint8_t m_alignment = 8;

    /// The size of each chunk allocated from the allocator
    /// This value must be set once before the allocator is used by using the
    /// constructor or after default initialisation.
    uint16_t m_chunkSize = JACK_KILOBYTES(4);
};

/// \note Boilerplate template was taken from
/// https://howardhinnant.github.io/allocator_boilerplate.html
/// Use this in STL containers like,
/*
   ArenaAllocator arena = {};
   std::vector<int, StdArenaAllocator<int>> foo(StdArenaAllocator<int>(&arena));
*/
template <class T>
class StdArenaAllocator
{
public:
    using value_type = T;

    StdArenaAllocator(ArenaAllocator& arena)
        : m_arena(arena) {}

    template <class U>
    StdArenaAllocator(const StdArenaAllocator<U>& other) : m_arena(other.m_arena) { }

    T* allocate(std::size_t n) {
        return static_cast<T*>(m_arena.allocate(JACK_CALL_SITE n * sizeof(T), alignof(T)));
    }

    void deallocate(T*, std::size_t) noexcept {
        /// \note Arena's don't deallocate individual objects, they either
        /// allocate objects or free the entire backing memory block instead.
    }

    ArenaAllocator& m_arena;
};

template <class T, class U>
bool operator==(const StdArenaAllocator<T>&, const StdArenaAllocator<U>&) noexcept
{
    return true;
}

template <class T, class U>
bool operator!=(const StdArenaAllocator<T>& x, const StdArenaAllocator<U>& y) noexcept
{
    return !(x == y);
}

/******************************************************************************
 * Virtual Memory
 * Wrappers around OS's virtual memory API for Windows and Linux.
 ******************************************************************************/
/// Reserve a range in the process's address space and optionally commit the
/// addresss space to some pages.
/// @param size The size in bytes to reserve a range for
/// @param commit Request the address space to be committed (i.e. backed by some pages)
/// @return A pointer to the reserved address space, null pointer if the OS was
/// unable to do so.
void *virtualReserve(size_t size, bool commit);

/// Request the OS to commit the range specified by the pointer to pages. The
/// pointer and size must specify a range previously reserved.
bool virtualCommit(void* ptr, size_t size);

/// Return the pages backing the pointer to the OS.
void virtualDecommit(void* ptr, size_t size);

/// Return the pages and release the address space specified by the given
/// pointer and size.
void virtualRelease(void* ptr, size_t size);

extern HeapAllocator globalHeapAllocator; /// The default global heap allocator

/******************************************************************************
 * @brief A fixed-size dynamically allocated array that cannot be resized. The
 * container is intended for use with arena style allocations (batch allocate
 * and batch free).
 *
 * This method of memory management simplifies the API usage of the container as
 * details about freeing memory or ownership is not a part of the API for this
 * container. By using an arena, the lifetime and onus of allocating and freeing
 * memory is tied to the lifetime of the arena not the container itself.
 *
 * @code
 * @endcode
 ******************************************************************************/
template <typename T>
class StaticArray
{
public:
    StaticArray() = default;

    StaticArray(ArenaAllocator& arena, size_t capacity)
    : m_data(arena.newArrayInternal<T>(JACK_CALL_SITE capacity))
    , m_size(0)
    , m_capacity(capacity)
    {
    }

    StaticArray(ArenaAllocator& arena, const std::vector<T>& other)
    : m_data(arena.newArrayInternal<T>(JACK_CALL_SITE other.size()))
    , m_size(0)
    , m_capacity(0)
    {
        if (!m_data) {
            return;
        }
        std::memcpy(m_data, other.data(), other.size() * sizeof(T));
        m_size     = other.size();
        m_capacity = other.size();
    }


    template <typename ...Args>
    T*       emplace_back(Args&&... args);
    T*       push_back(const T& item);
    T*       erase(T* it);
    void     clear() { m_size = 0; }

    T*       data()        { return m_data; }
    const T* data()  const { return m_data; }
    T*       begin()       { return m_data; }
    const T* begin() const { return m_data; }
    T*       end()         { return m_data + m_size; }
    const T* end()   const { return m_data + m_size; }
    size_t   size()  const { return m_size; }
    bool     empty() const { return m_size == 0; }

    T*     m_data;
    size_t m_size;
    size_t m_capacity;
};

template <typename T>
template <typename... Args>
T* StaticArray<T>::emplace_back(Args&&... args)
{
    T* result = nullptr;
    if (m_size < m_capacity) {
        m_data[m_size++] = T{std::forward<Args>(args)...};
        result           = &m_data[m_size - 1];
    }
    return result;
}

template <typename T>
T* StaticArray<T>::push_back(const T& item)
{
    T* result = nullptr;
    if (m_size < m_capacity) {
        m_data[m_size++] = item;
        result           = &m_data[m_size - 1];
    }
    return result;
}

template <typename T>
T* StaticArray<T>::erase(T* item)
{
    T*       result  = end();
    const T* beginIt = begin();
    const T* endIt   = result;

    if (!JACK_CHECK(item >= beginIt && item <= endIt)) {
        return result;
    }

    if (item == endIt) {
        return result;
    }

    size_t   eraseIndex  = (endIt - item) - 1;
    T*       dest        = m_data + eraseIndex;
    const T* src         = dest + 1;
    size_t   itemsToCopy = endIt - src;
    std::memmove(dest, src, itemsToCopy * sizeof(T));

    result = dest;
    m_size -= 1;
    return result;
}

/******************************************************************************
 * @brief A non-allocating slice/view that spans a contiguous list of objects.
 *
 * This data structure can be initialised from an implicit `array` in
 * C++ to avoid a heap allocation for functions that need a quick-n-dirty list
 * of pre-existing objects to operate on.
 *
 * @code
 * void Function(const Span<int>& list);
 * Function(std::array{1, 2, 3}); /// This is valid
 * @endcode
 ******************************************************************************/
template <typename T>
class Span
{
public:
    Span() = default;
    Span(const T* data, size_t size)    : m_data(const_cast<T*>(data)),         m_size(size)         {}
    Span(const std::vector<T>& array)   : m_data(const_cast<T*>(array.data())), m_size(array.size()) {}
    Span(const StaticArray<T>& array)   : m_data(const_cast<T*>(array.data())), m_size(array.size()) {}

    template <size_t N>
    Span(const T (&array)[N])           : m_data(const_cast<T*>(array)),        m_size(N)            {}

    template <size_t N>
    Span(const std::array<T, N>& array) : m_data(const_cast<T*>(array.data())), m_size(array.size()) {}

    /// Allocate a span using the passed in arena with the requested size. The
    /// span's validity is bound to the lifetime of the arena.
    static Span<T> alloc(ArenaAllocator& arena, size_t size);

    /// Deep copy the span into another span
    static Span<T> allocCopy(ArenaAllocator& arena, const Span<T>& other);

    T*       data()        { return m_data; }
    const T* data()  const { return m_data; }
    T*       begin()       { return m_data; }
    const T* begin() const { return m_data; }
    T*       end()         { return m_data + m_size; }
    const T* end()   const { return m_data + m_size; }
    size_t   size()  const { return m_size; }
    bool     empty() const { return m_size == 0; }

    const T& operator[](size_t index) const { JACK_ASSERT(m_size && index < m_size); return m_data[index]; }
    T&       operator[](size_t index)       { JACK_ASSERT(m_size && index < m_size); return m_data[index]; }

    T*     m_data;
    size_t m_size;
};

/******************************************************************************
 * @brief Fixed sized strings that are stack allocated with the size specified
 * in the template.
 *
 * The string is always null-terminated even if appending a string longer than
 * the capacity of the string. The resulting fixed string will be truncated with
 * a null-terminating byte.
 ******************************************************************************/
#define JACK_FSTRING(N, fmtLiteral, ...) aos::jack::FixedString<N>(FMT_STRING(fmtLiteral), ## __VA_ARGS__)
template <size_t N>
class FixedString
{
public:
    FixedString();

    template <typename... T>
    FixedString(fmt::format_string<T...> fmt, const T&... args);

    template <typename... T>
    bool             append(fmt::format_string<T...> fmt, const T&... args);
    char*            data()        { return m_data; }
    const char*      data()  const { return m_data; }
    char*            begin()       { return m_data; }
    const char*      begin() const { return m_data; }
    char*            end()         { return m_data + m_size; }
    const char*      end()   const { return m_data + m_size; }
    std::string_view view()  const { return std::string_view(m_data, m_size); }
    size_t           size()  const { return m_size; }
    bool             empty() const { return m_size == 0; }

    char   m_data[N + 1];
    size_t m_size;
};

template <size_t N>
FixedString<N>::FixedString()
    : m_size(0)
{
    m_data[0] = 0;
}

template <size_t N>
template <typename... T>
FixedString<N>::FixedString(fmt::format_string<T...> fmt, const T&... args)
: m_size(0)
{
    m_data[0] = 0;
    append(fmt, args...);
}

template <size_t N>
template <typename... T>
bool FixedString<N>::append(fmt::format_string<T...> fmt, const T&... args)
{
    size_t requiredSize = fmt::formatted_size(fmt, args...);
    size_t space        = N - m_size;
    if (!JACK_CHECK(space >= requiredSize)) {
        return false;
    }

    fmt::format_to(m_data + m_size, fmt, args...);
    m_size         += requiredSize;
    m_data[m_size] = 0;
    return true;
}

template <size_t N>
std::string_view format_as(const FixedString<N>& value)
{
    std::string_view result = value.view();
    return result;
}

/******************************************************************************
 * @brief Builder to construct variable length strings backed by a memory
 * arena.
 *
 * Strings are pushed onto a linked-list allocated by an arena. Building the
 * string involves traversing the linked-list to generate the string. This
 * allows dynamic strings to be allocated and freed efficiently by bumping a
 * base pointer avoiding individual allocations for each append (e.g.
 * std::string, std::stringstream)
 ******************************************************************************/
class StringBuilder
{
public:
    struct Link
    {
        std::string_view string;
        Link*            next;
    };

    StringBuilder(ArenaAllocator& arena) : m_arena(arena) { }

    template <typename... T>
    bool             append     (fmt::format_string<T...> fmt, const T&... args);

    template <typename... T>
    bool             appendV    (fmt::format_string<T...> fmt, fmt::format_args args);

    /// Store the string as-is into the builder. The memory backing the string
    /// must outlive the lifetime of the builder.
    bool             appendRef  (std::string_view string);

    /// Copy the string using the builder's allocator before adding it to the
    /// builder
    bool             appendCopy (std::string_view string);

    /// Construct the string whose lifetime is bound to the passed in arena.
    std::string_view toStringArena(ArenaAllocator& arena) const;

    /// Construct the string whose lifetime is bound to the RAII std::string object
    std::string      toString() const;

    ArenaAllocator& m_arena;                ///< Allocator to use to back the string list
    Link*           m_head       = nullptr; ///< First string in the linked list of strings
    Link*           m_tail       = nullptr; ///< Last string in the linked list of strings
    size_t          m_stringSize = 0;       ///< The size in bytes necessary to construct the current string
    size_t          m_count      = 0;       ///< The number of links in the linked list of strings
};

/******************************************************************************
 * Template implementation
 ******************************************************************************/
template <typename... T>
void logFmt(log::Severity level,
            std::string_view channel,
            int line,
            std::string_view file,
            fmt::format_string<T...> fmt,
            T &&...args)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    std::string_view log           = fmtAllocInternal(scratch.arena, fmt, args...);
    aos::jack::log(level, log, channel, line, file);
}

template <typename T>
Span<T> Span<T>::alloc(ArenaAllocator& arena, size_t size)
{
    Span<T> result = {};
    result.m_data  = JACK_ALLOCATOR_ALLOC_ARRAY(&arena, T, size);
    if (result.m_data) {
        result.m_size = size;
    }
    return result;
}

template <typename T>
Span<T> Span<T>::allocCopy(ArenaAllocator& arena, const Span<T>& other)
{
    Span<T> result = Span<T>::alloc(arena, other.size());
    std::memcpy(result.m_data, other.data(), sizeof(T) * result.m_size);
    return result;
}

template <typename... T>
std::string_view fmtAllocInternal(ArenaAllocator& arena, fmt::format_string<T...> fmt, const T&... args)
{
    std::string_view result       = {};
    size_t           requiredSize = fmt::formatted_size(fmt, args...);
    char*            buffer       = JACK_ALLOCATOR_ALLOC_ARRAY(&arena, char, requiredSize + 1);
    if (buffer) {
        fmt::format_to(buffer, fmt, args...);
        result               = std::string_view(buffer, requiredSize);
        /// \note Strings should be null-terminated because the C++ ecosystem
        /// still heavily interoperates with APIs that don't take length
        /// denoted strings.
        buffer[requiredSize] = 0;
    }
    return result;
}

template <typename... T>
bool StringBuilder::append(fmt::format_string<T...> fmt, const T&... args)
{
    bool result = appendV(fmt, fmt::make_format_args(args...));
    return result;
}

template <typename... T>
bool StringBuilder::appendV(fmt::format_string<T...> fmt, fmt::format_args args)
{
    class CountingIterator
    {
    public:
        using iterator_category [[maybe_unused]] = std::output_iterator_tag;
        using value_type        [[maybe_unused]] = char;
        using difference_type   [[maybe_unused]] = std::ptrdiff_t;
        using pointer           [[maybe_unused]] = char*;
        using reference         [[maybe_unused]] = char&;

        CountingIterator(std::size_t& count) : characterCount(count) {}
        CountingIterator(const CountingIterator& other) : characterCount(other.characterCount) {}
        CountingIterator& operator=(char) { ++characterCount; return *this; }
        CountingIterator& operator=(const CountingIterator& other) { characterCount = other.characterCount; return *this; }
        CountingIterator& operator*() { return *this; }
        CountingIterator& operator++(int) { return *this; }
    private:
        std::size_t& characterCount;
    };

    size_t requiredSize = 0;
    CountingIterator countingIt(requiredSize);
    fmt::vformat_to(countingIt, fmt, args);

    std::string_view string;
    char* buffer = JACK_ALLOCATOR_ALLOC_ARRAY(&m_arena, char, requiredSize + 1);
    if (buffer) {
        fmt::vformat_to(buffer, fmt, args);
        string = std::string_view(buffer, requiredSize);
        /// \note Strings should be null-terminated because the C++ ecosystem
        /// still heavily interoperates with APIs that don't take length
        /// denoted strings.
        buffer[requiredSize] = 0;
    }

    bool result = appendRef(string);
    return result;
}
} /// namespace aos::jack
#endif /// JACK_CORE_LIB_H
