/*
 * Copyright (C) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "aoclda.h"
#include "da_error.hpp"
#include "da_omp.hpp"
#include "da_std.hpp"
#include "decision_forest.hpp"
#include "decision_tree_options.hpp"
#include "decision_tree_types.hpp"
#include "macros.h"
#include "options.hpp"

#include <chrono>
#include <deque>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
#include <type_traits>
#include <vector>

#include "boost/sort/spreadsort/spreadsort.hpp"

namespace ARCH {

namespace da_decision_forest {

using namespace da_decision_tree_types;

template <typename T> void node<T>::dummy() {}

template <typename T> void split<T>::copy(split const &sp) {
    feat_idx = sp.feat_idx;
    samp_idx = sp.samp_idx;
    score = sp.score;
    threshold = sp.threshold;
    left_score = sp.left_score;
    right_score = sp.right_score;
}

/* Compute the impurity of a node containing n_samples samples.
 * On input, count_classes[i] is assumed to contain the number of
 * occurrences of class i within the node samples. */
template <class T>
using score_fun_t = typename std::function<T(da_int, da_int, std::vector<da_int> &)>;

template <class T>
T gini_score(da_int n_samples, da_int n_class, std::vector<da_int> &count_classes) {
    T score = 0.0;
    for (da_int c = 0; c < n_class; c++) {
        score += (T)count_classes[c] * (T)count_classes[c];
    }
    score = (T)1.0 - score / ((T)n_samples * (T)n_samples);
    return score;
}

template <class T>
T entropy_score(da_int n_samples, da_int n_class, std::vector<da_int> &count_classes) {
    T score = 0.0;
    for (da_int c = 0; c < n_class; c++) {
        T prob_c = (T)count_classes[c] / (T)n_samples;
        if (prob_c > (T)1.0e-5)
            score -= prob_c * std::log2(prob_c);
    }
    return score;
}

template <class T>
T misclassification_score(da_int n_samples, [[maybe_unused]] da_int n_class,
                          std::vector<da_int> &count_classes) {
    T score =
        (T)1.0 -
        ((T)*std::max_element(count_classes.begin(), count_classes.end())) / (T)n_samples;
    return score;
}

template <typename T>
decision_tree<T>::decision_tree(da_errors::da_error_t &err) : basic_handle<T>(err) {
    // Initialize the options registry
    // Any error is stored err->status[.] and this needs to be checked
    // by the caller.
    register_decision_tree_options<T>(this->opts, *this->err);
}

// Constructor bypassing the optional parameters for internal forest use
// Values will NOT be checked
template <typename T>
decision_tree<T>::decision_tree(da_int max_depth, da_int min_node_sample, da_int method,
                                da_int prn_times, da_int build_order, da_int nfeat_split,
                                da_int seed, da_int sort_method, T min_split_score,
                                T feat_thresh, T min_improvement, bool bootstrap)
    : max_depth(max_depth), min_node_sample(min_node_sample), method(method),
      prn_times(prn_times), build_order(build_order), nfeat_split(nfeat_split),
      seed(seed), sort_method(sort_method), min_split_score(min_split_score),
      feat_thresh(feat_thresh), min_improvement(min_improvement), bootstrap(bootstrap) {
    this->err = nullptr;
    read_public_options = false;
}

template <typename T> decision_tree<T>::~decision_tree() {
    // Destructor needs to handle arrays that were allocated due to row major storage of input data
    if (X_temp)
        delete[] (X_temp);
}

template <typename T> void decision_tree<T>::refresh() {
    model_trained = false;
    if (tree.capacity() > 0)
        tree = std::vector<node<T>>();
}

template <typename T> da_status decision_tree<T>::resize_tree(size_t new_size) {
    try {
        tree.resize(new_size);
        class_props.resize(new_size * this->n_class);
        return da_status_success;
    } catch (std::bad_alloc &) {                                  // LCOV_EXCL_LINE
        return da_error_bypass(this->err, da_status_memory_error, // LCOV_EXCL_LINE
                               "Memory allocation error");
    }
}

