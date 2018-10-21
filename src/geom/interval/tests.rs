use std::f64;

use rand::{Rng, XorShiftRng, SeedableRng};
use try_from::TryFrom;

use super::test_utils::TestIntervals;
use super::{Interval, IntervalElement, AbstractInterval, RealInterval, IndexInterval};


struct TestSuite<T: IntervalElement> {
    points: Vec<T>,
    intervals: TestIntervals<T>,
    a: T,
    b: T,
    ab: Interval<T>,
    ba: Interval<T>,
}


impl<T> TestSuite<T>
    where T: IntervalElement + Default + AbstractInterval<T>
{
    pub fn new(points: Vec<T>) -> Self {
        let a = points[0];
        let b = points[1];
        let intervals = TestIntervals::new(&points);
        Self {
            points,
            intervals,
            a,
            b,
            ab: Interval::new(a..=b),
            ba: Interval::new(b..=a),
        }
    }

    pub fn accessors(&self) {
        assert_eq!(self.ab.min().unwrap(), self.a);
        assert_eq!(self.ab.max().unwrap(), self.b);
        assert_eq!(self.ba.min(), None);
        assert_eq!(self.ba.min(), None);
    }

    pub fn empty(&self) {
        assert!(self.intervals.finite().all(|i| !i.is_empty()));
        assert!(self.intervals.empty().all(|i| i.is_empty()));
    }

    pub fn hull(&self) {
        let mut rng = XorShiftRng::from_seed([0; 16]);
        for (n1, p1) in self.points.iter().enumerate() {
            for (n2, p2) in self.points.iter().enumerate().skip(n1) {
                let mut seq = self.points[n1..=n2].to_vec();
                let i = Interval::new(*p1..=*p2);
                assert_eq!(i, Interval::hull(seq.iter().cloned()));
                assert_eq!(i, Interval::hull(seq.iter().rev().cloned()));
                rng.shuffle(&mut seq);
                assert_eq!(i, Interval::hull(seq.iter().cloned()));
            }
        }
    }

    pub fn contains(&self) {
        for lhs in self.intervals.finite() {
            for rhs in self.intervals.finite() {
                assert_eq!(lhs.contains(rhs),
                           lhs.min().unwrap() <= rhs.min().unwrap() &&
                           lhs.max().unwrap() >= rhs.max().unwrap());
            }
            for rhs in self.intervals.empty() {
                assert!(lhs.contains(rhs));
            }
            for rhs in &self.points {
                assert_eq!(lhs.contains(*rhs),
                           lhs.min().unwrap() <= *rhs && lhs.max().unwrap() >= *rhs);
            }
        }
        for lhs in self.intervals.empty() {
            for rhs in self.intervals.finite() {
                assert!(!lhs.contains(rhs));
            }
            for rhs in self.intervals.empty() {
                assert!(lhs.contains(rhs));
            }
        }
    }

    pub fn intersects(&self) {
        for lhs in self.intervals.finite() {
            for rhs in self.intervals.finite() {
                assert_eq!(lhs.intersects(rhs),
                           (lhs.min().unwrap() <= rhs.max().unwrap() &&
                            lhs.max().unwrap() >= rhs.min().unwrap()) ||
                           (rhs.min().unwrap() <= lhs.max().unwrap() &&
                            rhs.max().unwrap() >= lhs.min().unwrap()) ||
                           lhs.contains(rhs) || rhs.contains(lhs))
            }
            for rhs in self.intervals.empty() {
                assert!(!lhs.intersects(rhs));
            }
        }
        for lhs in self.intervals.empty() {
            for rhs in self.intervals.all() {
                assert!(!lhs.intersects(rhs));
            }
        }
    }

    pub fn equality(&self) {
        for lhs in self.intervals.all() {
            for rhs in self.intervals.all() {
                let should_be_equal = lhs.contains(rhs) && rhs.contains(lhs);
                assert_eq!(lhs == rhs, should_be_equal);
                assert_eq!(lhs != rhs, !should_be_equal);
            }
        }
    }

    pub fn shifted_by(&self) {
        for lhs in self.intervals.finite() {
            for rhs in &self.points {
                let shifted = lhs.shifted_by(*rhs);
                assert_eq!(lhs.size(), shifted.size());
                assert_eq!(lhs.min().unwrap() + *rhs, shifted.min().unwrap());
                assert_eq!(lhs.max().unwrap() + *rhs, shifted.max().unwrap());
            }
        }
        for lhs in self.intervals.empty() {
            for rhs in &self.points {
                let shifted = lhs.shifted_by(*rhs);
                assert!(shifted.is_empty());
            }
        }
    }

    pub fn clipped_to(&self) {
        for lhs in self.intervals.all() {
            for rhs in self.intervals.all() {
                let clipped = lhs.clipped_to(rhs);
                assert!(lhs.contains(&clipped));
                assert!(rhs.contains(&clipped));
                assert_eq!(clipped.is_empty(),
                           lhs.is_empty() || rhs.is_empty() || !lhs.intersects(rhs));
                assert_eq!(clipped == *rhs, lhs.contains(rhs));
                assert_eq!(clipped == *lhs, rhs.contains(lhs));
            }
        }
    }

    pub fn expanded_to(&self) {
        for lhs in self.intervals.all() {
            for rhs in self.intervals.all() {
                let expanded = lhs.expanded_to(rhs);
                assert!(expanded.contains(lhs));
                assert!(expanded.contains(rhs));
                assert_eq!(expanded.is_empty(),
                           lhs.is_empty() && rhs.is_empty());
                assert_eq!(expanded == *lhs, lhs.contains(rhs));
                assert_eq!(expanded == *rhs, rhs.contains(lhs));
            }
        }
    }

    pub fn dilated_by(&self) {
        for lhs in self.intervals.finite() {
            for rhs in &self.points {
                let dilated = lhs.dilated_by(*rhs);
                if dilated.is_empty() {
                    println!("lhs={:?} < rhs={:?}", lhs.size(), T::from(-2).unwrap()*(*rhs));
                    assert!(lhs.size() < T::from(-2).unwrap()*(*rhs));
                } else {
                    assert_eq!(lhs.min().unwrap() - *rhs, dilated.min().unwrap());
                    assert_eq!(lhs.max().unwrap() + *rhs, dilated.max().unwrap());
                }
            }
        }
    }

    pub fn run(&self) {
        self.accessors();
        self.empty();
        self.hull();
        self.contains();
        self.intersects();
        self.equality();
        self.shifted_by();
        self.clipped_to();
        self.expanded_to();
        self.dilated_by();
    }
}

