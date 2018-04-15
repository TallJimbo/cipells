import unittest
import itertools
import numpy as np

from cipells import (RealBox, IndexBox, IndexInterval, RealInterval, Real, Index)

from .Interval import TestIntervals


class TestBoxes:

    def __init__(self, BoxClass, points):
        self.intervals = TestIntervals(BoxClass.Interval, points)
        self.finite = list(BoxClass(x=x, y=y)
                           for x in self.intervals.finite
                           for y in self.intervals.finite)
        self.xEmpty = list(BoxClass(x=x, y=y)
                           for x in self.intervals.empty
                           for y in self.intervals.finite)
        self.yEmpty = list(BoxClass(x=x, y=y)
                           for x in self.intervals.finite
                           for y in self.intervals.empty)
        self.xyEmpty = list(BoxClass(x=x, y=y)
                            for x in self.intervals.empty
                            for y in self.intervals.empty)

    @property
    def empty(self):
        return itertools.chain(self.xEmpty, self.yEmpty, self.xyEmpty)

    @property
    def all(self):
        return itertools.chain(self.finite, self.xEmpty, self.yEmpty, self.xyEmpty)


class BoxTestMixin:

    def assertAllTrue(self, iterable):
        seq = list(iterable)
        self.assertEqual(seq, [True]*len(seq))

    def assertAllFalse(self, iterable):
        seq = list(iterable)
        self.assertEqual(seq, [False]*len(seq))

    def testAccessors(self):
        for box in self.boxes.finite:
            self.assertEqual(box.min, (box.x0, box.y0))
            self.assertEqual(box.max, (box.x1, box.y1))
            self.assertEqual(box.x, self.BoxClass.Interval(min=box.x0, max=box.x1))
            self.assertEqual(box.y, self.BoxClass.Interval(min=box.y0, max=box.y1))
            self.assertEqual(box.x.min, box.min.x)
            self.assertEqual(box.y.min, box.min.y)
            self.assertEqual(box.x.max, box.max.x)
            self.assertEqual(box.y.max, box.max.y)
            self.assertEqual(box.size, (box.width, box.height))

    def testEmpty(self):
        self.assertTrue(self.BoxClass().isEmpty())
        self.assertTrue(self.BoxClass.makeEmpty().isEmpty())
        self.assertAllFalse(s.isEmpty() for s in self.boxes.finite)
        self.assertAllTrue(s.isEmpty() for s in self.boxes.empty)

    def testConstructor(self):
        for box in self.boxes.finite:
            self.assertEqual(box, self.BoxClass(min=box.min, max=box.max))
            self.assertEqual(box, self.BoxClass(min=box.min, size=box.size))
            self.assertEqual(box, self.BoxClass(max=box.max, size=box.size))
            self.assertEqual(box, self.BoxClass(center=box.center, size=box.size))

    def testMakeHull(self):
        for n0x, p0x in enumerate(self.points):
            for n1x, p1x in enumerate(self.points):
                if n0x >= n1x:
                    continue
                for n0y, p0y in enumerate(self.points):
                    for n1y, p1y in enumerate(self.points):
                        if n0y >= n1y:
                            continue
                        sx = list(self.points[n0x:n1x + 1])
                        sy = list(self.points[n0y:n1y + 1])
                        b = self.BoxClass(min=(p0x, p0y), max=(p1x, p1y))
                        tuples = list(itertools.product(sx, sy))
                        self.assertEqual(b, self.BoxClass.makeHull(tuples), msg=tuples)
                        np.random.shuffle(sx)
                        np.random.shuffle(sy)
                        tuples = list(itertools.product(sx, sy))
                        self.assertEqual(b, self.BoxClass.makeHull(tuples), msg=tuples)
                        sx.reverse()
                        sy.reverse()
                        tuples = list(itertools.product(sx, sy))
                        self.assertEqual(b, self.BoxClass.makeHull(tuples), msg=tuples)

    def testContains(self):
        for lhs in self.boxes.finite:
            for rhs in self.boxes.finite:
                self.assertEqual(lhs.contains(rhs), lhs.x.contains(rhs.x) and lhs.y.contains(rhs.y))
            for rhs in self.boxes.xEmpty:
                self.assertEqual(lhs.contains(rhs), lhs.y.contains(rhs.y))
            for rhs in self.boxes.yEmpty:
                self.assertEqual(lhs.contains(rhs), lhs.x.contains(rhs.x))
            for rhs in self.boxes.xyEmpty:
                self.assertTrue(lhs.contains(rhs))
            for x, y in itertools.product(self.points, repeat=2):
                self.assertEqual(lhs.contains((x, y)), lhs.x.contains(x) and lhs.y.contains(y))
        for lhs in self.boxes.xEmpty:
            for rhs in self.boxes.finite:
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.boxes.xEmpty:
                self.assertIs(lhs.contains(rhs), lhs.y.contains(rhs.y))
            for rhs in self.boxes.yEmpty:
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.boxes.xyEmpty:
                self.assertTrue(lhs.contains(rhs))
            for rhs in itertools.product(self.points, repeat=2):
                self.assertFalse(lhs.contains(rhs))
        for lhs in self.boxes.yEmpty:
            for rhs in self.boxes.finite:
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.boxes.xEmpty:
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.boxes.yEmpty:
                self.assertIs(lhs.contains(rhs), lhs.x.contains(rhs.x))
            for rhs in self.boxes.xyEmpty:
                self.assertTrue(lhs.contains(rhs))
            for rhs in itertools.product(self.points, repeat=2):
                self.assertFalse(lhs.contains(rhs))
        for lhs in self.boxes.xyEmpty:
            for rhs in self.boxes.finite:
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.boxes.xEmpty:
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.boxes.yEmpty:
                self.assertFalse(lhs.contains(rhs))
            for rhs in self.boxes.xyEmpty:
                self.assertTrue(lhs.contains(rhs))
            for rhs in itertools.product(self.points, repeat=2):
                self.assertFalse(lhs.contains(rhs))

    def testIntersects(self):
        pass

    def testEquality(self):
        for lhs in self.boxes.all:
            for rhs in self.boxes.all:
                shouldBeEqual = lhs.contains(rhs) and rhs.contains(lhs)
                self.assertIs(lhs == rhs, shouldBeEqual)
                self.assertIs(lhs != rhs, not shouldBeEqual)

    def testClippedTo(self):
        for lhs in self.boxes.all:
            for rhs in self.boxes.all:
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
        for lhs in self.boxes.all:
            for rhs in self.boxes.all:
                expanded = lhs.expandedTo(rhs)
                self.assertTrue(expanded.contains(lhs))
                self.assertTrue(expanded.contains(rhs))
                self.assertIs(
                    expanded.isEmpty(),
                    (lhs.x.isEmpty() and rhs.x.isEmpty()) or
                    (lhs.y.isEmpty() and rhs.y.isEmpty())
                )
                self.assertIs(expanded == rhs, rhs.contains(lhs))
                self.assertIs(expanded == lhs, lhs.contains(rhs))
            for rhs in itertools.product(self.points, repeat=2):
                self.assertEqual(lhs.expandedTo(rhs),
                                 lhs.expandedTo(self.BoxClass(min=rhs, max=rhs)))

    def testDilatedBy(self):
        for lhs in self.boxes.finite:
            for rhs in itertools.product(self.points, repeat=2):
                dilated = lhs.dilatedBy(rhs)
                if dilated.isEmpty():
                    self.assertTrue(lhs.min.x - rhs[0] > lhs.max.x + rhs[0] or
                                    lhs.min.y - rhs[1] > lhs.max.y + rhs[1])
                else:
                    self.assertEqual(lhs.min - rhs, dilated.min)
                    self.assertEqual(lhs.max + rhs, dilated.max)
        for lhs in self.boxes.empty:
            for rhs in itertools.product(self.points, repeat=2):
                self.assertTrue(lhs.dilatedBy(rhs).isEmpty())

    def testErodedBy(self):
        for lhs in self.boxes.all:
            for x, y in itertools.product(self.points, repeat=2):
                self.assertEqual(lhs.erodedBy((x, y)), lhs.dilatedBy((-x, -y)))

    def testRepr(self):
        for i in self.boxes.all:
            self.assertEqual(eval(repr(i)), i)

    def testStr(self):
        for b in self.boxes.xEmpty:
            self.assertEqual(str(b), "[]×{}".format(b.y))
        for b in self.boxes.yEmpty:
            self.assertEqual(str(b), "{}×[]".format(b.x))
        for b in self.boxes.xyEmpty:
            self.assertEqual(str(b), "[]×[]")
        for b in self.boxes.finite:
            self.assertEqual(str(b), "{}×{}".format(b.x, b.y))