template <typename T>
da_status decision_tree<T>::get_result([[maybe_unused]] da_result query,
                                       [[maybe_unused]] da_int *dim,
                                       [[maybe_unused]] da_int *result) {

    return da_warn_bypass(this->err, da_status_unknown_query,
                          "There are no integer results available for this API.");
};

// Getters for testing purposes
template <typename T> std::vector<da_int> const &decision_tree<T>::get_samples_idx() {
    return samples_idx;
}
template <typename T> std::vector<T> const &decision_tree<T>::get_features_values() {
    return feature_values;
}
template <typename T> std::vector<da_int> const &decision_tree<T>::get_count_classes() {
    return count_classes;
}
template <typename T>
std::vector<da_int> const &decision_tree<T>::get_count_left_classes() {
    return count_left_classes;
}
template <typename T>
std::vector<da_int> const &decision_tree<T>::get_count_right_classes() {
    return count_right_classes;
}
template <typename T> std::vector<da_int> const &decision_tree<T>::get_features_idx() {
    return features_idx;
}
template <typename T> bool decision_tree<T>::model_is_trained() { return model_trained; }
template <typename T> std::vector<node<T>> const &decision_tree<T>::get_tree() {
    return tree;
}

// Setters for testing purposes
template <typename T> void decision_tree<T>::set_bootstrap(bool bs) {
    this->bootstrap = bs;
}

template <typename T>
da_status decision_tree<T>::get_result(da_result query, da_int *dim, T *result) {

    if (!model_trained)
        return da_warn_bypass(
            this->err, da_status_unknown_query,
            "Handle does not contain data relevant to this query. Was the "
            "last call to the solver successful?");
    // Pointers were already tested in the generic get_result

    da_int rinfo_size = 7;
    switch (query) {
    case da_result::da_rinfo:
        if (*dim < rinfo_size) {
            *dim = rinfo_size;
            return da_warn_bypass(this->err, da_status_invalid_array_dimension,
                                  "The array is too small. Please provide an array of at "
                                  "least size: " +
                                      std::to_string(rinfo_size) + ".");
        }
        result[0] = (T)n_features;
        result[1] = (T)n_samples;
        result[2] = (T)n_obs;
        result[3] = (T)seed;
        result[4] = (T)depth;
        result[5] = (T)n_nodes;
        result[6] = (T)n_leaves;
        break;
    default:
        return da_warn_bypass(this->err, da_status_unknown_query,
                              "The requested result could not be found.");
    }
    return da_status_success;
}

template <typename T>
da_status decision_tree<T>::set_training_data(da_int n_samples, da_int n_features,
                                              const T *X, da_int ldx, const da_int *y,
                                              da_int n_class, da_int n_obs,
                                              da_int *samples_subset) {

    // Guard against errors due to multiple calls using the same class instantiation
    if (X_temp) {
        delete[] (X_temp);
        X_temp = nullptr;
    }

    da_status status =
        this->store_2D_array(n_samples, n_features, X, ldx, &X_temp, &this->X, this->ldx,
                             "n_samples", "n_features", "X", "ldx");
    if (status != da_status_success)
        return status;

    status = this->check_1D_array(n_samples, y, "n_samples", "y", 1);
    if (status != da_status_success)
        return status;

    if (n_obs > n_samples || n_obs < 0) {
        return da_error_bypass(this->err, da_status_invalid_input,
                               "n_obs = " + std::to_string(n_obs) +
                                   ", it must be set between 0 and n_samples = " +
                                   std::to_string(n_samples));
    }

    this->refresh();
    this->y = y;
    this->n_samples = n_samples;
    this->n_features = n_features;
    this->n_class = n_class;
    if (n_class <= 0)
        this->n_class = *std::max_element(y, y + n_samples) + 1;
    this->n_obs = n_obs;
    if (this->n_obs == 0)
        this->n_obs = this->n_samples;
    this->samples_subset = samples_subset;

    // Initialize working memory
    // samples_idx contains all the indices in order [0:n_samples-1]
    try {
        samples_idx.resize(this->n_obs);
        count_classes.resize(this->n_class);
        feature_values.resize(this->n_obs);
        count_left_classes.resize(this->n_class);
        count_right_classes.resize(this->n_class);
        features_idx.resize(this->n_features);
    } catch (std::bad_alloc &) {                                  // LCOV_EXCL_LINE
        return da_error_bypass(this->err, da_status_memory_error, // LCOV_EXCL_LINE
                               "Memory allocation error");
    }
    da_std::iota(features_idx.begin(), features_idx.end(), 0);

    return da_status_success;
}