#[test]
fn real_suite() {
    let suite = TestSuite::new(vec![-1.5, 5.0, 6.75, 8.625]);
    suite.run();
}

#[test]
fn index_suite() {
    let suite = TestSuite::new(vec![-2, 4, 7, 11]);
    suite.run();
}

#[test]
fn real_construction() {
    let a = Interval::new(1.5..2.0);
    let b = Interval::new(1.5..=2.0);
    assert_eq!(a, b);
    assert_eq!(format!("{:3.1}", a), "[1.5, 2.0]");
    assert_eq!(format!("{:3.1}", b), "[1.5, 2.0]");

    let i = Interval::new(1.5..);
    assert_eq!(i.min().unwrap(), 1.5);
    assert_eq!(i.max().unwrap(), f64::INFINITY);
    assert_eq!(i.size(), f64::INFINITY);
    assert_eq!(format!("{:3.2}", i), "[1.50, inf)");

    let i = Interval::new(..2.0);
    assert_eq!(i.min().unwrap(), f64::NEG_INFINITY);
    assert_eq!(i.max().unwrap(), 2.0);
    assert_eq!(i.size(), f64::INFINITY);
    assert_eq!(format!("{:4.2}", i), "(-inf, 2.00]");

    let i = Interval::new(..=2.0);
    assert_eq!(i.min().unwrap(), f64::NEG_INFINITY);
    assert_eq!(i.max().unwrap(), 2.0);
    assert_eq!(i.size(), f64::INFINITY);
    assert_eq!(format!("{:4.2}", i), "(-inf, 2.00]");

    let i = Interval::new(..);
    assert_eq!(i.min().unwrap(), f64::NEG_INFINITY);
    assert_eq!(i.max().unwrap(), f64::INFINITY);
    assert_eq!(i.size(), f64::INFINITY);
    assert_eq!(format!("{}", i), "(-inf, inf)");

    let i = Interval::<f64>::new(());
    assert!(i.is_empty());
    assert_eq!(format!("{}", i), "()");

    let i = Interval::new(1.5);
    assert_eq!(i.min().unwrap(), 1.5);
    assert_eq!(i.max().unwrap(), 1.5);
    assert_eq!(i.size(), 0.0);

    let i = Interval::with_center_and_size(3.0, 1.0);
    assert_eq!(i.min().unwrap(), 2.5);
    assert_eq!(i.max().unwrap(), 3.5);
    assert_eq!(i.size(), 1.0);
    assert_eq!(i.center(), 3.0);
}

#[test]
fn index_construction() {
    let a = Interval::new(-2..3);
    let b = Interval::new(-2..=2);
    assert_eq!(a, b);
    assert_eq!(format!("{}", a), "[-2, 2]");
    assert_eq!(format!("{}", b), "[-2, 2]");

    let i = Interval::<i32>::new(());
    assert!(i.is_empty());
    assert_eq!(format!("{}", i), "()");

    let i = Interval::new(3);
    assert_eq!(i.min().unwrap(), 3);
    assert_eq!(i.max().unwrap(), 3);
    assert_eq!(i.size(), 0);
    assert_eq!(format!("{}", i), "[3, 3]");
}

#[test]
fn conversion() {
    let f1 = Interval::new(5.75..=8.25);
    let i1 = IndexInterval::try_from(&f1).unwrap();
    let f2 = RealInterval::from(&i1);
    let i2 = IndexInterval::try_from(&f2).unwrap();
    assert_eq!(i1, i2);
    assert!(f2.contains(&f1));

    assert!(IndexInterval::try_from(&RealInterval::new(..6.0)).is_err());
    assert!(IndexInterval::try_from(&RealInterval::new(3.0..)).is_err());
    assert!(IndexInterval::try_from(&RealInterval::new(..)).is_err());
    assert_eq!(IndexInterval::try_from(&RealInterval::new(())).unwrap(),
               IndexInterval::new(()));
    assert_eq!(RealInterval::from(&IndexInterval::new(())),
               RealInterval::new(()));
}
