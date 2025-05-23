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

# pylint: disable = import-error, invalid-name, too-many-arguments

"""
aoclda.decision_tree module
"""

from ._aoclda.decision_tree import pybind_decision_tree


class decision_tree():
    """
    A decision tree classifier.

    Args:
        seed (int, optional): Set the random seed for the random number generator.
            If the value is -1, a random seed is automatically generated. In this case
            the resulting classification will create non-reproducible results.
            Default = -1.

        max_depth (int, optional): Set the maximum depth of the tree. Default = 29.

        max_features (int, optional): Set the number of features to consider when
            splitting a node. 0 means take all the features. Default 0.

        criterion (str, optional): Select scoring function to use. It can take the values
            'cross-entropy', 'gini', or 'misclassification'

        min_samples_split (int, optional): The minimum number of samples required to
            split an internal node. Default 2.

        build_order (str, optional): Select in which order to explore the nodes. It can
            take the values 'breadth first' or 'depth first'. Default 'breadth first'.

        min_impurity_decrease (float, optional): Minimum score improvement needed to consider a
            split from the parent node. Default 0.0

        min_split_score (float, optional): Minimum score needed for a node to be considered for
            splitting. Default 0.0.

        feat_thresh (float, optional): Minimum difference in feature value required for splitting.
            Default 1.0e-06

        sort_method  (str, optional): Select sorting to use. It can take the values
            'boost', or 'stl'.
            Default = 'boost'.

        precision (str, optional): Whether to initialize the PCA object in double or
            single precision. It can take the values 'single' or 'double'.
            Default = 'double'.

        check_data (bool, optional): Whether to check the data for NaNs. Default = False.
    """

    def __init__(self,
                 criterion='gini',
                 seed=-1, max_depth=29,
                 min_samples_split=2, build_order='breadth first',
                 sort_method='boost',
                 max_features=0, min_impurity_decrease=0.0, min_split_score=0.0,
                 feat_thresh=1.0e-06, check_data=False):

        self.decision_tree_double = pybind_decision_tree(criterion=criterion,
                         seed=seed, max_depth=max_depth,
                         min_samples_split=min_samples_split, build_order=build_order,
                         sort_method = sort_method,
                         max_features=max_features, precision="double", check_data=check_data)
        self.decision_tree_single = pybind_decision_tree(criterion=criterion,
                         seed=seed, max_depth=max_depth,
                         min_samples_split=min_samples_split, build_order=build_order,
                         sort_method = sort_method,
                         max_features=max_features, precision="single", check_data=check_data)
        self.decision_tree = self.decision_tree_double

        self.max_features = max_features
        self.min_impurity_decrease = min_impurity_decrease
        self.min_split_score = min_split_score
        self.feat_thresh = feat_thresh

    @property
    def max_features(self):
        return self.max_features

    @max_features.setter
    def max_features(self, value):
        self.decision_tree.set_max_features_opt(max_features=value)

    def fit(self, X, y):
        """
        Computes the decision tree on the feature matrix ``X`` and response vector ``y``

        Args:
            X (numpy.ndarray): The feature matrix on which to compute the model.
                Its shape is (n_samples, n_features).

            y (numpy.ndarray): The response vector. Its shape is (n_samples).

        Returns:
            self (object): Returns the instance itself.
        """
        if X.dtype == "float32":
            self.decision_tree = self.decision_tree_single
            self.decision_tree_double = None

        return self.decision_tree.pybind_fit(X, y,
                                             self.min_impurity_decrease,
                                             self.min_split_score,
                                             self.feat_thresh)

    def score(self, X, y):
        """
        Calculates score (prediction accuracy) by comparing predicted labels and actual
        labels on a new set of data.

        Args:
            X (numpy.ndarray): The feature matrix to evaluate the model on. It must have
                n_features columns.

            y (numpy.ndarray): The response vector.  It must have shape (n_samples).

        Returns:
            float: The mean accuracy of the model on the test data.
        """
        return self.decision_tree.pybind_score(X, y)

    def predict(self, X):
        """
        Generate labels using fitted decision forest on a new set of data ``X``.

        Args:
            X (numpy.ndarray): The feature matrix to evaluate the model on. It must have
            n_features columns.

        Returns:
            numpy.ndarray of length n_samples: The prediction vector, where n_samples is
            the number of rows of ``X``.
        """
        return self.decision_tree.pybind_predict(X)

    def predict_proba(self, X):
        """
        Generate class probabilities using fitted decision forest on a new set of data ``X``.

        Args:
            X (numpy.ndarray): The feature matrix to evaluate the model on. It must have
            n_features columns.

        Returns:
            numpy.ndarray of length n_samples: The prediction vector, where n_samples is
            the number of rows of ``X``.
        """
        return self.decision_tree.pybind_predict_proba(X)

    def predict_log_proba(self, X):
        """
        Generate class log probabilities using fitted decision forest on a new set of data ``X``.

        Args:
            X (numpy.ndarray): The feature matrix to evaluate the model on.
                It must have n_features columns.

        Returns:
            numpy.ndarray of length n_samples: The prediction vector,
                where n_samples is the number of rows of X.
        """
        return self.decision_tree.pybind_predict_log_proba(X)

    @property
    def n_nodes(self):
        """int: The number of nodes in the trained tree"""
        return self.decision_tree.get_n_nodes()

    @property
    def n_leaves(self):
        """int: The number of nodes in the trained tree"""
        return self.decision_tree.get_n_leaves()