template <class T>
void decision_tree<T>::count_class_occurences(std::vector<da_int> &class_occ,
                                              da_int start_idx, da_int end_idx) {
    da_std::fill(class_occ.begin(), class_occ.end(), 0);
    for (da_int i = start_idx; i <= end_idx; i++) {
        da_int idx = samples_idx[i];
        da_int c = y[idx];
        class_occ[c] += 1;
    }
}

/* Possible errors:
 * - memory
 */
template <class T>
da_status decision_tree<T>::add_node(da_int parent_idx, bool is_left, T score,
                                     da_int split_idx) {

    da_status status = da_status_success;
    if (tree.size() <= (size_t)n_nodes) {
        size_t new_size = 2 * tree.size() + 1;
        // Resize the tree and class_props arrays
        if (predict_proba_opt)
            status = resize_tree(new_size);
        if (status != da_status_success)
            return status;
    }

    if (is_left) {
        tree[parent_idx].left_child_idx = n_nodes;
        tree[n_nodes].start_idx = tree[parent_idx].start_idx;
        tree[n_nodes].end_idx = split_idx;
    } else {
        tree[parent_idx].right_child_idx = n_nodes;
        tree[n_nodes].start_idx = split_idx + 1;
        tree[n_nodes].end_idx = tree[parent_idx].end_idx;
    }
    tree[n_nodes].depth = tree[parent_idx].depth + 1;
    if (tree[n_nodes].depth > this->depth)
        this->depth = tree[n_nodes].depth;
    tree[n_nodes].score = score;
    tree[n_nodes].n_samples = tree[n_nodes].end_idx - tree[n_nodes].start_idx + 1;
    // Prediction: most represented class in the samples subset
    count_class_occurences(count_classes, tree[n_nodes].start_idx, tree[n_nodes].end_idx);
    tree[n_nodes].y_pred = (da_int)std::distance(
        count_classes.begin(),
        std::max_element(count_classes.begin(), count_classes.end()));
    // Prediction probability
    if (predict_proba_opt) {
        for (da_int i = 0; i < n_class; i++) {
            T p = (T)count_classes[i] / (T)tree[n_nodes].n_samples;
            class_props[n_nodes * n_class + i] = p;
        }
    }
    n_nodes += 1;

    return status;
}

/* Partition samples_idx so that all the values below x_thresh are first */
template <typename T> void decision_tree<T>::partition_samples(const node<T> &nd) {
    da_int head_idx = nd.start_idx, tail_idx = nd.end_idx;
    da_int start_col = ldx * nd.feature;
    while (head_idx < tail_idx) {
        da_int idx = samples_idx[head_idx];
        T val = X[start_col + idx];
        if (val < nd.x_threshold)
            head_idx += 1;
        else {
            da_int aux = samples_idx[head_idx];
            samples_idx[head_idx] = samples_idx[tail_idx];
            samples_idx[tail_idx] = aux;
            tail_idx -= 1;
        }
    }
}

