#ifndef CIPELLS_FWD_BitSet_h_INCLUDED
#define CIPELLS_FWD_BitSet_h_INCLUDED

#include <cstdint>
#include <functional>

namespace cipells {


namespace detail {

using ChunkIndex = std::uint32_t;
using Chunk = std::uint8_t;
struct BitSetAccess;

} // namespace detail


class BitKey;
class BitSchema;

template <typename Derived> class BitSetBase;
template <typename Derived> class BitSetMutableBase;
class BitSet;
class SingleIndexBitSet;
template <typename E, typename F> class BitSetUnaryExpression;
template <typename L, typename R, typename F> class BitSetBinaryExpression;

template <typename E>
using BitSetNotExpression = BitSetUnaryExpression<E, std::bit_not<detail::Chunk>>;

template <typename L, typename R>
using BitSetOrExpression = BitSetBinaryExpression<L, R, std::bit_or<detail::Chunk>>;

template <typename L, typename R>
using BitSetAndExpression = BitSetBinaryExpression<L, R, std::bit_and<detail::Chunk>>;

template <typename L, typename R>
using BitSetXorExpression = BitSetBinaryExpression<L, R, std::bit_xor<detail::Chunk>>;

} // namespace cipells

#endif // !CIPELLS_FWD_BitSet_h_INCLUDED
