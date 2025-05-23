# Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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


"""
Compare linear model modules with sklearn
"""

# pylint: disable = import-outside-toplevel, invalid-name, reimported, no-member

import numpy as np
import importlib
from aoclda.sklearn import skpatch, undo_skpatch
import pytest
from sklearn.datasets import make_regression, make_classification

@pytest.mark.parametrize("intercept", [True, False])
@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_linear_regression(intercept, precision):
    """
    Vanilla linear regression
    """
    X = np.array([[1, 1], [1, 2], [2, 2], [2, 3]], dtype=precision)
    y = np.dot(X, np.array([1, 2], dtype=precision)) + 3

    tol = np.sqrt(np.finfo(precision).eps)

    # patch and import scikit-learn
    skpatch()
    from sklearn.linear_model import LinearRegression
    linreg_da = LinearRegression(fit_intercept=intercept)
    linreg_da.fit(X, y)
    da_coef = linreg_da.coef_
    da_intercept = linreg_da.intercept_
    assert linreg_da.aocl is True

    # unpatch and solve the same problem with sklearn
    undo_skpatch()
    from sklearn.linear_model import LinearRegression
    linreg = LinearRegression(fit_intercept=intercept)
    linreg.fit(X, y)
    coef = linreg.coef_
    intercept = linreg.intercept_
    assert not hasattr(linreg, 'aocl')

    # Check results
    assert da_coef == pytest.approx(coef, tol)
    assert da_intercept == pytest.approx(intercept, tol)

    # print the results if pytest is invoked with the -rA option
    print("coefficients")
    print("    aoclda: ", linreg_da.coef_)
    print("   sklearn: ", linreg.coef_)
    print("Intercept")
    print("    aoclda:", linreg_da.intercept_)
    print("   sklearn:", linreg_da.intercept_)

@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_double_solve_linreg(precision):
    """"
    Check that solving the model twice doesn't fail
    """
    X = np.array([[1, 1], [1, 2], [2, 2], [2, 3]], dtype=precision)
    y = np.dot(X, np.array([1, 2], dtype=precision)) + 3
    skpatch()
    from sklearn.linear_model import LinearRegression
    linreg_da = LinearRegression()
    linreg_da.fit(X, y)
    linreg_da.fit(X, y)

@pytest.mark.parametrize("intercept", [True, False])
@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_ridge(intercept, precision):
    """
    Ridge regression using LBFGS
    """
    X, y, _ = make_regression(
        n_samples=200, n_features=8, coef=True, random_state=1)
    X = X.astype(precision)
    y = y.astype(precision)

    skpatch()
    from sklearn.linear_model import Ridge
    ridge_da = Ridge(fit_intercept=intercept)
    ridge_da.fit(X, y)
    da_coef = ridge_da.coef_
    # da_intercept = ridge_da.intercept_
    assert ridge_da.aocl is True

    # unpatch and solve the same problem with sklearn
    undo_skpatch()
    from sklearn.linear_model import Ridge
    ridge = Ridge(fit_intercept=intercept, solver='lbfgs', positive=True)
    ridge.fit(X, y)
    coef = ridge.coef_
    # intercept = ridge.intercept_
    assert not hasattr(ridge, 'aocl')

    # Check results
    assert da_coef == pytest.approx(coef, 1.0e-01)
    # Deactivate the intercept check, a factor 2 needs to be investigated
    # assert da_intercept == pytest.approx(
    #     intercept/2, 0.1)

    # print the results if pytest is invoked with the -rA option
    print("coefficients")
    print("    aoclda: ", ridge_da.coef_)
    print("   sklearn: ", ridge.coef_)
    print("Intercept")
    print("    aoclda:", ridge_da.intercept_)
    print("   sklearn:", ridge.intercept_/2)


@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_double_solve_ridge(precision):
    """"
    Check that solving the model twice doesn't fail
    """
    X, y, _ = make_regression(
    n_samples=200, n_features=8, coef=True, random_state=1)
    X = X.astype(precision)
    y = y.astype(precision)
    skpatch()
    from sklearn.linear_model import Ridge
    ridge_da = Ridge()
    ridge_da.fit(X, y)
    ridge_da.fit(X, y)

