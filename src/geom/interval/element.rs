use std::fmt;
use std::f64;

use num::{NumCast, Signed};


pub trait IntervalElement :
    'static + Signed + PartialOrd + PartialEq + Copy + NumCast + fmt::Debug + fmt::Display
{
    fn empty_min() -> Self;
    fn empty_max() -> Self;
    fn min(self, other: Self) -> Self;
    fn max(self, other: Self) -> Self;
}


impl IntervalElement for i32 {
    fn empty_min() -> Self { 0 }
    fn empty_max() -> Self { -1 }
    fn min(self, other: Self) -> Self { Ord::min(self, other) }
    fn max(self, other: Self) -> Self { Ord::max(self, other) }
}


impl IntervalElement for f64 {
    fn empty_min() -> Self { f64::NAN }
    fn empty_max() -> Self { f64::NAN }
    fn min(self, other: Self) -> Self { self.min(other) }
    fn max(self, other: Self) -> Self { self.max(other) }
}

