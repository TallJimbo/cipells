use std::f64;
use std::ops::{RangeInclusive, Range, RangeTo, RangeFrom, RangeToInclusive, RangeFull};

use super::{Scalar, Interval, Real, Index};


pub trait AbstractInterval<T>
    where T: PartialOrd
{
    fn to_min_max_tuple(self) -> (T, T);
}

impl<T> AbstractInterval<T> for Interval<T>
    where T: Scalar
{
    fn to_min_max_tuple(self) -> (T, T) { (self.min, self.max) }
}

impl<'a, T> AbstractInterval<T> for &'a Interval<T>
    where T: Scalar
{
    fn to_min_max_tuple(self) -> (T, T) { (self.min, self.max) }
}

impl<T> AbstractInterval<T> for RangeInclusive<T>
    where T: Scalar
{
    fn to_min_max_tuple(self) -> (T, T) { (*self.start(), *self.end()) }
}

impl AbstractInterval<Real> for Real {
    fn to_min_max_tuple(self) -> (Real, Real) { (self, self) }
}

impl AbstractInterval<Index> for Index {
    fn to_min_max_tuple(self) -> (Index, Index) { (self, self) }
}

impl AbstractInterval<Real> for () {
    fn to_min_max_tuple(self) -> (Real, Real) { (f64::NAN, f64::NAN) }
}

impl AbstractInterval<Index> for () {
    fn to_min_max_tuple(self) -> (Index, Index) { (0, -1) }
}

impl AbstractInterval<Real> for Range<Real> {
    fn to_min_max_tuple(self) -> (Real, Real) { (self.start, self.end) }
}

impl AbstractInterval<Index> for Range<Index> {
    fn to_min_max_tuple(self) -> (Index, Index) { (self.start, self.end - 1) }
}

impl AbstractInterval<Real> for RangeTo<Real> {
    fn to_min_max_tuple(self) -> (Real, Real) { (f64::NEG_INFINITY, self.end) }
}

impl AbstractInterval<Real> for RangeFrom<Real> {
    fn to_min_max_tuple(self) -> (Real, Real) { (self.start, f64::INFINITY) }
}

impl AbstractInterval<Real> for RangeFull {
    fn to_min_max_tuple(self) -> (Real, Real) { (f64::NEG_INFINITY, f64::INFINITY) }
}

impl AbstractInterval<Real> for RangeToInclusive<Real> {
    fn to_min_max_tuple(self) -> (Real, Real) { (f64::NEG_INFINITY, self.end) }
}
