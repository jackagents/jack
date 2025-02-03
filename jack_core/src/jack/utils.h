// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_UTILS_H
#define JACK_UTILS_H

/// JACK
#include <jack/corelib.h>

/// Third Party
#include <fmt/core.h>
#include <stdarg.h> /// va_list
#include <stdio.h>  /// vsnprintf
#include <stdint.h> /// uint64_t
#include <stddef.h> /// ptrdiff_t

#include <vector>
#include <string>
#include <string_view>

namespace aos::jack
{
template<typename T>
struct Vec2
{
    Vec2() = default;
    Vec2(const Vec2 &other)      : m_x(other.m_x), m_y(other.m_y) {}
    Vec2(T x, T y) : m_x(x), m_y(y) {}
    Vec2(T x)            : m_x(x), m_y(x) {}
    Vec2(const struct V2i &other);

    T x() const { return m_x; }
    T y() const { return m_y; }
    
    Vec2   operator+ (const Vec2 &rhs) const { return {m_x + rhs.m_x, m_y + rhs.m_y}; }
    Vec2   operator- (const Vec2 &rhs) const { return {m_x - rhs.m_x, m_y - rhs.m_y}; }
    Vec2   operator* (const Vec2 &rhs) const { return {m_x * rhs.m_x, m_y * rhs.m_y}; }
    Vec2   operator* (T rhs)   const { return {m_x * rhs,     m_y * rhs};     }
    Vec2   operator/ (const Vec2 &rhs) const { return {m_x / rhs.m_x, m_y / rhs.m_y}; }
    Vec2   operator/ (T rhs)   const { return {m_x / rhs,     m_y / rhs};     }
    Vec2&  operator=(const Vec2& rhs)      { m_x = rhs.m_x; m_y = rhs.m_y; return *this; }
    Vec2&  operator+=(const Vec2 &rhs)       { m_x += rhs.m_x; m_y += rhs.m_y; return *this; }
    Vec2&  operator-=(const Vec2 &rhs)       { m_x -= rhs.m_x; m_y -= rhs.m_y; return *this; }
    Vec2&  operator*=(const Vec2 &rhs)       { m_x *= rhs.m_x; m_y *= rhs.m_y; return *this; }
    Vec2&  operator*=(T rhs)         { m_x *= rhs;     m_y *= rhs;     return *this; }
    Vec2&  operator/=(const Vec2 &rhs)       { m_x /= rhs.m_x; m_y /= rhs.m_y; return *this; }
    Vec2&  operator/=(T rhs)         { m_x /= rhs;     m_y /= rhs;     return *this; }
    bool operator==(const Vec2 &rhs) const { return (m_x == rhs.m_x) && (m_y == rhs.m_y); }
    bool operator!=(const Vec2 &rhs) const { return (m_x != rhs.m_x) || (m_y != rhs.m_y); }

    std::string toString() const;

    T m_x;
    T m_y;
};

using V2 = Vec2<float32>;

std::string format_as(const V2& val);
std::string format_as(Span<V2> val);

struct V2i
{
    V2i() = default;
    V2i(int32_t   x, int32_t   y)     : m_x(x), m_y(y) {}
    V2i(float32 x)            : m_x(static_cast<int32_t>(x)), m_y(static_cast<int32_t>(x)) {}
    V2i(const V2 &other)      : m_x(static_cast<int32_t>(other.x())), m_y(static_cast<int32_t>(other.y())) {}

    int32_t x() const { return m_x; }
    int32_t y() const { return m_y; }