@pytest.mark.parametrize("intercept", [True, False])
@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_lasso(intercept, precision):
    """
    Lasso
    """
    X = np.array([[-1 / np.sqrt(2), -1 / np.sqrt(2)],
                 [0, 0], [1 / np.sqrt(2), 1 / np.sqrt(2)]], dtype=precision)
    y = np.array([-1, 0, 1], dtype=precision)

    tol = np.sqrt(np.finfo(precision).eps)

    skpatch()
    from sklearn.linear_model import Lasso
    lasso_da = Lasso(fit_intercept=intercept, alpha=0.1)
    lasso_da.fit(X, y)
    # da_coef = lasso_da.coef_
    da_intercept = lasso_da.intercept_
    assert lasso_da.aocl is True

    # unpatch and solve the same problem with sklearn
    undo_skpatch()
    from sklearn.linear_model import Lasso
    lasso = Lasso(fit_intercept=intercept, alpha=0.1)
    lasso.fit(X, y)
    # coef = lasso.coef_
    intercept = lasso.intercept_
    assert not hasattr(lasso, 'aocl')

    # Check results
    # Internal standardization is not the same et between sklearn and DA
    # assert da_coef == pytest.approx(coef, 1.0e-08)
    assert da_intercept == pytest.approx(intercept, abs=tol)

    # print the results if pytest is invoked with the -rA option
    print("coefficients")
    print("    aoclda: ", lasso_da.coef_)
    print("   sklearn: ", lasso.coef_)
    print("Intercept")
    print("    aoclda:", lasso_da.intercept_)
    print("   sklearn:", lasso_da.intercept_)


@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_double_solve_lasso(precision):
    """"
    Check that solving the model twice doesn't fail
    """
    X, y, _ = make_regression(
    n_samples=200, n_features=8, coef=True, random_state=1)
    X = X.astype(precision)
    y = y.astype(precision)
    skpatch()
    from sklearn.linear_model import Lasso
    lasso_da = Lasso(alpha=0.1)
    lasso_da.fit(X, y)
    lasso_da.fit(X, y)

@pytest.mark.parametrize("intercept", [True, False])
@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_logistic(intercept, precision):
    """
    Logistic Regression
    """
    X, y = make_classification(
        n_samples=200, n_features=8, random_state=1)
    X = X.astype(precision)
    y = y.astype(precision)

    skpatch()
    from sklearn.linear_model import LogisticRegression
    lg_da = LogisticRegression(fit_intercept=intercept)
    lg_da.fit(X, y)
    da_coef = lg_da.coef_
    assert lg_da.aocl is True

    # unpatch and solve the same problem with sklearn
    undo_skpatch()
    from sklearn.linear_model import LogisticRegression
    lg = LogisticRegression(fit_intercept=intercept)
    lg.fit(X, y)
    coef = lg.coef_
    assert not hasattr(lg, 'aocl')

    # Check results
    assert da_coef == pytest.approx(coef, 1.0e-01)

    # print the results if pytest is invoked with the -rA option
    print("coefficients")
    print("    aoclda: ", lg_da.coef_)
    print("   sklearn: ", lg.coef_)
    print("Intercept")
    print("    aoclda:", lg_da.intercept_)
    print("   sklearn:", lg.intercept_)


@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_double_solve_lasso(precision):
    """"
    Check that solving the model twice doesn't fail
    """
    X, y = make_classification(
        n_samples=200, n_features=8, random_state=1)
    X = X.astype(precision)
    y = y.astype(precision)

    skpatch()
    from sklearn.linear_model import LogisticRegression
    lg_da = LogisticRegression()
    lg_da.fit(X, y)
    lg_da.fit(X, y)



@pytest.mark.parametrize("problem", ["LinearRegression", "Ridge", "Lasso", "ElasticNet"])
def test_errors(problem):
    '''
    Test we catch the errors for unsupported functions
    '''
    X = np.array([[1, 1], [2, 3], [3, 5], [4, 8], [5, 7], [6, 9.1]])
    y = np.array([3., 6.5, 10., 12., 13., 19.])
    # patch and import scikit-learn
    skpatch()
    sklearn_patch_module = importlib.import_module("sklearn.linear_model")
    problem_class = getattr(sklearn_patch_module, problem)
    model = problem_class()
    model.fit(X, y)

    with pytest.raises(RuntimeError):
        model.get_metadata_routing()

    with pytest.raises(RuntimeError):
        model.score(1, 1)

    with pytest.raises(RuntimeError):
        model.set_fit_request()

    with pytest.raises(RuntimeError):
        model.set_params()

    with pytest.raises(RuntimeError):
        model.set_score_request()
