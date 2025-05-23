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
PCA tests, check output of skpatch versus sklearn
"""

# pylint: disable = import-outside-toplevel, reimported, no-member

import numpy as np
import pytest
from aoclda.sklearn import skpatch, undo_skpatch


@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_pca(precision):
    """
    Basic 3 x 2 problem
    """
    a = np.array([[1, 2, 3], [0.22, 5, 4.1], [3, 6, 1]], dtype=precision)
    b = np.array([[3, 2, 3], [1.22, 5, 4.1], [3, 3, 1]], dtype=precision)

    tol = np.sqrt(np.finfo(precision).eps)

    # patch and import scikit-learn
    skpatch()
    from sklearn.decomposition import PCA
    pca_da = PCA(n_components=3)
    pca_da = pca_da.fit(a)
    da_x = pca_da.transform(b)
    da_y = pca_da.fit_transform(a)
    da_explained_variance_ratio = pca_da.explained_variance_ratio_
    da_explained_variance = pca_da.explained_variance_
    da_noise_variance = pca_da.noise_variance_
    assert pca_da.aocl is True

    # unpatch and solve the same problem with sklearn
    undo_skpatch()
    from sklearn.decomposition import PCA
    pca = PCA(n_components=3)
    pca.fit(a)
    sk_x = pca_da.transform(b)
    sk_y = pca_da.fit_transform(a)
    sk_explained_variance_ratio = pca.explained_variance_ratio_
    sk_explained_variance = pca.explained_variance_
    sk_noise_variance = pca.noise_variance_
    assert not hasattr(pca, 'aocl')

    # Check results
    da_components = pca_da.components_
    components = pca.components_
    assert np.abs(da_components) == pytest.approx(np.abs(components), tol)
    da_mean = pca_da.mean_
    mean = pca.mean_
    assert da_mean == pytest.approx(mean, tol)
    da_singval = pca_da.singular_values_
    singval = pca.singular_values_
    assert da_singval == pytest.approx(singval, abs=tol)
    assert np.abs(da_x) == pytest.approx(np.abs(sk_x), abs=tol)
    assert np.abs(da_y) == pytest.approx(np.abs(sk_y), abs=tol)
    assert da_explained_variance_ratio == pytest.approx(
        sk_explained_variance_ratio, tol)
    assert da_explained_variance == pytest.approx(
        sk_explained_variance, tol)
    assert da_noise_variance == pytest.approx(sk_noise_variance, tol)
    assert pca_da.n_features_in_ == pca.n_features_in_
    assert pca_da.n_components_ == pca.n_components_
    assert pca_da.n_samples_ == pca.n_samples_

    # print the results if pytest is invoked with the -rA option
    print("Components")
    print("    aoclda: \n", da_components)
    print("   sklearn: \n", components)


@pytest.mark.parametrize("precision", [np.float64,  np.float32])
def test_double_solve(precision):
    """"
    Check that solving the model twice doesn't fail
    """
    a = np.array([[1, 2, 3], [0.22, 5, 4.1], [3, 6, 1]], dtype=precision)
    skpatch()
    from sklearn.decomposition import PCA
    pca_da = PCA(n_components=3)
    pca_da = pca_da.fit(a)
    pca_da = pca_da.fit(a)


def test_pca_errors():
    '''
    Check we can catch errors in the sklearn pca patch
    '''
    a = np.array([[1, 2, 3], [0.22, 5, 4.1], [3, 6, 1]])

    skpatch()
    from sklearn.decomposition import PCA
    with pytest.raises(ValueError):
        pca = PCA(n_components=0.5)

    with pytest.warns(RuntimeWarning):
        pca = PCA(n_components=3, n_oversamples=2)

    pca.fit(a)

    # Test unsupported functions
    with pytest.raises(RuntimeError):
        pca.get_covariance()

    with pytest.raises(RuntimeError):
        pca.get_precision()

    with pytest.raises(RuntimeError):
        pca.set_output()

    with pytest.raises(RuntimeError):
        pca.set_params()

    with pytest.raises(RuntimeError):
        pca.get_feature_names_out()

    with pytest.raises(RuntimeError):
        pca.get_metadata_routing()

    with pytest.raises(RuntimeError):
        pca.score(1)

    with pytest.raises(RuntimeError):
        pca.score_samples(1)

    assert pca.feature_names_in_ is None


if __name__ == "__main__":
    test_pca()
    test_pca_errors()
