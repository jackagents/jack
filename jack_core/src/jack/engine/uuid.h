// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_UUID_H
#define JACK_UUID_H

#include <string> // for string, operator!=, operator<, operator==
#include <array>

namespace aos::jack
{
/*! ***********************************************************************************************
 * @class UniqueId
 *
 * A unique identifier used to identify JACK concepts
 * ************************************************************************************************/

class UniqueId
{
public:
    static const int JACK_UUID_SIZE = 16;

    static const int UUID_SIZE_IN_U64 = JACK_UUID_SIZE / sizeof(uint64_t);

    UniqueId() = default;

    /// Constructs an ID from a 128 bit number
    UniqueId(uint64_t hi, uint64_t lo);

    /// Copy constructs a UniqueId from another
    /// @param other The other UniqueId to copy
    UniqueId(const UniqueId& other) { *this = other; }

    /// Constructs a random id
    static UniqueId random();

    /// Seed the random id generator
    static void seed(uint64_t seed);

    /// Constructs an ID from a hex string. If the id string is not a valid
    /// UUID, the returned UniqueId will return false when validity is checked.
    static UniqueId initFromString(std::string_view idString);

    /// Copy operator
    /// @param other The UniqueId to copy from
    UniqueId& operator=(const UniqueId& other);

    /// Compare operator
    bool operator==(const UniqueId& other) const { return m_data == other.m_data; }

    /// Compare operator
    bool operator!=(const UniqueId& other) const { return !(m_data == other.m_data); }

    /// Compare operator
    bool operator<(const UniqueId& other) const { return m_data < other.m_data; }

    /// Return a C++ string hex representation of the UUID
    /// @return The id as a string
    std::string_view toString() const;

    /// Get the human readable tag
    const std::string& tag() const { return m_tag; }

    /// Set a human readable tag for the UUID
    void setTag(std::string&& tag) { m_tag = tag; }

    /// Return the underlying data representing the UUID
    const std::array<uint64_t, UUID_SIZE_IN_U64>& data() const { return m_data; }

    /// @return True if the UniqueId is set to a non-zero (non-null) UUID.
    bool valid() const { return m_data[0] != 0 || m_data[1] != 0; }

protected:

    /* ****************************************************************************************
    * Attributes
    * ****************************************************************************************/
    std::string                            m_tag; // Stores an optional human readable tag for the UUID
    std::array<uint64_t, UUID_SIZE_IN_U64> m_data;
    char                                   m_string[(JACK_UUID_SIZE * 2) + 1 /*null terminate*/];
};

std::string_view format_as(const UniqueId& value);
} // namespace aos::jack
#endif
