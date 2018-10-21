mod interval;

pub use self::interval::{Interval, RealInterval, IndexInterval, RealToIndexError};

#[cfg(test)]
pub use self::interval::test_utils;
