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

"""
Patching scikit-learn metrics: pairwise_distances
"""
# pylint: disable = too-many-positional-arguments, invalid-name, unused-argument, too-many-arguments

import warnings
from aoclda.metrics import pairwise_distances as pairwise_distances_da

def pairwise_distances(X, Y=None, metric='euclidean', p=2.0, n_jobs=None,
    force_all_finite=True, **kwds):
    """
    Overwrite sklearn.metrics.pairwise_distances to call AOCL-DA library
    """

    # Check for unsupported attributes
    if force_all_finite is not False:
        warnings.warn(
            "The parameter force_all_finite is not supported and has been ignored.\
            No checks are performed on input data.",
            category=RuntimeWarning)

    if n_jobs is not None:
        warnings.warn(
            "The parameter n_jobs is not supported and has been ignored.",
            category=RuntimeWarning)

    return pairwise_distances_da(X=X, Y=Y, metric=metric, p=p)