template <class T>
void boost_sort_samples(const T *X, da_int ldx, da_int feat_idx,
                        std::vector<da_int>::iterator start,
                        std::vector<da_int>::iterator stop) {
    // namespace for spreadsort::float_sort and spreadsort::float_mem_cast
    using namespace boost::sort;
    auto rightshift_64 = [&](const da_int &idx, const unsigned offset) {
        int64_t X_cast =
            spreadsort::float_mem_cast<double, int64_t>(X[ldx * feat_idx + idx]);
        return X_cast >> offset;
    };
    auto rightshift_32 = [&](const da_int &idx, const unsigned offset) {
        int32_t X_cast =
            spreadsort::float_mem_cast<float, int32_t>(X[ldx * feat_idx + idx]);
        return X_cast >> offset;
    };
    if constexpr (std::is_same<T, double>::value) {
        // call spreadsort::float_sort with casting to INT64
        spreadsort::float_sort(start, stop, rightshift_64,
                               [&](const da_int &i1, const da_int &i2) {
                                   return X[ldx * feat_idx + i1] < X[ldx * feat_idx + i2];
                               });

    } else if constexpr (std::is_same<T, float>::value) {
        // define function for casting to INT32
        spreadsort::float_sort(start, stop, rightshift_32,
                               [&](const da_int &i1, const da_int &i2) {
                                   return X[ldx * feat_idx + i1] < X[ldx * feat_idx + i2];
                               });
    }
}

template <class T>
void std_sort_samples(const T *X, da_int ldx, da_int feat_idx,
                      std::vector<da_int>::iterator start,
                      std::vector<da_int>::iterator stop) {
    std::sort(start, stop, [&](const da_int &i1, const da_int &i2) {
        return X[ldx * feat_idx + i1] < X[ldx * feat_idx + i2];
    });
}

template <class T> void decision_tree<T>::sort_samples(node<T> &nd, da_int feat_idx) {
    /* Sort samples_idx according to the values of a given feature.
     * On output:
     * - the values of samples_idx will be sorted between the start and end indices
     *   of the node nd
     * - feature_values[nd.start_idx:nd.end_idx] will contain the values of the feat_idx feature
     *   corresponding to the indices in samples_idx
     */

    std::vector<da_int>::iterator start = samples_idx.begin() + nd.start_idx;
    std::vector<da_int>::iterator stop =
        samples_idx.begin() + nd.start_idx + nd.n_samples;

    switch (sort_method) {
    case stl_sort:
        std_sort_samples(X, ldx, feat_idx, start, stop);
        break;

    case boost_sort:
        boost_sort_samples(X, ldx, feat_idx, start, stop);
        break;
    }

    for (da_int i = nd.start_idx; i <= nd.end_idx; i++)
        feature_values[i] = X[ldx * feat_idx + samples_idx[i]];
}

template <class T> da_int decision_tree<T>::get_next_node_idx(da_int build_order) {
    // Get the next node index to treat in function of the building order selected.
    // LIFO: depth-first
    // FIFO: breadth-first
    da_int node_idx = -1;
    switch (build_order) {
    case depth_first:
        node_idx = nodes_to_treat.back();
        nodes_to_treat.pop_back();
        break;
    case breadth_first:
        node_idx = nodes_to_treat.front();
        nodes_to_treat.pop_front();
        break;
    }

    return node_idx;
}

/* Test all the possible splits and return the best one */
template <typename T>
void decision_tree<T>::find_best_split(node<T> &current_node, T feat_thresh,
                                       T maximum_split_score, split<T> &sp) {

    // Initialize the split, all nodes to the right child.
    // count_class, samples_idx and feature_values are required to be up to date
    std::copy(count_classes.begin(), count_classes.end(), count_right_classes.begin());
    da_std::fill(count_left_classes.begin(), count_left_classes.end(), 0);
    T right_score = current_node.score, left_score = 0.0;
    da_int ns_left = 0;
    da_int ns_right = current_node.n_samples;
    sp.score = current_node.score;
    sp.samp_idx = -1;

    T split_score;
    da_int sidx = current_node.start_idx;
    while (sidx <= current_node.end_idx - 1) {
        da_int c = y[samples_idx[sidx]];
        count_left_classes[c] += 1;
        count_right_classes[c] -= 1;
        ns_left += 1;
        ns_right -= 1;

        // Skip testing splits where feature values are too close
        while (sidx + 1 <= current_node.end_idx &&
               std::abs(feature_values[sidx + 1] - feature_values[sidx]) < feat_thresh) {
            c = y[samples_idx[sidx + 1]];
            count_left_classes[c]++;
            count_right_classes[c]--;
            ns_left += 1;
            ns_right -= 1;
            sidx++;
        }
        if (sidx == current_node.end_idx)
            // All samples are in the left child. Do not check the split
            break;

        left_score = score_function(ns_left, n_class, count_left_classes);
        right_score = score_function(ns_right, n_class, count_right_classes);
        split_score =
            (left_score * ns_left + right_score * ns_right) / current_node.n_samples;
        // Consider the split only if it brings at least minimum improvement
        // compared to the parent node
        if (split_score < sp.score && split_score < maximum_split_score) {
            sp.score = split_score;
            sp.samp_idx = sidx;
            sp.threshold = (feature_values[sidx] + feature_values[sidx + 1]) / 2;
            sp.right_score = right_score;
            sp.left_score = left_score;
        }

        sidx++;
    }
}

