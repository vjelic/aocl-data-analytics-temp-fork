# Copyright (C) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# pylint: disable = missing-module-docstring,missing-function-docstring,invalid-name
"""
Linear models Python test script
"""

import numpy as np
import pytest
from aoclda.linear_model import linmod


@pytest.mark.parametrize("numpy_precision", [np.float64,  np.float32])
@pytest.mark.parametrize("numpy_order", ["C", "F"])
def test_linear_regression(numpy_precision, numpy_order):
    X = np.array([[1, 1], [2, 3], [3, 5], [4, 8], [5, 7], [6, 9]],
                 dtype=numpy_precision, order=numpy_order)
    y = np.array([3., 6.5, 10., 12., 13., 19.], dtype=numpy_precision)
    tol = np.sqrt(np.finfo(numpy_precision).eps)

    # compute linear regression without intercept
    lmod = linmod("mse")
    lmod.fit(X, y)

    # check expected results
    expected_coef = np.array([2.45, 0.35], dtype=numpy_precision)
    norm = np.linalg.norm(np.abs(lmod.coef) - np.abs(expected_coef))
    assert norm < tol

    # same test with intercept
    lmod = linmod("mse", intercept=True)
    lmod.fit(X, y)

    # check expected results
    expected_coef = np.array(
        [2.35, 0.35, 0.43333333333333535], dtype=numpy_precision)
    norm = np.linalg.norm(np.abs(lmod.coef) - np.abs(expected_coef))
    assert norm < tol


@pytest.mark.parametrize("numpy_precision", [np.float64,  np.float32])
@pytest.mark.parametrize("numpy_order", ["C", "F"])
def test_linear_regression_error_exits(numpy_precision, numpy_order):
    X = np.array([[1, 1], [2, 3], [3, 5], [4, 8], [5, 7], [6, 9]],
                 dtype=numpy_precision, order=numpy_order)
    y = np.array([3., 6.5, 10., 12., 13., 19.], dtype=numpy_precision)

    # lambda out of bounds
    with pytest.raises(RuntimeError):
        lmod = linmod("mse", reg_lambda=-1)
        lmod.fit(X, y)

    # alpha out of bounds
    with pytest.raises(RuntimeError):
        lmod = linmod("mse", reg_alpha=-1)
        lmod.fit(X, y)
    with pytest.raises(RuntimeError):
        lmod = linmod("mse", reg_alpha=1.1)
        lmod.fit(X, y)

    # max_iter out of bounds
    with pytest.raises(RuntimeError):
        lmod = linmod("mse", max_iter=-1)
        lmod.fit(X, y)

    # solving lasso with cholesky
    with pytest.raises(RuntimeError):
        lmod = linmod("mse", solver='cholesky', reg_alpha=1, reg_lambda=1)
        lmod.fit(X, y)

    # solving ridge with qr
    with pytest.raises(RuntimeError):
        lmod = linmod("mse", solver='qr', reg_alpha=0, reg_lambda=1)
        lmod.fit(X, y)

    # NaN checking
    X = np.array([[1, 1], [2, 3], [3, 5], [4, 8], [5, 7], [6, 9]],
                 dtype=numpy_precision, order=numpy_order)
    y = np.array([3., 6.5, 10., 12., np.nan, 19.], dtype=numpy_precision)
    lmod2 = linmod("mse", check_data=True)
    with pytest.raises(RuntimeError):
        lmod2.fit(X, y)
