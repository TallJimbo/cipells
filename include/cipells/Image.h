#ifndef CIPELLS_Image_h_INCLUDED
#define CIPELLS_Image_h_INCLUDED

#include <memory>
#include <type_traits>
#include <complex>

#include "Eigen/Core"
#include "cipells/XYTuple.h"
#include "cipells/Box.h"
#include "cipells/fwd/Image.h"

namespace cipells {

using ImageStorage = std::aligned_union_t<16, float, std::complex<float>>;
using ImageOwner = std::shared_ptr<ImageStorage const>;


template <typename T>
class Image<T const> {
public:

    using Scalar = T;

    using Array = Eigen::Map<Eigen::Array<Scalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> const,
                             Eigen::Unaligned, Eigen::OuterStride<>>;

    Image();

    Image(Scalar const * data, IndexBox const & bbox, ImageOwner owner=nullptr, Index stride=0);

    Image(Image const &);
    Image(Image &&);

    Image & operator=(Image const &);
    Image & operator=(Image &&);

    Scalar const & operator[](Index2 const & index) const {
        assert(bbox().contains(index));
        return data()[_offset(index)];
    }

    Image operator[](IndexBox const & box) const {
        assert(bbox().contains(box));
        return Image(data() + _offset(box.min()), box, owner(), stride());
    }

    Array array() const {
        return Array(data(), this->bbox().height(), this->bbox().width(),
                     Eigen::OuterStride<>(this->stride()));
    }

    Array array(IndexBox const & box) const {
        assert(this->bbox().contains(box));
        return Array(data() + this->_offset(box.min()), box.height(), box.width(),
                     Eigen::OuterStride<>(this->stride()));
    }

    Image<T> copy() const;

    Image<T const> freeze() &&;

    IndexBox const & bbox() const { return _bbox; }

    Scalar const * data() const { return _data; }

    Index stride() const { return _stride; }

    ImageOwner const & owner() const { return _owner; }

protected:

    Index _offset(Index2 const & index) const {
        return _stride*(index.y() - _bbox.y0()) + (index.x() - _bbox.x0());
    }

    explicit Image(IndexBox const & box);

private:
    Scalar const * _data;
    Index _stride;
    ImageOwner _owner;
    IndexBox _bbox;
};

template <typename T>
class Image : public Image<T const> {
    using Base = Image<T const>;
public:

    using Scalar = T;

    using Array = Eigen::Map<Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
                             Eigen::Unaligned, Eigen::OuterStride<>>;

    Image() = default;

    Image(Scalar * data, IndexBox const & bbox, ImageOwner owner=nullptr, Index stride=0) :
        Base(data, bbox, std::move(owner), stride)
    {}

    explicit Image(IndexBox const & box);

    Image(Image const &) = default;
    Image(Image &&) = default;

    Image & operator=(Image const &) = default;
    Image & operator=(Image &&) = default;

    Scalar & operator[](Index2 const & index) const {
        assert(this->bbox().contains(index));
        return data()[this->_offset(index)];
    }

    Image operator[](IndexBox const & box) const {
        assert(this->bbox().contains(box));
        return Image(data() + this->_offset(box.min()), box, this->owner(), this->stride());
    }

    Array array() const {
        return Array(data(), this->bbox().height(), this->bbox().width(),
                     Eigen::OuterStride<>(this->stride()));
    }

    Array array(IndexBox const & box) const {
        assert(this->bbox().contains(box));
        return Array(data() + this->_offset(box.min()), box.height(), box.width(),
                     Eigen::OuterStride<>(this->stride()));
    }

    Scalar * data() const { return const_cast<Scalar*>(Base::data()); }

};


template <typename T, typename Func>
void apply(Image<T> const & image, Func func) {
    T * row = image.data();
    for (Index y = image.bbox().y().min(); y <= image.bbox().y().max(); ++y, row += image.stride()) {
        T * pixel = row;
        for (Index2 index(image.bbox().x().min(), y); index.x() <= image.bbox().x().max(); ++index.x(), ++pixel) {
            func(index, *pixel);
        }
    }
}


} // namespace cipells

#endif // !CIPELLS_Image_h_INCLUDED
