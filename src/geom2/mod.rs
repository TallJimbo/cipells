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

    pub trait GuaranteedBounded: Sized + 'static {
        type Point;
        fn _new(lower: Self::Point, upper: Self::Point) -> super::Bounds<Self>;
        fn _expand_to<U: super::AbstractBounds<Self>>(&mut self, other: U);
        // low-level implementation of clip is not in-place because it can
        // transform non-Empty to Empty.
        fn _clipped_to<U: super::AbstractBounds<Self>>(&self, other: U) -> super::Bounds<Self>;
        fn _shift_by(&mut self, point: Self::Point);
    }

} // mod detail

pub trait AbstractBounds<R> {
    fn to_bounds(self) -> Bounds<R>;

    fn is_empty(self) -> bool;

    fn contains<U>(self, other: U) -> bool
    where
        U: AbstractBounds<R>;

    fn intersects<U>(self, other: U) -> bool
    where
        U: AbstractBounds<R>;

    fn clipped_to<U>(self, other: U) -> Bounds<R>
    where
        U: AbstractBounds<R>;

    fn expanded_to<U>(self, other: U) -> Bounds<R>
    where
        U: AbstractBounds<R>;
}

mod impl_bounds;
mod impl_interval;
mod impl_rect;
mod impl_scalar;