template <typename T> da_status decision_tree<T>::fit() {
    da_status status = da_status_success;

    if (model_trained)
        // Nothing to do, exit
        return da_status_success;

    // Extract options
    if (read_public_options) {
        std::string opt_val;
        bool opt_pass = true;
        opt_pass &= this->opts.get("predict probabilities", predict_proba_opt) ==
                    da_status_success;
        opt_pass &= this->opts.get("maximum depth", max_depth) == da_status_success;
        opt_pass &=
            this->opts.get("scoring function", opt_val, method) == da_status_success;
        opt_pass &=
            this->opts.get("Node minimum samples", min_node_sample) == da_status_success;
        opt_pass &=
            this->opts.get("Minimum split score", min_split_score) == da_status_success;
        opt_pass &= this->opts.get("tree building order", opt_val, build_order) ==
                    da_status_success;
        opt_pass &= this->opts.get("maximum features", nfeat_split) == da_status_success;
        opt_pass &= this->opts.get("seed", seed) == da_status_success;
        opt_pass &= this->opts.get("feature threshold", feat_thresh) == da_status_success;
        opt_pass &= this->opts.get("minimum split improvement", min_improvement) ==
                    da_status_success;
        opt_pass &= this->opts.get("minimum split improvement", min_improvement) ==
                    da_status_success;
        opt_pass &=
            this->opts.get("print timings", opt_val, prn_times) == da_status_success;
        opt_pass &=
            this->opts.get("sorting method", opt_val, sort_method) == da_status_success;
        if (!opt_pass)
            return da_error_bypass(
                this->err, da_status_internal_error, // LCOV_EXCL_LINE
                "Unexpected error while reading the optional parameters.");
    }

    switch (method) {
    case gini:
        score_function = gini_score<T>;
        break;

    case cross_entropy:
        score_function = entropy_score<T>;
        break;

    case misclassification:
        score_function = misclassification_score<T>;
        break;
    }

    if (nfeat_split == 0 || nfeat_split > n_features) {
        // All the features are to be considered in splitting a node
        nfeat_split = n_features;
    }

    // Initialize random number generator
    if (seed == -1) {
        std::random_device r;
        seed = std::abs((da_int)r());
    }
    mt_engine.seed(seed);

    // Allocate the tree and class_props arrays
    // accounting for a full binary tree of depth 10 (or maximum depth)
    size_t init_capacity = ((da_int)1 << std::min(max_depth, (da_int)9)) + (da_int)1;
    if (predict_proba_opt)
        status = resize_tree(init_capacity);
    if (status != da_status_success)
        return status;

    if (!bootstrap) {
        // Take all the samples
        da_std::iota(samples_idx.begin(), samples_idx.end(), 0);
    } else {
        if (samples_subset == nullptr) {
            // Fill the index vector with a random selection with replacement
            std::uniform_int_distribution<da_int> uniform_dist(0, n_samples - 1);
            std::generate(samples_idx.begin(), samples_idx.end(),
                          [&uniform_dist, &mt_engine = this->mt_engine]() {
                              return uniform_dist(mt_engine);
                          });
        } else {
            // Copy the input from the samples_subset array.
            // As it is intended mainly for testing, samples_subset is NOT validated.
            for (da_int i = 0; i < n_obs; i++)
                samples_idx[i] = samples_subset[i];
        }
    }

    // Reset number of leaves (if calling fit multiple times)
    n_leaves = 0;

    // Initialize the root node
    n_nodes = 1;
    tree[0].start_idx = 0;
    tree[0].end_idx = n_obs - 1;
    tree[0].depth = 0;
    tree[0].n_samples = n_obs;
    count_class_occurences(count_classes, 0, n_obs - 1);
    tree[0].score = score_function(n_obs, n_class, count_classes);
    tree[0].y_pred = (da_int)std::distance(
        count_classes.begin(),
        std::max_element(count_classes.begin(), count_classes.end()));
    // Prediction probability
    if (predict_proba_opt) {
        for (da_int i = 0; i < n_class; i++) {
            T p = (T)count_classes[i] / (T)n_obs;
            class_props[i] = p;
        }
    }

    // Insert the root node in the queue if the maximum depth is big enough
    if (max_depth > 0)
        nodes_to_treat.push_back(0);

    split<T> sp, best_split;
    while (!nodes_to_treat.empty()) {
        da_int node_idx = get_next_node_idx(build_order);
        node<T> &current_node = tree[node_idx];
        T maximum_split_score = current_node.score - min_improvement;

        // Explore the candidate features for splitting
        // Randomly shuffle the index array and explore the first nfeat_split
        if (nfeat_split < n_features)
            std::shuffle(features_idx.begin(), features_idx.end(), mt_engine);
        best_split.score = current_node.score;
        best_split.feat_idx = -1;
        count_class_occurences(count_classes, current_node.start_idx,
                               current_node.end_idx);
        for (da_int j = 0; j < nfeat_split; j++) {
            da_int feat_idx = features_idx[j];
            sort_samples(current_node, feat_idx);
            sp.feat_idx = feat_idx;
            find_best_split(current_node, feat_thresh, maximum_split_score, sp);

            if (sp.score < best_split.score) {
                best_split.copy(sp);
            }
        }

        // Split the node and add the 2 children
        if (best_split.feat_idx != -1) {
            current_node.is_leaf = false;
            current_node.feature = best_split.feat_idx;
            current_node.x_threshold = best_split.threshold;

            // Sort again the samples according to the chosen feature
            partition_samples(current_node);

            // Add children nodes and push them into the queue
            // if potential for further improvements is still high enough
            add_node(node_idx, false, best_split.right_score, best_split.samp_idx);
            if (best_split.right_score > min_split_score &&
                tree[n_nodes - 1].n_samples >= min_node_sample &&
                tree[n_nodes - 1].depth < max_depth)
                nodes_to_treat.push_back(n_nodes - 1);
            else
                n_leaves += 1;
            add_node(node_idx, true, best_split.left_score, best_split.samp_idx);
            if (best_split.left_score > min_split_score &&
                tree[n_nodes - 1].n_samples >= min_node_sample &&
                tree[n_nodes - 1].depth < max_depth)
                nodes_to_treat.push_back(n_nodes - 1);
            else
                n_leaves += 1;
        } else
            n_leaves += 1;
    }

    model_trained = true;
    return status;
}

