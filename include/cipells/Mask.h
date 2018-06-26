#ifndef CIPELLS_Mask_h_INCLUDED
#define CIPELLS_Mask_h_INCLUDED

#include "cipells/BitSet.h"

namespace cipells {


class BitSetRef : public BitSetMutableBase<BitSetRef> {
public:

    std::shared_ptr<BitSchema const> schema() const { return _schema->shared_from_this(); }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const { return _chunks[i]; }
    detail::Chunk & _get_chunk_ref(detail::ChunkIndex i) { return _chunks[i]; }

    detail::Chunk * _chunks;
    BitSchema const * _schema;
};


class BitSetCRef : public BitSetBase<BitSetCRef> {
public:

    std::shared_ptr<BitSchema const> schema() const { return _schema->shared_from_this(); }

private:

    friend class detail::BitSetAccess;

    detail::Chunk _get_chunk(detail::ChunkIndex i) const { return _chunks[i]; }

    detail::Chunk const * _chunks;
    BitSchema const * _schema;
};


} // namespace cipells

#endif // !CIPELLS_Mask_h_INCLUDED
