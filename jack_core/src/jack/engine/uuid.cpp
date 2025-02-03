// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/engine/uuid.h>

#include <cassert>      // for assert
#include <cstring>      // for memcpy
#include <random>       // for mt19937, random_device, uniform_int_distribution
#include <string>       // for char_traits, allocator
#include <type_traits>  // for move
#include <utility>      // for max, min

namespace aos::jack
{
std::random_device                      g_rd;
std::mt19937                            g_gen(g_rd());
std::uniform_int_distribution<uint64_t> g_dis(0, std::numeric_limits<uint64_t>::max());

/// \todo Move this to a general utility file because the C/C++ standard library
/// sucks is there a function for this that supports > 64 bit numbers?
static bool hexToByte(char hex1, char hex2, unsigned char &value)
{
    auto BAD_HEX = std::numeric_limits<unsigned char>::max();
    unsigned char nibble1 =   (hex1 >= '0' && hex1 <= '9') ? (hex1 - '0')
                            : (hex1 >= 'a' && hex1 <= 'f') ? (10 + (hex1 - 'a'))
                            : (hex1 >= 'A' && hex1 <= 'F') ? (10 + (hex1 - 'A'))
                                                           : BAD_HEX;
    unsigned char nibble2 =   (hex2 >= '0' && hex2 <= '9') ? (hex2 - '0')
                            : (hex2 >= 'a' && hex2 <= 'f') ? (10 + (hex2 - 'a'))
                            : (hex2 >= 'A' && hex2 <= 'F') ? (10 + (hex2 - 'A'))
                                                           : BAD_HEX;

    if (nibble1 == BAD_HEX || nibble2 == BAD_HEX)
        return false;

    value = (nibble1 << 4) | (nibble2 << 0);
    return true;
}

static bool hexToU64(const char *string, int size, uint64_t &value)
{
    const char *hex      = string;
    int         hex_size = size;
    if (size >= 2 && (string[0] == '0' && (string[1] == 'x' || string[1] == 'X'))) {  // i.e. "0xABCD"
        hex += 2;
        hex_size -= 2;
    } else if (size >= 1 && (string[0] == 'x' || string[0] == 'X')) { // ie. "xABCD"
        hex += 1;
        hex_size -= 1;
    }

    if (hex_size > static_cast<int>((sizeof(uint64_t) * 2))) {
        return false; // Hex string is too long to deserialize into a U64
    }

    value = 0;
    for (int i = 0; i < hex_size; i += 2) {
        unsigned char asByte = 0;
        if (!hexToByte(hex[i + 0], hex[i + 1], asByte)) {
            return false;
        }

        value <<= 8;
        value |= asByte;
    }

    return true;
}

/// Seed the random id generator
void UniqueId::seed(uint64_t seed)
{
    g_gen.seed(static_cast<unsigned int>(seed));
}

/// a placeholder uuid generator
/// construct a unique identifier
UniqueId UniqueId::random()
{
    UniqueId result = UniqueId(g_dis(g_gen) /*hi*/, g_dis(g_gen) /*lo*/);
    return result;
}

UniqueId UniqueId::initFromString(std::string_view idString)
{
    const int UUID_SIZE_IN_HEX = UniqueId::JACK_UUID_SIZE * 2;
    if (idString.size() != UUID_SIZE_IN_HEX) {
        return UniqueId{};
    }

    /// UUID is 128 bits or 2 x u64
    const int   HALF_UUID_SIZE_IN_HEX = UUID_SIZE_IN_HEX / 2;
    const char *hiPtr                 = idString.data();
    const char *loPtr                 = idString.data() + HALF_UUID_SIZE_IN_HEX;

    uint64_t hi = 0, lo = 0;
    bool     converted  = hexToU64(hiPtr, HALF_UUID_SIZE_IN_HEX, hi);
             converted &= hexToU64(loPtr, HALF_UUID_SIZE_IN_HEX, lo);

    if (!converted) {
        return UniqueId{};
    }

    UniqueId result = UniqueId(hi, lo);
    return result;
}

UniqueId::UniqueId(uint64_t hi, uint64_t lo)
{
    m_data[0] = hi;
    m_data[1] = lo;

    /// \todo Pull in a real world uuid generator eventually
    char *dest = m_string;
    for (uint64_t chunk : m_data) {
        for (int rShift = 56; rShift >= 0; rShift -= 8) {
            unsigned char byte  = (chunk >> rShift) & 0xFF;
            char          hex01 = (byte >> 4) & 0xF;
            char          hex02 = (byte >> 0) & 0xF;
            *dest++             = hex01 <= 9 ? ('0' + hex01) : ('a' + (hex01 - 10));
            *dest++             = hex02 <= 9 ? ('0' + hex02) : ('a' + (hex02 - 10));
        }
    }
    *dest++ = 0; // null-terminate
}

/// copy from another UniqueId
UniqueId &UniqueId::operator=(const UniqueId &other)
{
    m_tag  = other.m_tag;
    m_data = other.m_data;
    std::memcpy(m_string, other.m_string, sizeof(m_string));
    return *this;
}

std::string_view UniqueId::toString() const
{
    std::string_view result(m_string, JACK_UUID_SIZE * 2);
    return result;
}

std::string_view format_as(const UniqueId& value)
{
    std::string_view result = value.toString();
    return result;
}
} // namespace aos::jack
