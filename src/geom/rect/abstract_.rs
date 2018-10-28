use std::ops::{RangeInclusive, Range, RangeTo, RangeFrom, RangeToInclusive, RangeFull};

use super::{Scalar, Rect, Point2, Interval, Real, Index};


pub trait AbstractRect<T> {
    type Element;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element);
}

impl<T> AbstractRect<T> for Rect<T>
    where T: Scalar
{
    type Element = Interval<T>;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) {
        (self.x, self.y)
    }
}

impl<'a, T> AbstractRect<T> for &'a Rect<T>
    where T: Scalar
{
    type Element = &'a Interval<T>;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) {
        (&self.x, &self.y)
    }
}

impl<T> AbstractRect<T> for RangeInclusive<Point2<T>>
    where T: Scalar
{
    type Element = RangeInclusive<T>;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) {
        (self.start().x..=self.end().x,
         self.start().y..=self.end().y)
    }
}

impl AbstractRect<Real> for Point2<Real> {
    type Element = Real;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) { (self.x, self.y) }
}

impl AbstractRect<Index> for Point2<Index> {
    type Element = Index;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) { (self.x, self.y) }
}

impl<'a> AbstractRect<Real> for &'a Point2<Real> {
    type Element = Real;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) { (self.x, self.y) }
}

impl<'a> AbstractRect<Index> for &'a Point2<Index> {
    type Element = Index;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) { (self.x, self.y) }
}

impl<T> AbstractRect<T> for Range<Point2<T>>
    where T: Scalar
{
    type Element = Range<T>;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) {
        (self.start.x..self.end.x,
         self.start.y..self.end.y)
    }
}

impl AbstractRect<Real> for RangeTo<Point2<Real>> {
    type Element = RangeTo<Real>;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) {
        (..self.end.x, ..self.end.y)
    }
}

impl AbstractRect<Real> for RangeToInclusive<Point2<Real>> {
    type Element = RangeToInclusive<Real>;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) {
        (..=self.end.x, ..=self.end.y)
    }
}

impl AbstractRect<Real> for RangeFrom<Point2<Real>> {
    type Element = RangeFrom<Real>;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) {
        (self.start.x.., self.start.y..)
    }
}

impl AbstractRect<Real> for RangeFull {
    type Element = RangeFull;
    fn to_xy_tuple(self) -> (Self::Element, Self::Element) {
        (.., ..)
    }
}
