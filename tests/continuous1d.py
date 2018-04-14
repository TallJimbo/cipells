import unittest
import numpy as np
from cipells.experiments.continuous1d import AffineTransform


class TestCase(unittest.TestCase):

    def testAffineTransform(self):
        scaling = np.random.rand()
        offset = np.random.rand()
        t1 = AffineTransform(scaling=scaling, offset=offset)
        self.assertEqual(t1.scaling, scaling)
        self.assertEqual(t1.offset, offset)
        x1 = np.random.rand()
        y1 = t1(x1)
        self.assertEqual(scaling*x1 + offset, y1)
        x2 = np.random.rand(5)
        y2 = t1(x2)
        np.testing.assert_allclose(scaling*x2 + offset, y2, rtol=1E-13)
        t2 = t1.inverse
        np.testing.assert_allclose(t2(y2), x2, rtol=1E-13)
        identity1 = t2.then(t1)
        np.testing.assert_allclose(identity1.scaling, 1.0)
        np.testing.assert_allclose(identity1.offset, 0.0, atol=1E-13)
        identity2 = t1.then(t2)
        np.testing.assert_allclose(identity2.scaling, 1.0)
        np.testing.assert_allclose(identity2.offset, 0.0, atol=1E-13)
        scaling3 = np.random.randn()
        offset3 = np.random.randn()
        t3 = AffineTransform(scaling3, offset3)
        t4 = t1.then(t3)
        np.testing.assert_allclose(t4(x2), t3(t1(x2)))

if __name__ == "__main__":
    unittest.main()
