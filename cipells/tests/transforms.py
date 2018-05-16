import unittest
import itertools
import numpy as np

from cipells import Real, Real2, Identity, Translation, Jacobian, Affine, RealBox

from cipells.tests import acceptJacobian, acceptTranslation, acceptAffine
from .Box import TestBoxes


class TestTransforms:

    def __init__(self, rng, n=2):
        self.translations = [Translation(rng.randn(2))
                             for i in range(n)]
        self.jacobians = [Jacobian(rng.randn(2, 2))]
        self.affine = [Affine(rng.randn(2, 2), rng.randn(2))]

    @property
    def all(self):
        return itertools.chain([Identity()], self.translations, self.jacobians, self.affine)


class TransformTestCase(unittest.TestCase):

    def setUp(self):
        self.identity = np.array([[1, 0], [0, 1]], dtype=Real)
        self.zero = np.array([0, 0], dtype=Real)

    def assertTransformsClose(self, a, b):
        np.testing.assert_allclose(a.matrix, b.matrix, atol=1E-14, rtol=1E-14)
        np.testing.assert_allclose(a.vector, b.vector, atol=1E-14, rtol=1E-14)

    def testTranslationConstructors(self):
        t0 = Translation()
        np.testing.assert_array_equal(t0.matrix, self.identity)
        np.testing.assert_array_equal(t0.vector, self.zero)
        t1 = Translation(np.array([4, -3], dtype=Real))
        np.testing.assert_array_equal(t1.matrix, self.identity)
        np.testing.assert_array_equal(t1.vector, np.array([4, -3], dtype=Real))
        t2 = Translation(Real2(-2, 5))
        np.testing.assert_array_equal(t2.matrix, self.identity)
        np.testing.assert_array_equal(t2.vector, np.array([-2, 5], dtype=Real))
        t3 = Translation(Identity())
        np.testing.assert_array_equal(t3.matrix, self.identity)
        np.testing.assert_array_equal(t3.vector, self.zero)

    def testJacobianConstructors(self):
        j0 = Jacobian()
        np.testing.assert_array_equal(j0.matrix, self.identity)
        np.testing.assert_array_equal(j0.vector, self.zero)
        m = np.array([[4, 2], [-1, 5]], dtype=Real)
        j1 = Jacobian(m)
        np.testing.assert_array_equal(j1.matrix, m)
        np.testing.assert_array_equal(j1.vector, self.zero)
        j2 = Jacobian(Identity())
        np.testing.assert_array_equal(j2.matrix, self.identity)
        np.testing.assert_array_equal(j2.vector, self.zero)

    def testAffineConstructors(self):
        a0 = Affine()
        np.testing.assert_array_equal(a0.matrix, self.identity)
        np.testing.assert_array_equal(a0.vector, self.zero)
        m = np.array([[4, 2], [-1, 5]], dtype=Real)
        a1 = Affine(m, self.zero)
        np.testing.assert_array_equal(a1.matrix, m)
        np.testing.assert_array_equal(a1.vector, self.zero)
        v = np.array([-2, 1], dtype=Real)
        a2 = Affine(self.identity, v)
        np.testing.assert_array_equal(a2.matrix, self.identity)
        np.testing.assert_array_equal(a2.vector, v)
        a3 = Affine(m, v)
        np.testing.assert_array_equal(a3.matrix, m)
        np.testing.assert_array_equal(a3.vector, v)
        a4 = Affine(Jacobian(m), Translation(v))
        np.testing.assert_array_equal(a4.matrix, m)
        np.testing.assert_array_equal(a4.vector, v)
        a5 = Affine(Jacobian(m))
        np.testing.assert_array_equal(a5.matrix, m)
        np.testing.assert_array_equal(a5.vector, self.zero)
        a6 = Affine(Translation(v))
        np.testing.assert_array_equal(a6.matrix, self.identity)
        np.testing.assert_array_equal(a6.vector, v)
        a7 = Affine(Identity())
        np.testing.assert_array_equal(a7.matrix, self.identity)
        np.testing.assert_array_equal(a7.vector, self.zero)

    def testInverted(self):
        transforms = TestTransforms(np.random)
        for t in transforms.all:
            if t.det <= 0.0:
                continue
            i = t.inverted()
            self.assertIs(type(t), type(i))
            np.testing.assert_allclose(np.linalg.inv(t.matrix), i.matrix)
            self.assertTransformsClose(t.then(i), Identity())
            self.assertTransformsClose(i.then(t), Identity())
            p1 = Real2(np.random.randn(2))
            p2 = t(p1)
            p3 = i(p2)
            p4 = t(p3)
            np.testing.assert_allclose(p1.vector, p3.vector)
            np.testing.assert_allclose(p2.vector, p4.vector)

    def testComposition(self):
        transforms = TestTransforms(np.random)
        for t1 in transforms.all:
            for t2 in transforms.all:
                c = t1.then(t2)
                p1 = Real2(np.random.randn(2))
                p2 = t2(t1(p1))
                p3 = c(p1)
                np.testing.assert_allclose(p2.vector, p3.vector)
                # Test that result type is as specific as possible
                # (depends on inputs being as specific as possible, of course)
                if np.all(c.matrix == self.identity):
                    self.assertIsInstance(c, (Translation, Identity))
                if np.all(c.vector == self.zero):
                    self.assertIsInstance(c, (Jacobian, Identity))

    def testDeterminant(self):
        transforms = TestTransforms(np.random)
        for t in transforms.all:
            np.testing.assert_allclose(np.linalg.det(t.matrix), t.det)

    def testFormat(self):
        transforms = TestTransforms(np.random)
        for t in transforms.all:
            self.assertTransformsClose(eval(repr(t)), t)

    def testBoxes(self):
        points = [Real(-1.5), Real(5.0), Real(6.75), Real(8.625)]
        transforms = TestTransforms(np.random)
        boxes = TestBoxes(RealBox, points, n=3)
        for t in transforms.all:
            for b in boxes.finite:
                b2 = t(b)
                for p in b.corners:
                    p2 = t(p)
                    self.assertTrue(b2.contains(p2))

    def testImplicitConversion(self):
        transforms = TestTransforms(np.random)
        self.assertTrue(acceptJacobian(Identity()))
        self.assertTrue(acceptTranslation(Identity()))
        self.assertTrue(acceptAffine(Identity()))
        for jacobian in transforms.jacobians:
            self.assertTrue(acceptAffine(jacobian))
        for translation in transforms.translations:
            self.assertTrue(acceptAffine(translation))


if __name__ == "__main__":
    unittest.main()
