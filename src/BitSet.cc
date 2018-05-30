#define CIPELLS_BitSet_cc_SRC

#include <algorithm>

#include "fmt/format.h"

#include "cipells/BitSet.h"

namespace cipells {

namespace detail {

constexpr ChunkIndex BITS_PER_CHUNK = sizeof(Chunk)*8u;

constexpr Chunk TOP_CHUNK_MASK = Chunk(1u) << (BITS_PER_CHUNK - 1u);

ChunkIndex BitSetAccess::count_chunks(std::size_t nbits) {
    return nbits/BITS_PER_CHUNK + (nbits%BITS_PER_CHUNK != 0u);
}

BitKey BitSetAccess::first(std::shared_ptr<BitSchema const> schema) {
    return BitKey(0u, 0u, std::move(schema));
}

BitKey BitSetAccess::next(BitKey const & current) {
    BitKey r(current);
    if (r._mask == TOP_CHUNK_MASK) {
        ++r._index;
        r._mask = 0u;
    } else {
        r._mask <<= 1;
    }
    return r;
}

} // namespace detail


bool BitKey::operator==(BitKey const & rhs) const {
    return _index == rhs._index && _mask == rhs._mask && schema() == rhs.schema();
}

bool BitKey::operator<(BitKey const & rhs) const {
    if (_index < rhs._index) {
        return true;
    } else if (_index == rhs._index) {
        return _mask < rhs._mask;
    }
    return false;
}

bool BitKey::operator<=(BitKey const & rhs) const {
    if (_index < rhs._index) {
        return true;
    } else if (_index == rhs._index) {
        return _mask <= rhs._mask;
    }
    return false;
}

BitKey::BitKey() :
    _mask(0u), _index(0u), _schema()
{}

BitKey::BitKey(detail::Chunk mask, detail::ChunkIndex index, std::shared_ptr<BitSchema const> schema) :
    _mask(mask), _index(index), _schema(std::move(schema))
{}


std::shared_ptr<BitSchema> BitSchema::make(std::size_t max_size) {
    return std::shared_ptr<BitSchema>(new BitSchema(max_size));
}

bool BitSchema::operator==(BitSchema const & rhs) const {
    if (this == &rhs) return true;  // pointer equality shortcut for common case
    return _n_chunks == rhs._n_chunks && size() == rhs.size() &&
        std::equal(begin(), end(), rhs.begin(),
                   [](Item const & a, Item const & b) { return a.name == b.name; });
}

std::size_t BitSchema::max_size() const { return _n_chunks*detail::BITS_PER_CHUNK; }

BitSchema::const_iterator BitSchema::find(std::string const & name) const {
    return std::find_if(begin(), end(), [&name](Item const & item) { return item.name == name; });
}

BitSchema::const_iterator BitSchema::find(BitKey const & key) const {
    return std::lower_bound(begin(), end(), key, [](Item const & a, BitKey const & b) { return a.key < b; });
}

BitSchema::Item const & BitSchema::operator[](std::string const & name) const {
    auto i = find(name);
    if (i == end()) {
        throw std::out_of_range(fmt::format("Bit with name '{:s}' not found.", name));
    }
    return *i;
}

BitSchema::Item const & BitSchema::operator[](BitKey const & key) const {
    auto i = find(key);
    if (i == end()) {
        throw std::out_of_range("Bit not found.");
    }
    return *i;
}

BitSchema::BitSchema(std::size_t max_size) :
    _items(),
    _n_chunks(detail::BitSetAccess::count_chunks(max_size))
{
    assert(this->max_size() >= max_size);  // We should have rounded up to the nearest full chunk.
    _items.reserve(this->max_size());
}

std::shared_ptr<BitSchema> BitSchema::copy(std::size_t max_size) const {
    if (max_size == 0u) {
        max_size = this->max_size();
    }
    if (max_size < this->size()) {
        throw std::length_error(fmt::format("New max_size {:d} is less than current size {:d}.",
                                            max_size, this->size()));
    }
    auto r = make(max_size);
    r->_items.insert(r->_items.begin(), this->begin(), this->end());
    return r;
}

BitKey const & BitSchema::append(std::string const & name, std::string const & description) {
    if (_items.size() == max_size()) {
        throw std::length_error("Not enough room in BitSchema.");
    }
    if (_items.empty()) {
        _items.push_back({name, description, detail::BitSetAccess::first(shared_from_this())});
    } else {
        _items.push_back({name, description, detail::BitSetAccess::next(_items.back().key)});
    }
    return _items.back().key;
}


} // namespace cipells