template <typename T>
da_status decision_tree<T>::predict(da_int nsamp, da_int nfeat, const T *X_test,
                                    da_int ldx_test, da_int *y_pred, da_int mode) {
    if (y_pred == nullptr) {
        return da_error_bypass(this->err, da_status_invalid_pointer,
                               "y_pred is not a valid pointer.");
    }

    const T *X_test_temp;
    T *utility_ptr1 = nullptr;
    da_int ldx_test_temp;

    if (nfeat != n_features) {
        return da_error_bypass(this->err, da_status_invalid_input,
                               "n_features = " + std::to_string(nfeat) +
                                   " doesn't match the expected value " +
                                   std::to_string(n_features) + ".");
    }

    if (!model_trained) {
        return da_error_bypass(this->err, da_status_out_of_date,
                               "The model has not yet been trained or the data it is "
                               "associated with is out of date.");
    }

    da_status status = this->store_2D_array(nsamp, nfeat, X_test, ldx_test, &utility_ptr1,
                                            &X_test_temp, ldx_test_temp, "n_samples",
                                            "n_features", "X_test", "ldx_test", mode);
    if (status != da_status_success)
        return status;

    // Fill y_pred with the values of all the requested samples
    node<T> *current_node;
    for (da_int i = 0; i < nsamp; i++) {
        current_node = &tree[0];
        while (!current_node->is_leaf) {
            T feat_val = X_test_temp[ldx_test_temp * current_node->feature + i];
            if (feat_val < current_node->x_threshold)
                current_node = &tree[current_node->left_child_idx];
            else
                current_node = &tree[current_node->right_child_idx];
        }
        y_pred[i] = current_node->y_pred;
    }
    if (utility_ptr1)
        delete[] (utility_ptr1);
    return da_status_success;
}

