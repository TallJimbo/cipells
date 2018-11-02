use std::f64;
use std::fmt;

use alga::general::RingCommutative;
pub use nalgebra::geometry::Point2;
pub use nalgebra::Vector2;
use num::{NumCast, Signed};

pub type Real = f64;
pub type Index = i32;

pub type RealPoint = Point2<Real>;
pub type IndexPoint = Point2<Index>;
pub type RealVector = Vector2<Real>;
pub type IndexVector = Vector2<Index>;

pub trait Scalar:
    'static
    + Signed
    + RingCommutative
    + PartialOrd
    + PartialEq
    + Copy
    + NumCast
    + fmt::Debug
    + fmt::Display
{
    fn empty_interval_min() -> Self;
    fn empty_interval_max() -> Self;
    fn min(self, other: Self) -> Self;
    fn max(self, other: Self) -> Self;
    fn max_from_size(self, size: Self) -> Self;
}

impl Scalar for Index {
    fn empty_interval_min() -> Self {
        0
    }
    fn empty_interval_max() -> Self {
        -1
    }
    fn min(self, other: Self) -> Self {
        Ord::min(self, other)
    }
    fn max(self, other: Self) -> Self {
        Ord::max(self, other)
    }
    fn max_from_size(self, size: Self) -> Self {
        self + size - 1
    }
}

impl Scalar for Real {
    fn empty_interval_min() -> Self {
        f64::NAN
    }
    fn empty_interval_max() -> Self {
        f64::NAN
    }
    fn min(self, other: Self) -> Self {
        self.min(other)
    }
    fn max(self, other: Self) -> Self {
        self.max(other)
    }
    fn max_from_size(self, size: Self) -> Self {
        self + size
    }
}

mod interval;
pub use self::interval::{
    AbstractInterval, IndexInterval, Interval, RealInterval, RealToIndexError,
};

#[cfg(test)]
pub use self::interval::test_utils::TestIntervals;

mod rect;
pub use self::rect::{AbstractRect, IndexRect, RealRect, Rect};

#[cfg(test)]
pub use self::rect::test_utils::TestRects;
