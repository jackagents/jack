#ifndef BITMASKOPS_H
#define BITMASKOPS_H

#include <type_traits>

/**
 * @brief a set or helper templates and MACRO to define bitwise operators on a enum
 * 
 * could use this instead: https://github.com/oliora/bitmask
 * 
 * @tparam Enum The enum that we want to have bitwise operators for
 * 
 * enum class Flags {
 *     Empty = 0x0,
 *      A  = 0x1,
 *      B  = 0x2,
 * };
 *
 * ENABLE_BITMASK_OPERATORS(Flags);
 * 
 * Flags f = A | B;
 * f = Empty;
 * f = A;
 * f == A // true  
 * f |= B;
 * f == A | B; // true
 */

template<typename Enum>
struct EnableBitMaskOperators  
{
    static const bool enable = false;
};

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator |(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
    );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator |=(Enum &lhs, Enum rhs)  
{
    using underlying = typename std::underlying_type<Enum>::type;
    lhs = static_cast<Enum> (
        static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
    );
    return lhs;
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator &(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        static_cast<underlying>(lhs) & static_cast<underlying>(rhs)
    );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator &=(Enum &lhs, Enum rhs)  
{
    using underlying = typename std::underlying_type<Enum>::type;
    lhs = static_cast<Enum> (
        static_cast<underlying>(lhs) & static_cast<underlying>(rhs)
    );
    return lhs;
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator ~(Enum lhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        ~static_cast<underlying>(lhs)
    );
}

#define ENABLE_BITMASK_OPERATORS(x)  \
template<>                           \
struct EnableBitMaskOperators<x>     \
{                                    \
    static const bool enable = true; \
};

#endif