template <typename T>
da_status decision_tree<T>::predict_proba(da_int nsamp, da_int nfeat, const T *X_test,
                                          da_int ldx_test, T *y_proba_pred, da_int nclass,
                                          da_int ldy, da_int mode) {

    const T *X_test_temp;
    T *utility_ptr1;
    T *utility_ptr2;
    da_int ldx_test_temp;
    T *y_proba_pred_temp;
    da_int ldy_proba_pred_temp;

    if (!predict_proba_opt) {
        return da_error_bypass(this->err, da_status_invalid_input,
                               "predict_proba must be set to 1");
    }

    if (nfeat != n_features) {
        return da_error_bypass(this->err, da_status_invalid_input,
                               "n_features = " + std::to_string(nfeat) +
                                   " doesn't match the expected value " +
                                   std::to_string(n_features) + ".");
    }

    if (nclass != n_class) {
        return da_error_bypass(this->err, da_status_invalid_input,
                               "n_features = " + std::to_string(nclass) +
                                   " doesn't match the expected value " +
                                   std::to_string(nclass) + ".");
    }

    if (!model_trained) {
        return da_error_bypass(this->err, da_status_out_of_date,
                               "The model has not yet been trained or the data it is "
                               "associated with is out of date.");
    }

    da_status status = this->store_2D_array(nsamp, nfeat, X_test, ldx_test, &utility_ptr1,
                                            &X_test_temp, ldx_test_temp, "n_samples",
                                            "n_features", "X_test", "ldx_test", mode);
    if (status != da_status_success)
        return status;

    da_int mode_output = (mode == 0) ? 1 : mode;
    status = this->store_2D_array(nsamp, nclass, y_proba_pred, ldy, &utility_ptr2,
                                  const_cast<const T **>(&y_proba_pred_temp),
                                  ldy_proba_pred_temp, "n_samples", "n_class", "y_proba",
                                  "ldy", mode_output);
    if (status != da_status_success)
        return status;

    // Fill y_proba_pred with the values of all the requested samples
    node<T> *current_node;
    for (da_int i = 0; i < nsamp; i++) {
        current_node = &tree[0];
        da_int current_node_idx = 0;
        while (!current_node->is_leaf) {
            T feat_val = X_test_temp[ldx_test_temp * current_node->feature + i];
            if (feat_val < current_node->x_threshold) {
                current_node_idx = current_node->left_child_idx;
                current_node = &tree[current_node_idx];
            } else {
                current_node_idx = current_node->right_child_idx;
                current_node = &tree[current_node_idx];
            }
        }
        for (da_int j = 0; j < n_class; j++)
            y_proba_pred_temp[ldy_proba_pred_temp * j + i] =
                class_props[n_class * current_node_idx + j];
    }

    if (this->order == row_major) {

        da_utils::copy_transpose_2D_array_column_to_row_major(
            nsamp, n_class, y_proba_pred_temp, ldy_proba_pred_temp, y_proba_pred, ldy);
        if (utility_ptr1)
            delete[] (utility_ptr1);
        if (utility_ptr2)
            delete[] (utility_ptr2);
    }
    return da_status_success;
}

