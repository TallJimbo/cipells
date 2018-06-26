#ifndef CIPELLS_BitSet_h_INCLUDED
#define CIPELLS_BitSet_h_INCLUDED

#include <vector>
#include <string>
#include <memory>
#include "cipells/fwd/BitSet.h"

namespace cipells {


class BitKey {
public:

    BitKey();

    bool operator==(BitKey const & rhs) const;
    bool operator!=(BitKey const & rhs) const { return !(*this == rhs); }

    bool operator<(BitKey const & rhs) const;
    bool operator<=(BitKey const & rhs) const;

    bool operator>(BitKey const & rhs) const { return !(*this <= rhs); }
    bool operator>=(BitKey const & rhs) const { return !(*this < rhs); }

private:

    friend class detail::BitSetAccess;
    friend class SingleIndexBitSet;

    BitKey(detail::Chunk mask, detail::ChunkIndex index, detail::ChunkIndex n_chunks);

    detail::Chunk _mask;
    detail::ChunkIndex _index;
};


namespace detail {

constexpr ChunkIndex BITS_PER_CHUNK = sizeof(Chunk)*8u;
constexpr Chunk TOP_CHUNK_MASK = Chunk(1u) << (BITS_PER_CHUNK - 1u);
constexpr Chunk FULL_CHUNK_MASK = Chunk(0xFF);

struct BitSetAccess {

    static Chunk upper_chunk_mask(std::size_t nbits);

    static ChunkIndex count_chunks(std::size_t nbits);

    static BitKey next(BitKey const & current);

    template <typename Derived>
    static ChunkIndex n_chunks(BitSetBase<Derived> const & bs) {
        return static_cast<Derived const &>(bs)._n_chunks();
    }

    template <typename Derived>
    static Chunk get_chunk(BitSetBase<Derived> const & bs, ChunkIndex i) {
        return static_cast<Derived const &>(bs)._get_chunk(i);
    }

    template <typename Derived>
    static Chunk & get_chunk_ref(BitSetMutableBase<Derived> & bs, ChunkIndex i) {
        return static_cast<Derived &>(bs)._get_chunk_ref(i);
    }

    template <typename L, typename R, typename F>
    static L & assign(BitSetMutableBase<L> & lhs, BitSetBase<R> const & rhs, F func) {
        assert(lhs.size() == rhs.size());
        ChunkIndex const n = n_chunks(lhs);
        for (ChunkIndex i = 0; i < n; ++i) {
            func(get_chunk_ref(lhs, i), get_chunk(rhs, i));
        }
        return static_cast<L &>(lhs);
    }

    template <typename Derived>
    static bool get_bit(BitSetBase<Derived> const & bs, BitKey i) {
        return get_chunk(bs, i._index) & i._mask;
    }

};

} // namespace detail


class BitSchema {
public:

    struct Item {
        std::string name;
        std::string description;
        BitKey key;
    };

    using const_iterator = std::vector<Item>::const_iterator;

    BitSchema(std::size_t max_size);

    const_iterator begin() const { return _items.begin(); }
    const_iterator end() const { return _items.end(); }

    std::size_t size() const { return _items.size(); }
    std::size_t max_size() const;
    bool empty() const { return _items.empty(); }

    bool operator==(BitSchema const & rhs) const;
    bool operator!=(BitSchema const & rhs) const { return !(*this == rhs); }

    // Retrieve iterator, returning end() when not found.
    const_iterator find(std::string const & name) const;
    const_iterator find(BitKey const & name) const;

    // Retrieve item, throwing std::out_of_range when not found.
    Item const & operator[](std::string const & name) const;
    Item const & operator[](BitKey const & index) const;

    BitSchema copy(std::size_t max_size=0) const;

    BitSchema compressed() const { return copy(this->size()); }

    BitKey const & append(std::string const & name, std::string const & description);

    // TODO: extend() -> BitTranslator

private:

    friend class detail::BitSetAccess;

    std::vector<Item> _items;
    detail::ChunkIndex const _n_chunks;
};


template <typename Derived>
class BitSetBase {
public:

    bool operator[](BitKey const & index) const {
        return detail::BitSetAccess::get_bit(*this, index);
    }

    bool any() const {
        ChunkIndex const n = n_chunks(lhs) - 1;
        for (ChunkIndex i = 0; i < n; ++i) {
            if (detail::BitSetAccess::get_chunk(i)) {
                return true;
            }
        }
        if (detail::BitSetAccess::get_chunk(i) & detail::BitSetAccess::upper_chunk_mask(_size)) {
            return true;
        }
        return false;
    }

