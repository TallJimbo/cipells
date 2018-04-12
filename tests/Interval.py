import unittest
import itertools
import re
import numpy as np

from cipells import RealInterval, IndexInterval, Real, Index


class IntervalTestMixin:

    def assertAllTrue(self, iterable):
        seq = list(iterable)
        self.assertEqual(seq, [True]*len(seq))

    def assertAllFalse(self, iterable):
        seq = list(iterable)
        self.assertEqual(seq, [False]*len(seq))

    def makeIntervals(self):
        self.finite = []
        self.singular = []
        self.empty = []
        for i, lhs in enumerate(self.points):
            for j, rhs in enumerate(self.points):
                if i < j:
                    self.finite.append(self.IntervalClass(min=lhs, max=rhs))
                elif i > j:
                    self.empty.append(self.IntervalClass(min=lhs, max=rhs))
                else:
                    self.singular.append(self.IntervalClass(min=lhs, max=rhs))
        self.ab = self.finite[0]
        self.ba = self.empty[0]

    def testAccessors(self):
        self.assertEqual(self.ab.min, self.a)
        self.assertEqual(self.ab.max, self.b)

    def testEmpty(self):
        self.assertTrue(self.IntervalClass().isEmpty())
        self.assertTrue(self.IntervalClass.makeEmpty().isEmpty())
        self.assertAllFalse(s.isEmpty() for s in self.finite)
        self.assertAllFalse(s.isEmpty() for s in self.singular)
        self.assertAllTrue(s.isEmpty() for s in self.empty)

    def testContains(self):
        for lhs in itertools.chain(self.finite, self.singular):
            for rhs in itertools.chain(self.finite, self.singular):
                self.assertEqual(lhs.contains(rhs), lhs.min <= rhs.min and lhs.max >= rhs.max)
            for rhs in self.empty:
                self.assertTrue(lhs.contains(rhs))
            for rhs in self.points:
                self.assertEqual(lhs.contains(rhs), lhs.min <= rhs and lhs.max >= rhs)
        for lhs in self.empty:
            for rhs in itertools.chain(self.finite, self.singular):
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.empty:
                self.assertTrue(lhs.contains(rhs))

    def testIntersects(self):
        for lhs in itertools.chain(self.finite, self.singular):
            for rhs in itertools.chain(self.finite, self.singular):
                self.assertEqual(lhs.intersects(rhs),
                                 (lhs.min <= rhs.max and lhs.max >= rhs.min) or
                                 (rhs.min <= lhs.max and rhs.max >= lhs.min) or
                                 lhs.contains(rhs) or rhs.contains(lhs))
            for rhs in self.empty:
                self.assertFalse(lhs.intersects(rhs))
        for lhs in self.empty:
            for rhs in itertools.chain(self.finite, self.singular, self.empty):
                self.assertFalse(lhs.intersects(rhs))

    def testEquality(self):
        for lhs in itertools.chain(self.finite, self.singular, self.empty):
            for rhs in itertools.chain(self.finite, self.singular, self.empty):
                shouldBeEqual = lhs.contains(rhs) and rhs.contains(lhs)
                self.assertIs(lhs == rhs, shouldBeEqual)
                self.assertIs(lhs != rhs, not shouldBeEqual)

    def testClippedTo(self):
        for lhs in itertools.chain(self.finite, self.singular, self.empty):
            for rhs in itertools.chain(self.finite, self.singular, self.empty):
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
        for lhs in itertools.chain(self.finite, self.singular, self.empty):
            for rhs in itertools.chain(self.finite, self.singular, self.empty):
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
                                 lhs.expandedTo(self.IntervalClass(rhs, rhs)))

    def testDilatedBy(self):
        for lhs in itertools.chain(self.finite, self.singular):
            for rhs in self.points:
                dilated = lhs.dilatedBy(rhs)
                if dilated.isEmpty():
                    self.assertGreater(lhs.min - rhs, lhs.max + rhs)
                else:
                    self.assertEqual(lhs.min - rhs, dilated.min)
                    self.assertEqual(lhs.max + rhs, dilated.max)
        for lhs in self.empty:
            for rhs in self.points:
                self.assertTrue(lhs.dilatedBy(rhs).isEmpty())

    def testErodedBy(self):
        for lhs in itertools.chain(self.finite, self.singular, self.empty):
            for rhs in self.points:
                self.assertEqual(lhs.erodedBy(rhs), lhs.dilatedBy(-rhs))

    def testRepr(self):
        for i in itertools.chain(self.finite, self.singular, self.empty):
            self.assertEqual(eval(repr(i)), i)


class RealIntervalTestCase(unittest.TestCase, IntervalTestMixin):
    IntervalClass = RealInterval

    def setUp(self):
        self.points = [Real(-1.5), Real(5.0), Real(6.8), Real(8.2)]
        self.a = self.points[0]
        self.b = self.points[1]
        self.makeIntervals()

    def testTypes(self):
        self.assertIs(self.IntervalClass.Scalar, Real)
        self.assertIs(self.IntervalClass.Scalar, Real)

    def testStr(self):
        for i in itertools.chain(self.finite, self.singular, self.empty):
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
        self.makeIntervals()

    def testTypes(self):
        self.assertIs(self.IntervalClass.Scalar, Index)

    def testStr(self):
        for i in itertools.chain(self.finite, self.singular, self.empty):
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