template <typename T>
da_status decision_tree<T>::predict_log_proba(da_int nsamp, da_int nfeat, const T *X_test,
                                              da_int ldx_test, T *y_log_proba,
                                              da_int nclass, da_int ldy) {
    da_status status = da_status_success;

    status = predict_proba(nsamp, nfeat, X_test, ldx_test, y_log_proba, n_class, ldy);
    if (status != da_status_success)
        return status;

    if (this->order == column_major) {
        for (da_int j = 0; j < nclass; j++) {
            for (da_int i = 0; i < nsamp; i++) {
                y_log_proba[ldy * j + i] = log(y_log_proba[ldy * j + i]);
            }
        }
    } else {
        for (da_int j = 0; j < nsamp; j++) {
            for (da_int i = 0; i < nclass; i++) {
                y_log_proba[j * ldy + i] = log(y_log_proba[j * ldy + i]);
            }
        }
    }
    return status;
}

template <typename T>
da_status decision_tree<T>::score(da_int nsamp, da_int nfeat, const T *X_test,
                                  da_int ldx_test, const da_int *y_test, T *accuracy) {

    const T *X_test_temp;
    T *utility_ptr1 = nullptr;
    da_int ldx_test_temp;

    if (accuracy == nullptr) {
        return da_error_bypass(this->err, da_status_invalid_pointer,
                               "mean_accuracy is not valid pointers.");
    }

    if (nfeat != n_features) {
        return da_error_bypass(this->err, da_status_invalid_input,
                               "nfeat = " + std::to_string(nfeat) +
                                   " doesn't match the expected value " +
                                   std::to_string(n_features) + ".");
    }

    if (!model_trained) {
        return da_error_bypass(this->err, da_status_out_of_date,
                               "The model has not yet been trained or the data it is "
                               "associated with is out of date.");
    }

    da_status status = this->store_2D_array(nsamp, nfeat, X_test, ldx_test, &utility_ptr1,
                                            &X_test_temp, ldx_test_temp, "n_samples",
                                            "n_features", "X_test", "ldx_test");
    if (status != da_status_success)
        return status;

    status = this->check_1D_array(nsamp, y_test, "n_samples", "y_test", 1);
    if (status != da_status_success)
        return status;

    node<T> *current_node;
    *accuracy = 0.;
    for (da_int i = 0; i < nsamp; i++) {
        current_node = &tree[0];
        while (!current_node->is_leaf) {
            T feat_val = X_test_temp[ldx_test_temp * current_node->feature + i];
            if (feat_val < current_node->x_threshold)
                current_node = &tree[current_node->left_child_idx];
            else
                current_node = &tree[current_node->right_child_idx];
        }
        if (current_node->y_pred == y_test[i])
            *accuracy += (T)1.0;
    }
    *accuracy = *accuracy / (T)nsamp;
    if (utility_ptr1)
        delete[] (utility_ptr1);

    return da_status_success;
}

template <typename T> void decision_tree<T>::clear_working_memory() {
    samples_idx = std::vector<da_int>();
    count_classes = std::vector<da_int>();
    feature_values = std::vector<T>();
    count_left_classes = std::vector<da_int>();
    count_right_classes = std::vector<da_int>();
    features_idx = std::vector<da_int>();
    if (X_temp) {
        delete[] (X_temp);
        X = nullptr;
    }
}

template <class T>
using score_fun_t = typename std::function<T(da_int, da_int, std::vector<da_int> &)>;

template double gini_score<double>(da_int n_samples, da_int n_class,
                                   std::vector<da_int> &count_classes);

template float gini_score<float>(da_int n_samples, da_int n_class,
                                 std::vector<da_int> &count_classes);

template double entropy_score<double>(da_int n_samples, da_int n_class,
                                      std::vector<da_int> &count_classes);

template float entropy_score<float>(da_int n_samples, da_int n_class,
                                    std::vector<da_int> &count_classes);

template double misclassification_score<double>(da_int n_samples,
                                                [[maybe_unused]] da_int n_class,
                                                std::vector<da_int> &count_classes);

template float misclassification_score<float>(da_int n_samples,
                                              [[maybe_unused]] da_int n_class,
                                              std::vector<da_int> &count_classes);
template class decision_tree<double>;
template class decision_tree<float>;

} // namespace da_decision_forest

} // namespace ARCH