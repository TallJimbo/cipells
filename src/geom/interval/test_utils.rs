use std::slice::Iter;
use std::iter::Chain;

use super::{Interval, IntervalElement, AbstractInterval};

#[derive(Default)]
pub struct TestIntervals<T: IntervalElement> {
    nonsingular: Vec<Interval<T>>,
    singular: Vec<Interval<T>>,
    empty: Vec<Interval<T>>,
}

impl<T> TestIntervals<T>
    where T: IntervalElement + Default + AbstractInterval<T>
{
    pub fn new(points: &Vec<T>) -> Self{
        let mut result: Self = Default::default();
        for (i, lhs) in points.iter().enumerate() {
            for (j, rhs) in points.iter().enumerate() {
                if i < j {
                    result.nonsingular.push(Interval::new(*lhs..=*rhs));
                } else if i > j {
                    result.empty.push(Interval::new(*lhs..=*rhs));
                } else {
                    result.singular.push(Interval::new(*lhs))
                }
            }
        }
        result
    }

    pub fn singular(&self) -> Iter<Interval<T>> {
        self.singular.iter()
    }

    pub fn empty(&self) -> Iter<Interval<T>> {
        self.empty.iter()
    }

    pub fn finite(&self) -> Chain<Iter<Interval<T>>, Iter<Interval<T>>> {
        self.nonsingular.iter().chain(self.singular.iter())
    }

    pub fn all(&self) -> Chain<Chain<Iter<Interval<T>>, Iter<Interval<T>>>, Iter<Interval<T>>> {
        self.finite().chain(self.empty())
    }
}