class RealBoxTestCase(unittest.TestCase, BoxTestMixin):
    BoxClass = RealBox

    def setUp(self):
        self.points = [Real(-1.5), Real(5.0), Real(6.75), Real(8.625)]
        self.boxes = TestBoxes(self.BoxClass, self.points)

    def testTypes(self):
        self.assertIs(self.BoxClass.Scalar, Real)
        self.assertIs(self.BoxClass.Interval, RealInterval)

    def testCenter(self):
        for box in self.boxes.finite:
            self.assertEqual(box.center, 0.5*(box.min + box.max))


class IndexBoxTestCase(unittest.TestCase, BoxTestMixin):
    BoxClass = IndexBox

    def setUp(self):
        self.points = [Index(-2), Index(4), Index(7), Index(11)]
        self.boxes = TestBoxes(self.BoxClass, self.points)

    def testTypes(self):
        self.assertIs(self.BoxClass.Scalar, Index)
        self.assertIs(self.BoxClass.Interval, IndexInterval)

    def testCenter(self):
        self.assertEqual(self.BoxClass(min=(2, 0), max=(4, 2)).center, (3, 1))
        self.assertEqual(self.BoxClass(min=(1, 0), max=(4, 2)).center, (3, 1))
        self.assertEqual(self.BoxClass(min=(0, 2), max=(2, 4)).center, (1, 3))
        self.assertEqual(self.BoxClass(min=(0, 1), max=(2, 4)).center, (1, 3))


if __name__ == "__main__":
    unittest.main()
