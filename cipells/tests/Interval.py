import unittest
import itertools
import re
import numpy as np

from cipells import RealInterval, IndexInterval, Real, Index


class TestIntervals:

    def __init__(self, IntervalClass, points):
        self.nonsingular = []
        self.singular = []
        self.empty = []
        for i, lhs in enumerate(points):
            for j, rhs in enumerate(points):
                if i < j:
                    self.nonsingular.append(IntervalClass(min=lhs, max=rhs))
                elif i > j:
                    self.empty.append(IntervalClass(min=lhs, max=rhs))
                else:
                    self.singular.append(IntervalClass(min=lhs, max=rhs))

    @property
    def finite(self):
        return itertools.chain(self.nonsingular, self.singular)

    @property
    def all(self):
        return itertools.chain(self.nonsingular, self.singular, self.empty)


class IntervalTestMixin:

    def assertAllTrue(self, iterable):
        seq = list(iterable)
        self.assertEqual(seq, [True]*len(seq))

    def assertAllFalse(self, iterable):
        seq = list(iterable)
        self.assertEqual(seq, [False]*len(seq))

    def testAccessors(self):
        self.assertEqual(self.ab.min, self.a)
        self.assertEqual(self.ab.max, self.b)

    def testEmpty(self):
        self.assertTrue(self.IntervalClass().isEmpty())
        self.assertTrue(self.IntervalClass.makeEmpty().isEmpty())
        self.assertAllFalse(s.isEmpty() for s in self.intervals.finite)
        self.assertAllTrue(s.isEmpty() for s in self.intervals.empty)

    def testConstructor(self):
        for i in self.intervals.finite:
            self.assertEqual(i, self.IntervalClass(min=i.min, max=i.max))
            self.assertEqual(i, self.IntervalClass(min=i.min, size=i.size))
            self.assertEqual(i, self.IntervalClass(max=i.max, size=i.size))
            self.assertEqual(i, self.IntervalClass(center=i.center, size=i.size))

    def testMakeHull(self):
        for n1, p1 in enumerate(self.points):
            for n2, p2 in enumerate(self.points):
                seq = list(self.points[n1:n2+1])
                i = self.IntervalClass(min=p1, max=p2)
                self.assertEqual(i, self.IntervalClass.makeHull(seq))
                np.random.shuffle(seq)
                self.assertEqual(i, self.IntervalClass.makeHull(seq))
                seq.reverse()
                self.assertEqual(i, self.IntervalClass.makeHull(seq))

    def testContains(self):
        for lhs in self.intervals.finite:
            for rhs in self.intervals.finite:
                self.assertEqual(lhs.contains(rhs), lhs.min <= rhs.min and lhs.max >= rhs.max)
            for rhs in self.intervals.empty:
                self.assertTrue(lhs.contains(rhs))
            for rhs in self.points:
                self.assertEqual(lhs.contains(rhs), lhs.min <= rhs and lhs.max >= rhs)
        for lhs in self.intervals.empty:
            for rhs in self.intervals.finite:
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.intervals.empty:
                self.assertTrue(lhs.contains(rhs))

    def testIntersects(self):
        for lhs in self.intervals.finite:
            for rhs in self.intervals.finite:
                self.assertEqual(lhs.intersects(rhs),
                                 (lhs.min <= rhs.max and lhs.max >= rhs.min) or
                                 (rhs.min <= lhs.max and rhs.max >= lhs.min) or
                                 lhs.contains(rhs) or rhs.contains(lhs))
            for rhs in self.intervals.empty:
                self.assertFalse(lhs.intersects(rhs))
        for lhs in self.intervals.empty:
            for rhs in self.intervals.all:
                self.assertFalse(lhs.intersects(rhs))

    def testEquality(self):
        for lhs in self.intervals.all:
            for rhs in self.intervals.all:
                shouldBeEqual = lhs.contains(rhs) and rhs.contains(lhs)
                self.assertIs(lhs == rhs, shouldBeEqual)
                self.assertIs(lhs != rhs, not shouldBeEqual)

    def testClippedTo(self):
        for lhs in self.intervals.all:
            for rhs in self.intervals.all:
                clipped = lhs.clippedTo(rhs)
                self.assertTrue(lhs.contains(clipped))
                self.assertTrue(rhs.contains(clipped))
                self.assertIs(
                    clipped.isEmpty(),
                    lhs.isEmpty() or rhs.isEmpty() or not lhs.intersects(rhs)
                )
                self.assertIs(clipped == rhs, lhs.contains(rhs))
                self.assertIs(clipped == lhs, rhs.contains(lhs))

    def testExpandedTo(self):
        for lhs in self.intervals.all:
            for rhs in self.intervals.all:
                expanded = lhs.expandedTo(rhs)
                self.assertTrue(expanded.contains(lhs))
                self.assertTrue(expanded.contains(rhs))
                self.assertIs(
                    expanded.isEmpty(),
                    lhs.isEmpty() and rhs.isEmpty()
                )
                self.assertIs(expanded == rhs, rhs.contains(lhs))
                self.assertIs(expanded == lhs, lhs.contains(rhs))
            for rhs in self.points:
                self.assertEqual(lhs.expandedTo(rhs),
                                 lhs.expandedTo(self.IntervalClass(min=rhs, max=rhs)))

    def testDilatedBy(self):
        for lhs in self.intervals.finite:
            for rhs in self.points:
                dilated = lhs.dilatedBy(rhs)
                if dilated.isEmpty():
                    self.assertGreater(lhs.min - rhs, lhs.max + rhs)
                else:
                    self.assertEqual(lhs.min - rhs, dilated.min)
                    self.assertEqual(lhs.max + rhs, dilated.max)
        for lhs in self.intervals.empty:
            for rhs in self.points:
                self.assertTrue(lhs.dilatedBy(rhs).isEmpty())

    def testErodedBy(self):
        for lhs in self.intervals.all:
            for rhs in self.points:
                self.assertEqual(lhs.erodedBy(rhs), lhs.dilatedBy(-rhs))

    def testRepr(self):
        for i in self.intervals.all:
            self.assertEqual(eval(repr(i)), i)


