use std::f64;
use std::ops::{RangeInclusive, Range, RangeTo, RangeFrom, RangeToInclusive, RangeFull};

use super::{IntervalElement, Interval};


pub trait AbstractInterval<T>
    where T: PartialOrd
{
    fn to_min_max_tuple(self) -> (T, T);
}

impl<'a, T> AbstractInterval<T> for &'a Interval<T>
    where T: IntervalElement
{
    fn to_min_max_tuple(self) -> (T, T) { (self.min, self.max) }
}

impl<T> AbstractInterval<T> for RangeInclusive<T>
    where T: IntervalElement
{
    fn to_min_max_tuple(self) -> (T, T) { (*self.start(), *self.end()) }
}

impl AbstractInterval<f64> for Range<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (self.start, self.end) }
}

impl AbstractInterval<i32> for Range<i32> {
    fn to_min_max_tuple(self) -> (i32, i32) { (self.start, self.end - 1) }
}

impl AbstractInterval<f64> for () {
    fn to_min_max_tuple(self) -> (f64, f64) { (f64::NAN, f64::NAN) }
}

impl AbstractInterval<i32> for () {
    fn to_min_max_tuple(self) -> (i32, i32) { (0, -1) }
}

impl AbstractInterval<f64> for f64 {
    fn to_min_max_tuple(self) -> (f64, f64) { (self, self) }
}

impl AbstractInterval<i32> for i32 {
    fn to_min_max_tuple(self) -> (i32, i32) { (self, self) }
}

impl AbstractInterval<f64> for RangeTo<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (f64::NEG_INFINITY, self.end) }
}

impl AbstractInterval<f64> for RangeFrom<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (self.start, f64::INFINITY) }
}

impl AbstractInterval<f64> for RangeFull {
    fn to_min_max_tuple(self) -> (f64, f64) { (f64::NEG_INFINITY, f64::INFINITY) }
}

impl AbstractInterval<f64> for RangeToInclusive<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (f64::NEG_INFINITY, self.end) }
}
