use std::f64;
use std::cmp::Ordering;
use std::ops::{
    Range, RangeTo, RangeFrom, RangeFull,
    RangeInclusive, RangeToInclusive
};

pub trait GenericInterval<T>
    where T: PartialOrd
{
    fn to_min_max_tuple(self) -> (T, T);
}

impl GenericInterval<f64> for super::RealInterval {
    fn to_min_max_tuple(self) -> (f64, f64) { (self.min, self.max) }
}

impl<'a> GenericInterval<f64> for &'a super::RealInterval {
    fn to_min_max_tuple(self) -> (f64, f64) { (self.min, self.max) }
}

impl GenericInterval<f64> for f64 {
    fn to_min_max_tuple(self) -> (f64, f64) { (self, self) }
}

impl GenericInterval<f64> for Range<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (self.start, self.end) }
}

impl GenericInterval<f64> for RangeTo<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (f64::NEG_INFINITY, self.end) }
}

impl GenericInterval<f64> for RangeFrom<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (self.start, f64::INFINITY) }
}

impl GenericInterval<f64> for RangeFull {
    fn to_min_max_tuple(self) -> (f64, f64) { (f64::NEG_INFINITY, f64::INFINITY) }
}

impl GenericInterval<f64> for RangeInclusive<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (*self.start(), *self.end()) }
}

impl GenericInterval<f64> for RangeToInclusive<f64> {
    fn to_min_max_tuple(self) -> (f64, f64) { (f64::NEG_INFINITY, self.end) }
}

fn is_empty_generic(min: f64, max: f64) -> bool { !(min <= max) }

#[derive(Debug, Copy, Clone, PartialEq)]
pub struct RealInterval {
    min: f64,
    max: f64,
}

impl RealInterval {

    pub fn new<T: GenericInterval<f64>>(other: T) -> Self {
        let (min, max) = other.to_min_max_tuple();
        if is_empty_generic(min, max) {
            Default::default()
        } else {
            Self { min, max }
        }
    }

    pub fn from_center_and_size(x: f64, size: f64) -> Self {
        let min = x - 0.5*size;
        Self::new(min..=min + size)
    }

    pub fn from_hull_iter<T>(iter: T) -> Self
        where T: IntoIterator<Item=f64>
    {
        let mut result: Self = Default::default();
        for x in iter {
            result = result.expanded_to(x);
        }
        result
    }

    pub fn min(&self) -> f64 { self.min }

    pub fn max(&self) -> f64 { self.max }

    pub fn center(&self) -> f64 { 0.5*(self.min + self.max) }

    pub fn is_empty(&self) -> bool { is_empty_generic(self.min, self.max) }

    pub fn contains<T: GenericInterval<f64>>(&self, other: T) -> bool {
        let (min, max) = other.to_min_max_tuple();
        if is_empty_generic(min, max) {
            true
        } else if self.is_empty() {
            false
        } else {
            self.min <= min && self.max >= max
        }
    }

    pub fn intersects<T: GenericInterval<f64>>(&self, other: T) -> bool {
        let (min, max) = other.to_min_max_tuple();
        if self.is_empty() || is_empty_generic(min, max) {
            false
        } else {
            !(self.max < min || self.min > max)
        }
    }

    pub fn shifted_by(self, x: f64) -> Self {
        Self { min: self.min + x, max: self.max + x }
    }

    pub fn clipped_to<T: GenericInterval<f64>>(self, other: T) -> Self {
        if self.is_empty() {
            Default::default()
        } else {
            let (min, max) = other.to_min_max_tuple();
            if is_empty_generic(min, max) {
                Default::default()
            } else {
                Self::new(self.min.max(min)..=self.max.min(max))
            }
        }
    }

    pub fn expanded_to<T: GenericInterval<f64>>(self, other: T) -> Self {
        let (min, max) = other.to_min_max_tuple();
        if is_empty_generic(min, max) {
            self
        } else if self.is_empty() {
            Self::new(min..=max)
        } else {
            Self { min: self.min.min(min), max: self.max.max(max) }
        }
    }

    pub fn dilated_by(self, x: f64) -> Self {
        if self.is_empty() || x.is_nan() {
            self
        } else {
            Self::new((self.min - x)..=(self.max + x))
        }
    }

    pub fn eroded_by(self, x: f64) -> Self {
        self.dilated_by(-x)
    }

}

impl Default for RealInterval {
    fn default() -> Self {
        Self { min: f64::NAN, max: f64::NAN }
    }
}

impl PartialOrd for RealInterval {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        if self.max() < other.min() {
            Some(Ordering::Less)
        } else if self.min() > other.max() {
            Some(Ordering::Greater)
        } else if self.min() == other.min() && self.max() == other.max() {
            Some(Ordering::Equal)
        } else {
            None
        }
    }
}


#[cfg(test)]
mod tests {
    use super::RealInterval;

    #[test]
    fn real_ctors() {
        let a = RealInterval::new(1.5..8.75);
        let b = RealInterval::new(1.5);
        assert!(!a.is_empty());
        assert!(!b.is_empty());
        assert!(!(a < b));
        assert!(!(b < a));
        assert_ne!(a, b);

        let c = b.expanded_to(a.max());
        assert_eq!(a, c);
        assert!(a.contains(b));
        assert!(a.contains(&b));
        assert!(a.contains(5.0));
        assert!(a.contains(2.0..5.0));
    }
}