    V2i  operator+ (const V2i &rhs) const { return    {m_x + rhs.m_x, m_y + rhs.m_y}; }
    V2i  operator- (const V2i &rhs) const { return    {m_x - rhs.m_x, m_y - rhs.m_y}; }
    V2i  operator* (const V2i &rhs) const { return    {m_x * rhs.m_x, m_y * rhs.m_y}; }
    V2i  operator* (int32_t    rhs) const { return    {m_x * rhs,     m_y * rhs};     }
    V2i  operator* (float32    rhs) const { return V2i{static_cast<int32_t>(m_x * rhs), static_cast<int32_t>(m_y * rhs)};     }
    V2i  operator/ (const V2i &rhs) const { return    {m_x / rhs.m_x, m_y / rhs.m_y}; }
    V2i  operator/ (int32_t    rhs) const { return    {m_x / rhs,     m_y / rhs};     }
    V2i  operator/ (float32    rhs) const { return V2i{static_cast<int32_t>(m_x / rhs), static_cast<int32_t>(m_y / rhs)};     }
    V2i& operator+=(const V2i &rhs)       { m_x += rhs.m_x; m_y += rhs.m_y; return *this; }
    V2i& operator-=(const V2i &rhs)       { m_x -= rhs.m_x; m_y -= rhs.m_y; return *this; }
    V2i& operator*=(const V2i &rhs)       { m_x *= rhs.m_x; m_y *= rhs.m_y; return *this; }
    V2i& operator*=(int32_t    rhs)       { m_x *= rhs;     m_y *= rhs;     return *this; }
    V2i& operator*=(float32    rhs)       { m_x = static_cast<int32_t>(m_x * rhs);  m_y = static_cast<int32_t>(m_y * rhs); return *this; }
    V2i& operator/=(const V2i &rhs)       { m_x /= rhs.m_x; m_y /= rhs.m_y; return *this; }
    V2i& operator/=(int32_t    rhs)       { m_x /= rhs;     m_y /= rhs;     return *this; }
    V2i& operator/=(float32    rhs)       { m_x = static_cast<int32_t>(m_x / rhs);  m_y = static_cast<int32_t>(m_y / rhs); return *this; }
    bool operator==(const V2i &rhs) const { return (m_x == rhs.m_x) && (m_y == rhs.m_y); }
    bool operator!=(const V2i &rhs) const { return (m_x != rhs.m_x) || (m_y != rhs.m_y); }

    std::string toString() const;

    int32_t m_x;
    int32_t m_y;
};

template<typename T>
Vec2<T>::Vec2(const struct V2i &other)
    : m_x(static_cast<T>(other.m_x)), m_y(static_cast<T>(other.m_y))
{
}

template<typename T>
std::string Vec2<T>::toString() const
{
    std::string result = JACK_FMT("{{x={}, y={}}}", m_x, m_y);
    return result;
}

inline V2  operator*(float32 lhs, const V2&  rhs) { return rhs * lhs; }
inline V2i operator*(int   lhs,   const V2i& rhs) { return rhs * lhs; }
inline V2i operator*(float32 lhs, const V2i& rhs) { return rhs * lhs; }

template <typename T, size_t N>
constexpr size_t arrayCountUSize(T (&)[N]) { return N; }

template <typename T, ptrdiff_t N>
constexpr ptrdiff_t arrayCountISize(T (&)[N]) { return N; }

/// Defer Macro
/*
   Helper to construct lambdas that execute at the end of the scope, i.e.

   {
       DEFER { printf("Good Evening\n"); };
       printf("Good Morning\n");

       // Good Morning
       // Good Evening
   }
 */
template <typename Proc>
struct Defer
{
    Defer(Proc proc) : proc(proc) {}
    ~Defer() { proc(); }
    Proc proc;
};

struct DeferHelper
{
    template <typename Lambda>
    Defer<Lambda> operator+(Lambda lambda) { return Defer<Lambda>(lambda); }
};

#define JACK_TOKEN_COMBINE2(x, y) x ## y
#define JACK_TOKEN_COMBINE(x, y) JACK_TOKEN_COMBINE2(x, y)
#define JACK_UNIQUE_NAME(prefix) JACK_TOKEN_COMBINE(prefix, __LINE__)
#define JACK_DEFER const auto JACK_UNIQUE_NAME(defer_lambda_) = aos::jack::DeferHelper() + [&]()

/// Convert a byte count into a standardised unit of digital size (bytes, KiB,
/// MiB, GiB) string. For example, 1024 is turned into "1.0 KiB"
/// @param buffer The buffer to write into. Pass a null pointer to calculate the
/// size of the buffer required to convert 'numBytes'.
/// @param bufferSize The size of the buffer passed in
/// @param numBytes The number of bytes to convert into a string
/// @return The size of the newly constructed string not including the
/// null-terminator.
int readableByteSizeString(char *buffer, int bufferSize, size_t numBytes);

enum ReadableDurationFlags
{
    ReadableDurationFlags_MS   = 1 << 0,
    ReadableDurationFlags_SEC  = 1 << 1,
    ReadableDurationFlags_MIN  = 1 << 2,
    ReadableDurationFlags_HR   = 1 << 3,
    ReadableDurationFlags_DAYS = 1 << 4,
    ReadableDurationFlags_HMS  = ReadableDurationFlags_HR | ReadableDurationFlags_MIN | ReadableDurationFlags_SEC,
    ReadableDurationFlags_DHMS = ReadableDurationFlags_DAYS | ReadableDurationFlags_HMS,
    ReadableDurationFlags_ALL  = ReadableDurationFlags_DHMS | ReadableDurationFlags_MS,
};
/// Convert a duration in milliseconds to a human readable duration string. For
/// example (1000*60*60 /*1hr*/ + 1500 /*1.5s*/) is turned into "1hr 1s 500ms".
///
/// The output string can be limited to a specific set of denominated time units
/// by specifying the subset of output flags desired. Flags times are rounded
/// to the nearest integer where output flags are omitted and the duration
/// cannot be entirely represented in the desired units.
FixedString<64> readableDurationStringFromMs(uint64_t milliseconds, ReadableDurationFlags flags = ReadableDurationFlags_ALL);

class U64String
{
public:
    char*            data()       { return m_data; }
    const char*      data() const { return m_data; }
    uint8_t          size() const { return m_size; }
    std::string_view view() const { return std::string_view(m_data, m_size); }