    bool all() const {
        ChunkIndex const n = n_chunks(lhs) - 1;
        for (ChunkIndex i = 0; i < n; ++i) {
            if (detail::BitSetAccess::get_chunk(i) != detail::FULL_CHUNK_MASK) {
                return false;
            }
        }
        if (detail::BitSetAccess::get_chunk(i) != detail::BitSetAccess::upper_chunk_mask(_size)) {
            return false;
        }
        return true;
    }

    bool none() const { return !any(); }

    std::size_t size() const { return static_cast<Derived const &>(*this); }

    std::size_t max_size() const { return detail::BitSetAccess::n_chunks(*this)*BITS_PER_CHUNK; }

    BitSetNotExpression<Derived> operator~() const {
        return BitSetNotExpression<Derived>(*this);
    }

    template <typename R>
    BitSetOrExpression<Derived, R> operator|(BitSetBase<R> const & rhs) const {
        return BitSetOrExpression<Derived, R>(*this, rhs);
    }

    template <typename R>
    BitSetAndExpression<Derived, R> operator&(BitSetBase<R> const & rhs) const {
        return BitSetAndExpression<Derived, R>(*this, rhs);
    }

    template <typename R>
    BitSetXorExpression<Derived, R> operator^(BitSetBase<R> const & rhs) const {
        BitSetXorExpression<Derived, R>(*this, rhs);
    }

};


template <typename Derived>
class BitSetMutableBase :  public BitSetBase<Derived> {
public:

    template <typename E>
    Derived & operator=(BitSetBase<E> const & rhs) {
        return detail::BitSetAccess::assign(*this, rhs, [](detail::Chunk & a, detail::Chunk b) { a = b; });
    }

    template <typename E>
    Derived & operator|=(BitSetBase<E> const & rhs) {
        return detail::BitSetAccess::assign(*this, rhs, [](detail::Chunk & a, detail::Chunk b) { a |= b; });
    }

    template <typename E>
    Derived & operator&=(BitSetBase<E> const & rhs) {
        return detail::BitSetAccess::assign(*this, rhs, [](detail::Chunk & a, detail::Chunk b) { a &= b; });
    }

    template <typename E>
    Derived & operator^=(BitSetBase<E> const & rhs) {
        return detail::BitSetAccess::assign(*this, rhs, [](detail::Chunk & a, detail::Chunk b) { a ^= b; });
    }

};


template <typename E, typename F>
class BitSetUnaryExpression : public BitSetBase<BitSetUnaryExpression<E, F>>, private F {
public:

    explicit BitSetUnaryExpression(E arg, F func=F()) : F(func), _arg(std::move(arg)) {}

    std::size_t size() const { return _arg.size(); }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const {
        return F::operator()(detail::BitSetAccess::get_chunk(_arg, i));
    }

    detail::ChunkSize _n_chunks() const {
        return detail::BitSetAccess::n_chunks(_arg);
    }

    E _arg;
};


template <typename L, typename R, typename F>
class BitSetBinaryExpression : public BitSetBase<BitSetBinaryExpression<L, R, F>>, private F {
public:

    BitSetBinaryExpression(L lhs, R rhs, F func=F()) : F(func), _lhs(std::move(lhs)), _rhs(std::move(rhs)) {
        assert(_lhs.size() == _rhs.size());
    }

    std::size_t size() const { return _lhs.size(); }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const {
        return F::operator()(detail::BitSetAccess::get_chunk(_lhs, i),
                             detail::BitSetAccess::get_chunk(_rhs, i));
    }

    detail::ChunkSize _n_chunks() const {
        return detail::BitSetAccess::n_chunks(_lhs);
    }

    L _lhs;
    R _rhs;
};


class SingleIndexBitSet : public BitSetBase<SingleIndexBitSet> {
public:

    explicit SingleIndexBitSet(BitKey const & key, detail::ChunkSize n_chunks, std::size_t size);

    std::size_t size() const { return _size; }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const {
        return (i == _key._index) ? _key._mask : detail::Chunk(0u);
    }

    detail::ChunkSize _n_chunks() const { return _chunk_count; }

    BitKey _key;
    detail::ChunkSize _chunk_count;
    std::size_t _size;
};


class BitSet : public BitSetMutableBase<BitSet> {
public:

    explicit BitSet(std::size_t max_size);

    template <typename Derived>
    BitSet(BitSetBase<Derived> const & other) : BitSet(other.max_size()) {
        *this = other;
    }

    using BitSetMutableBase<BitSet>::operator=;

    std::size_t size() const { return _size; }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const { return _chunks[i]; }
    detail::Chunk & _get_chunk_ref(detail::ChunkIndex i) { return _chunks[i]; }
    detail::ChunkSize _n_chunks() const { return _chunks.size(); }

    std::size_t _size;
    std::vector<detail::Chunk> _chunks;
};


} // namespace cipells

#endif // !CIPELLS_BitSet_h_INCLUDED
