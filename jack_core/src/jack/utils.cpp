/// JACK
#include <jack/utils.h>

/// Third Party
#include <stdio.h>
#include <string.h>
#include <filesystem>

/// \note OS definitions for implementing getExecutableDirectory which C++17's
/// <filesystem> does not have.
#if defined(_WIN32) && !defined(_INC_WINDOWS)
extern "C" unsigned long __stdcall GetModuleFileNameA(void *hModule, char *lpFilename, unsigned long nSize);
#else
#include <errno.h>
#include <unistd.h> // readlink
#endif

namespace aos::jack
{



std::string format_as(const V2& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(Span<V2> val)
{
    std::string result = toStringArray(val);
    return result;
}

std::string V2i::toString() const
{
    std::string result = JACK_FMT("{{x={}, y={}}}", m_x, m_y);
    return result;
}

std::string format_as(const V2i& val)
{
    std::string result = val.toString();
    return result;
}

int readableByteSizeString(char *buffer, int bufferSize, size_t numBytes)
{
    enum ByteSize {
        ByteSize_BYTES,
        ByteSize_KILOBYTES,
        ByteSize_MEGABYTES,
        ByteSize_GIGABYTES,
        ByteSize_COUNT,
    };

    unsigned byteSize = ByteSize_BYTES;
    auto memSize      = static_cast<double>(numBytes);
    for (; byteSize < ByteSize_COUNT && memSize >= 1024.0; byteSize++) {
        memSize /= 1024.0;
    }

    const char *memLabel = nullptr;
    switch (byteSize) {
        case ByteSize_BYTES: memLabel = "B"; break;
        case ByteSize_KILOBYTES: memLabel = "KiB"; break;
        case ByteSize_MEGABYTES: memLabel = "MiB"; break;
        case ByteSize_GIGABYTES: memLabel = "GiB"; break;
        case ByteSize_COUNT: memLabel = "Bad Byte Size"; break;
    }

    int result = 0;
    if (memSize == 0) {
        result = snprintf(buffer, bufferSize, "0%s", memLabel);
    } else {
        result = snprintf(buffer, bufferSize, "%.2f%s", memSize, memLabel);
    }
    return result;
}

FixedString<64> readableDurationStringFromMs(uint64_t milliseconds, ReadableDurationFlags flags)
{
    const uint64_t SECONDS_IN_MS = 1000;
    const uint64_t MINUTES_IN_MS = SECONDS_IN_MS * 60;
    const uint64_t HOURS_IN_MS   = MINUTES_IN_MS * 60;
    const uint64_t DAY_IN_MS     = HOURS_IN_MS * 24;

    FixedString<64> result   = {};
    uint64_t        duration = milliseconds;
    if (duration > DAY_IN_MS && (flags & ReadableDurationFlags_DAYS)) {
        size_t days = duration / DAY_IN_MS;
        duration -= DAY_IN_MS * days;
        result.append("{}d", days);
    }

    if (duration > HOURS_IN_MS && (flags & ReadableDurationFlags_HR)) {
        size_t hours = duration / HOURS_IN_MS;
        duration -= HOURS_IN_MS * hours;
        result.append("{}{}hr", result.m_size ? " " : "", hours);
    }

    if (duration > MINUTES_IN_MS && (flags & ReadableDurationFlags_MIN)) {
        size_t minutes = duration / MINUTES_IN_MS;
        duration -= MINUTES_IN_MS * minutes;
        result.append("{}{}m", result.m_size ? " " : "", minutes);
    }

    if (duration > SECONDS_IN_MS && (flags & ReadableDurationFlags_SEC)) {
        size_t seconds = duration / SECONDS_IN_MS;
        duration -= SECONDS_IN_MS * seconds;
        result.append("{}{}s", result.m_size ? " " : "", seconds);
    }

    if (flags & ReadableDurationFlags_MS) {
        result.append("{}{}ms", result.m_size ? " " : "", duration);
    }

    return result;
}

U64String u64ToString(uint64_t val, bool commaSep)
{
    U64String result = {};
    if (val == 0) {
        result.m_data[result.m_size++] = '0';
    } else {
        char buf[sizeof(result.m_data)] = {};
        uint8_t buf_index               = (sizeof(buf) - 1);
        uint8_t buf_size                = 0;
        buf[buf_index--]                = 0;

        for (int digit_count = 0; val > 0; buf_size++, digit_count++) {
            if (commaSep && (digit_count != 0) && (digit_count % 3 == 0)) {
                buf[buf_index--] = ',';
                buf_size++;
            }

            auto digit       = (char)(val % 10);
            buf[buf_index--] = '0' + digit;
            val /= 10;
        }

        result.m_size = buf_size;
        memcpy(result.m_data, buf + buf_index + 1, result.m_size);
    }

    return result;
}

std::string compactString(std::string_view input,
                          unsigned         head,
                          unsigned         tail,
                          unsigned         partitionSize,
                          char             partitionChar)
{
    std::string result;
    size_t minLength = head + tail + partitionSize;
    if (input.size() > minLength) {
       result.append(input.data(), head);
       result.append(partitionSize, partitionChar);
       result.append(input.data() + input.size() - tail, tail);
    } else {
       result = input;
    }

    return result;
}

std::string getExecutableDirectory()
{
    /// \todo Every FS has quirks on the largest size, we'll probably need to
    /// switch to some heap allocation style for a robust implementation.
    char buffer[4096];

#if defined(_WIN32)
    int directorySize = GetModuleFileNameA(nullptr /*module*/, buffer, sizeof(buffer));
#else
    int directorySize = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (directorySize == -1) {
        /// Failed, we're unable to determine the executable directory
    } else if (directorySize == sizeof(buffer)) {
        /// \todo Make this more robust, we should just keep allocating
        /// a temporary buffer until we have enough
        /// Try again, if returned size was equal- we may of prematurely
        /// truncated according to the man pages
    } else {
        /// readlink will give us the path to the executable. Once we
        /// determine the correct buffer size required to get the full file
        /// path, we do some post-processing on said string and extract just
        /// the directory.
    }
#endif

    for (int index = directorySize - 1; index >= 0; index--, directorySize--) {
        if (buffer[index] == '\\' || buffer[index] == '/') {
            buffer[index] = 0; // Null-terminate
            directorySize--;
            break;
        }
    }

    std::string result(buffer, directorySize);
    return result;
}

FixedString<512> allocatorMetricsLog(const AllocatorMetrics &metrics)
{
    char allocs[32];
    char allocsHWM[32];
    char lifetimeAllocs[32];

    readableByteSizeString(allocs, sizeof(allocs), metrics.bytesAllocated);
    readableByteSizeString(allocsHWM, sizeof(allocsHWM), metrics.bytesAllocatedHighWaterMark);
    readableByteSizeString(lifetimeAllocs, sizeof(lifetimeAllocs), metrics.totalBytesAllocated);

    U64String allocsCount         = u64ToString(metrics.allocations, true /*comma_sep*/);
    U64String allocsHWMCount      = u64ToString(metrics.allocationsHighWaterMark, true /*comma_sep*/);
    U64String lifetimeAllocsCount = u64ToString(metrics.totalAllocations, true /*comma_sep*/);

    auto result = JACK_FSTRING(512, "Alloc{{allocs/HWM={}({})/{}({}), life={}({})}}",
                               allocs,         allocsCount.data(),
                               allocsHWM,      allocsHWMCount.data(),
                               lifetimeAllocs, lifetimeAllocsCount.data());
    return result;
}

FixedString<512> arenaMetricsLog(const ArenaAllocator::Metrics &metrics)
{
    char capacity[32];
    char capacityHWM[32];
    char wasted[32];
    char wastedHWM[32];
    char commit[32];

    readableByteSizeString(capacity, sizeof(capacity), metrics.capacity);
    readableByteSizeString(capacityHWM, sizeof(capacityHWM), metrics.capacityHighWaterMark);
    readableByteSizeString(wasted, sizeof(wasted), metrics.wastedSpace);
    readableByteSizeString(wastedHWM, sizeof(wastedHWM), metrics.wastedSpaceHighWaterMark);
    readableByteSizeString(commit, sizeof(commit), metrics.commitSize);

    FixedString<512> allocatorMetrics = allocatorMetricsLog(metrics);
    auto result = JACK_FSTRING(512, "Arena{{{}, cap/HWM={}/{}, waste/HWM={}/{}, blks/HWM={}/{}, commit={}, sysCalls={}}}",
                               allocatorMetrics,
                               capacity, capacityHWM,
                               wasted,   wastedHWM,
                               metrics.blocks, metrics.blocksHighWaterMark,
                               commit,
                               metrics.sysCalls);
    return result;
}

FixedString<512> chunkAllocatorMetricsLog(const ChunkAllocator &allocator)
{
    FixedString<512> arena = arenaMetricsLog(allocator.m_arena.metrics());
    auto result = JACK_FSTRING(512, "ChunkAlloc{{used/HWM={}/{}, chunks/HWM={}/{}, arena={}}}",
                            allocator.m_metrics.m_usedChunks.load(),
                            allocator.m_metrics.m_usedChunksHighWaterMark.load(),
                            allocator.m_metrics.m_chunks.load(),
                            allocator.m_metrics.m_chunksHighWaterMark.load(),
                            arena);

    return result;
}


bool fsWriteEntireFile(std::string_view path, const void* buffer, size_t size)
{
    bool  result = false;
    FILE* handle = fopen(path.data(), "w+");
    if (handle) {
        result = fwrite(buffer, size, 1, handle) == 1;
        fclose(handle);
    }

    return result;
}

FSReadEntireFileResult fsReadEntireFile(std::string_view path)
{
    FSReadEntireFileResult result = {};
    FILE* handle                  = fopen(path.data(), "r+b");
    if (handle) {
        size_t fileSize = std::filesystem::file_size(path);
        result.buffer.resize(fileSize);
        result.success = fread(result.buffer.data(), result.buffer.size(), 1, handle) == 1;
        fclose(handle);
    }

    if (!result.success) {
        result.buffer = {};
    }
    return result;
}

std::string format_as(aos::jack::Span<uint8_t> val)
{
    std::string result = aos::jack::toStringArray<uint8_t>(val);
    return result;
}

std::string format_as(aos::jack::Span<uint16_t> val)
{
    std::string result = aos::jack::toStringArray<uint16_t>(val);
    return result;
}

std::string format_as(aos::jack::Span<uint32_t> val)
{
    std::string result = aos::jack::toStringArray<uint32_t>(val);
    return result;
}

std::string format_as(aos::jack::Span<uint64_t> val)
{
    std::string result = aos::jack::toStringArray<uint64_t>(val);
    return result;
}

std::string format_as(aos::jack::Span<int8_t> val)
{
    std::string result = aos::jack::toStringArray<int8_t>(val);
    return result;
}
} // namespace aos::jack