    char    m_data[27]; /// \note 27 is the maximum size of uint64_t including commas
    uint8_t m_size;
};

/// Convert a u64 value to a string, optionally comma separated.
U64String u64ToString(uint64_t val, bool comma_sep);

/// Convert a long string, typically an ID into a short-hand version of it,
/// "1234567890" to "123...456" or similar. If the sum of the head, tail and
/// partition is smaller or equal to the length of the string, the string
/// returned is a copy of the original string.
///
/// @param head How many prefix characters to preserve at the start of the input
/// @param tail How many suffix characters to preserve at the end of the input
/// @param partitionSize The length of the partition inbetween the head and tail
/// @param partitionChar The character to insert between the head and tail
std::string compactString(std::string_view input,
                          unsigned         head = 3,
                          unsigned         tail = 3,
                          unsigned         partitionSize = 2,
                          char             partitionChar = '.');

/// Get the current executing application's directory
std::string getExecutableDirectory();

/// @return A short log line describing an allocator's metrics
FixedString<512> allocatorMetricsLog(const AllocatorMetrics &metrics);

/// @return A short log line describing an arena's metrics
FixedString<512> arenaMetricsLog(const ArenaAllocator::Metrics &metrics);

/// @return A short log line describing a chunk allocator's metrics
FixedString<512> chunkAllocatorMetricsLog(const ChunkAllocator &allocator);

/// Write the buffer entirely to the file
/// @return True if the file was written successfully
bool fsWriteEntireFile(std::string_view path, const void* buffer, size_t size);

struct FSReadEntireFileResult
{
    bool success;
    std::vector<char> buffer;
};

FSReadEntireFileResult fsReadEntireFile(std::string_view path);

template <typename T>
std::string toStringArray(Span<T> array)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);

    for (size_t index = 0; index < array.size(); index++) {
       const T& item = array[index];
       if (index) {
           builder.appendRef(", ");
       }
       builder.append(FMT_STRING("{}"), item);
    }
    builder.appendRef(", ");

    std::string result = builder.toString();
    return result;
}

template <typename T>
std::string_view toStringArrayArena(ArenaAllocator& arena, Span<T> array)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(&arena);
    StringBuilder builder          = StringBuilder(scratch.arena);

    for (size_t index = 0; index < array.size(); index++) {
       const T& item = array[index];
       if (index) {
           builder.appendRef(", ");
       }
       builder.append(FMT_STRING("{}"), item);
    }
    builder.appendRef(", ");

    std::string_view result = builder.toStringArena(arena);
    return result;
}
} // namespace aos::jack
#endif // JACK_UTILS_H
