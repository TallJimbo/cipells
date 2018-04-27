import unittest
import re
import numpy as np

from cipells import Real2, Index2, Real, Index


class XYTupleTestMixin:

    def testAccessors(self):
        p = self.TupleClass(x=5, y=6)
        self.assertEqual(p.x, 5)
        self.assertEqual(p.y, 6)
        with self.assertRaises(AttributeError):
            p.x = 4
        with self.assertRaises(AttributeError):
            p.y = 3
        self.assertEqual(p[0], 5)
        self.assertEqual(p[1], 6)
        self.assertEqual(p[-2], 5)
        self.assertEqual(p[-1], 6)
        with self.assertRaises(IndexError):
            z = p[3]
        with self.assertRaises(IndexError):
            z = p[-3]
        with self.assertRaises(TypeError):
            p[0] = 3

    def testVector(self):
        p = self.TupleClass(x=5, y=6)
        v = p.vector
        self.assertEqual(v[0], p.x)
        self.assertEqual(v[1], p.y)
        self.assertEqual(v.dtype, np.dtype(p.Scalar))
        self.assertFalse(v.flags["WRITEABLE"])
        r = self.TupleClass(v)
        self.assertEqual(p, r)

    def testUnaryOps(self):
        p = self.TupleClass(x=5, y=6)
        r = +p
        self.assertIs(r, p)
        s = -p
        self.assertEqual(p.x, -s.x)
        self.assertEqual(p.y, -s.y)

    def testBinaryOps(self):
        p = self.TupleClass(x=5, y=6)
        r = self.TupleClass(x=2, y=3)
        self.assertEqual(p + r, self.TupleClass(7, 9))
        self.assertEqual(p - r, self.TupleClass(3, 3))
        self.assertEqual(p * 2, self.TupleClass(10, 12))
        s = p
        s += r   # XYTuple is immutable, so this must not modify p
        self.assertEqual(s, p + r)
        s = p
        s -= r
        self.assertEqual(s, p - r)
        s = p
        s *= 2
        self.assertEqual(s, p * 2)

    def testComparison(self):
        p = self.TupleClass(x=5, y=6)
        self.assertEqual(p, self.TupleClass(5, 6))
        self.assertEqual(p, (5, 6))
        self.assertNotEqual(p, self.TupleClass(6, 6))
        self.assertNotEqual(p, self.TupleClass(5, 7))

    def testDot(self):
        p = self.TupleClass(x=5, y=6)
        r = self.TupleClass(x=2, y=3)
        self.assertEqual(p.dot(r), p.x*r.x + p.y*r.y)
        self.assertEqual(p.dot((1, 2)), p.x*1 + p.y*2)

    def testFormat(self):
        p = self.TupleClass(x=5, y=6)
        self.assertEqual(str(p), "{:s}".format(p))
        self.assertEqual(str(p), "{}".format(p))
        self.assertEqual(repr(p), "{:r}".format(p))


class Real2TestCase(unittest.TestCase, XYTupleTestMixin):
    TupleClass = Real2

    def testTypes(self):
        self.assertIs(self.TupleClass.Scalar, Real)

    def testNumPyScalars(self):
        a = np.arange(2, dtype=np.float32)
        p = Real2(*a)
        self.assertEqual(p, tuple(a))

    def testDivision(self):
        p = Real2(8.0, -4.0)
        self.assertEqual(p / 2.0, Real2(p.x/2.0, p.y/2.0))
        self.assertEqual(p / 3.0, Real2(p.x/3.0, p.y/3.0))
        self.assertEqual(p / -3.0, Real2(p.x/-3.0, p.y/-3.0))

    def testStr(self):
        p = Real2(8.0, -4.0)
        self.assertEqual(eval(repr(p)), p)
        self.assertTrue(re.match(r"\(x=8(\,0*)?, y=-4(\,0*)?\)", str(p)))


class Index2TestCase(unittest.TestCase, XYTupleTestMixin):
    TupleClass = Index2

    def testTypes(self):
        self.assertIs(self.TupleClass.Scalar, Index)

    def testNumPyScalars(self):
        a = np.arange(2, dtype=np.uint16)
        p = Index2(*a)
        self.assertEqual(p, tuple(a))

    def testDivision(self):
        p = Index2(8, -4)
        self.assertEqual(p // 2, Index2(p.x//2, p.y//2))
        self.assertEqual(p // 3, Index2(p.x//3, p.y//3))
        self.assertEqual(p // -3, Index2(p.x//-3, p.y//-3))
        self.assertEqual(p % 3, Index2(p.x%3, p.y%3))
        self.assertEqual(p % -3, Index2(p.x%-3, p.y%-3))

    def testStr(self):
        p = Index2(8, -4)
        self.assertEqual(eval(repr(p)), p)
        self.assertEqual(str(p), "(x=8, y=-4)")


if __name__ == "__main__":
    unittest.main()
