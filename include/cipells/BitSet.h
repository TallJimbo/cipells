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

    std::shared_ptr<BitSchema const> schema() const { return _schema.lock(); }

    bool operator==(BitKey const & rhs) const;
    bool operator!=(BitKey const & rhs) const { return !(*this == rhs); }

    bool operator<(BitKey const & rhs) const;
    bool operator<=(BitKey const & rhs) const;

    bool operator>(BitKey const & rhs) const { return !(*this <= rhs); }
    bool operator>=(BitKey const & rhs) const { return !(*this < rhs); }

private:

    friend class detail::BitSetAccess;
    friend class SingleIndexBitSet;

    BitKey(detail::Chunk mask, detail::ChunkIndex index, std::shared_ptr<BitSchema const> schema);

    detail::Chunk _mask;
    detail::ChunkIndex _index;
    std::weak_ptr<BitSchema const> _schema;
};


namespace detail {

struct BitSetAccess {

    static ChunkIndex count_chunks(std::size_t nbits);

    static BitKey first(std::shared_ptr<BitSchema const> schema);

    static BitKey next(BitKey const & current);

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
        assert(*lhs.schema() == *rhs.schema());
        ChunkIndex const n = lhs.schema()->_n_chunks;
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


class BitSchema : public std::enable_shared_from_this<BitSchema> {
public:

    struct Item {
        std::string name;
        std::string description;
        BitKey key;
    };

    using const_iterator = std::vector<Item>::const_iterator;

    static std::shared_ptr<BitSchema> make(std::size_t max_size);

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

    std::shared_ptr<BitSchema> copy(std::size_t max_size=0) const;

    std::shared_ptr<BitSchema> compressed() const { return copy(this->size()); }

    BitKey const & append(std::string const & name, std::string const & description);

    // TODO: extend() -> BitTranslator

private:

    friend class detail::BitSetAccess;

    explicit BitSchema(std::size_t max_size);

    std::vector<Item> _items;
    detail::ChunkIndex const _n_chunks;
};


template <typename Derived>
class BitSetBase {
public:

    bool operator[](BitKey const & index) const {
        assert(*index.schema() == *schema());
        return detail::BitSetAccess::get_bit(*this, index);
    }

    std::shared_ptr<BitSchema const> schema() const {
        return static_cast<Derived const &>(*this).schema();
    }

    std::size_t size() const { return schema().size(); }

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

    std::shared_ptr<BitSchema const> schema() const { return _arg.schema(); }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const {
        return F::operator()(detail::BitSetAccess::get_chunk(_arg, i));
    }

    E _arg;
};


template <typename L, typename R, typename F>
class BitSetBinaryExpression : public BitSetBase<BitSetBinaryExpression<L, R, F>>, private F {
public:

    BitSetBinaryExpression(L lhs, R rhs, F func=F()) : F(func), _lhs(std::move(lhs)), _rhs(std::move(rhs)) {
        assert(*_lhs.schema() == *_rhs.schema());
    }

    std::shared_ptr<BitSchema const> schema() const { return _lhs.schema(); }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const {
        return F::operator()(detail::BitSetAccess::get_chunk(_lhs, i),
                             detail::BitSetAccess::get_chunk(_rhs, i));
    }

    L _lhs;
    R _rhs;
};


class SingleIndexBitSet : public BitSetBase<SingleIndexBitSet> {
public:

    std::shared_ptr<BitSchema const> schema() const { return _key.schema(); }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const {
        return (i == _key._index) ? _key._mask : detail::Chunk(0u);
    }

    BitKey _key;
};


class BitSet : public BitSetMutableBase<BitSet> {
public:

    explicit BitSet(std::shared_ptr<BitSchema const> schema);

    using BitSetMutableBase<BitSet>::operator=;

    template <typename Derived>
    BitSet(BitSetBase<Derived> const & other) : BitSet(other.schema()) {
        *this = other;
    }

    std::shared_ptr<BitSchema const> schema() const { return _schema; }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const { return _chunks[i]; }
    detail::Chunk & _get_chunk_ref(detail::ChunkIndex i) { return _chunks[i]; }

    std::vector<detail::Chunk> _chunks;
    std::shared_ptr<BitSchema const> _schema;
};


} // namespace cipells

#endif // !CIPELLS_BitSet_h_INCLUDED
