use alga::general::RingCommutative;

pub type Index = i64;
pub type Real = f64;

pub trait Scalar:
    'static
    + std::ops::Neg
    + RingCommutative
    + PartialOrd
    + PartialEq
    + Copy
    + std::fmt::Debug
    + std::fmt::Display
{
    fn min(self, other: Self) -> Self;
    fn max(self, other: Self) -> Self;
    fn size(lower: Self, upper: Self) -> Self;
}

#[derive(Debug, Clone, PartialEq)]
pub struct Interval<T: Scalar> {
    lower: T,
    upper: T,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Rect<T: Scalar> {
    pub x: Interval<T>,
    pub y: Interval<T>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Bounds<R> {
    Bounded(R),
    Empty,
}

pub use self::Bounds::*;

mod detail {

    pub trait BoundedValue: Sized {
        type Point;
        fn new(lower: Self::Point, upper: Self::Point) -> super::Bounds<Self>;
    }

} // mod detail

pub trait AbstractBounds<I> {
    fn is_empty(&self) -> bool;

    fn contains<U>(&self, other: U) -> bool
    where
        Bounds<I>: From<U>;

    fn intersects<U>(&self, other: U) -> bool
    where
        Bounds<I>: From<U>;

    fn clipped_to<U>(&self, other: U) -> Bounds<I>
    where
        Bounds<I>: From<U>;

    fn expanded_to<U>(&self, other: U) -> Bounds<I>
    where
        Bounds<I>: From<U>;
}

mod impl_bounds;
mod impl_interval;
mod impl_rect;
mod impl_scalar;