class RealIntervalTestCase(unittest.TestCase, IntervalTestMixin):
    IntervalClass = RealInterval

    def setUp(self):
        self.points = [Real(-1.5), Real(5.0), Real(6.75), Real(8.625)]
        self.a = self.points[0]
        self.b = self.points[1]
        self.intervals = TestIntervals(self.IntervalClass, self.points)
        self.ab = self.IntervalClass(min=self.a, max=self.b)
        self.ba = self.IntervalClass(min=self.b, max=self.a)

    def testTypes(self):
        self.assertIs(self.IntervalClass.Scalar, Real)

    def testCenter(self):
        self.assertEqual(self.ab.center, 0.5*(self.a + self.b))

    def testStr(self):
        for i in self.intervals.all:
            if i.isEmpty():
                self.assertEqual(str(i), "[]")
            else:
                m = re.match(r"\[(\-?[\d\.]+), (\-?[\d\.]+)\]", str(i))
                self.assertTrue(m, msg=str(i))
                self.assertEqual(m.group(1), "{:g}".format(i.min))
                self.assertEqual(m.group(2), "{:g}".format(i.max))


class IndexIntervalTestCase(unittest.TestCase, IntervalTestMixin):
    IntervalClass = IndexInterval

    def setUp(self):
        self.points = [Index(-2), Index(4), Index(7), Index(11)]
        self.a = self.points[0]
        self.b = self.points[1]
        self.intervals = TestIntervals(self.IntervalClass, self.points)
        self.ab = self.IntervalClass(min=self.a, max=self.b)
        self.ba = self.IntervalClass(min=self.b, max=self.a)

    def testTypes(self):
        self.assertIs(self.IntervalClass.Scalar, Index)

    def testCenter(self):
        self.assertEqual(self.IntervalClass(min=2, max=4).center, 3)
        self.assertEqual(self.IntervalClass(min=1, max=4).center, 3)

    def testStr(self):
        for i in self.intervals.all:
            if i.isEmpty():
                self.assertEqual(str(i), "[]")
            else:
                m = re.match(r"\[(\-?\d+), (\-?\d+)\]", str(i))
                self.assertTrue(m, msg=str(i))
                self.assertEqual(m.group(1), "{:d}".format(i.min))
                self.assertEqual(m.group(2), "{:d}".format(i.max))

    def testExtensions(self):
        s = list(range(10))
        i = IndexInterval(min=3, max=8)
        self.assertEqual(s[i.slice], list(i))
        self.assertEqual(len(i), i.size)
        np.testing.assert_array_equal(np.array(list(i), dtype=i.Scalar), i.arange())
        np.testing.assert_array_equal(np.array(list(i), dtype=np.int64), i.arange(dtype=np.int64))


if __name__ == "__main__":
    unittest.main()
