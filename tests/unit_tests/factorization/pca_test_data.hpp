/*
 * Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include <limits>
#include <list>
#include <string.h>

#include "../datests_cblas.hh"
#include "../utest_utils.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

template <typename T> struct PCAParamType {

    std::string test_name;

    da_int n = 0;
    da_int p = 0;
    std::vector<T> A;
    da_int lda = 0;

    da_int components_required = 0;
    std::string method;
    std::string degrees_of_freedom;
    std::string svd_solver;

    std::vector<T> expected_scores;
    std::vector<T> expected_components;
    std::vector<T> expected_variance;
    std::vector<T> expected_u;
    std::vector<T> expected_vt;
    std::vector<T> expected_sigma;
    T expected_total_variance = 0.0;
    da_int expected_n_components = 0;

    std::vector<T> expected_means;
    std::vector<T> expected_sdevs;
    std::vector<T> expected_rinfo;

    da_int m = 0;
    std::vector<T> X;
    da_int ldx = 0;
    std::vector<T> expected_X_transform;
    da_int ldx_transform = 0;
    std::string order = "column-major";

    da_int k = 0;
    std::vector<T> Xinv;
    da_int ldxinv = 0;
    std::vector<T> expected_Xinv_transform;
    da_int ldxinv_transform = 0;

    da_status expected_status = da_status_success;
    da_int store_U = 1;
    T epsilon = 10 * std::numeric_limits<T>::epsilon();
};

template <typename T> void Get1by1Data(std::vector<PCAParamType<T>> &params) {
    // Test with a 1 x 1 data matrix
    PCAParamType<T> param;
    param.test_name = "1 by 1 data matrix";
    param.n = 1;
    param.p = 1;
    std::vector<double> A{2.1};
    param.A = convert_vector<double, T>(A);
    param.lda = 1;
    param.components_required = 1;
    param.method = "covariance";
    param.svd_solver = "gesvdx";
    // For this test various combinations or 0s and 1s are valid to don't test scores or components, or u or vt
    std::vector<double> expected_variance{0.0};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{0.0};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)0.0;
    param.expected_n_components = 1;
    std::vector<double> expected_means{2.1};
    param.expected_means = convert_vector<double, T>(expected_means);
    std::vector<double> expected_rinfo{1.0, 1.0, 1.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);

    param.epsilon = 10 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void Get1by5Data(std::vector<PCAParamType<T>> &params) {
    // Test with a 1 x 5 data matrix
    PCAParamType<T> param;
    param.test_name = "1 by 5 data matrix";
    param.n = 1;
    param.p = 5;
    std::vector<double> A{2.1, 0.0, -0.3, 1.0, 1.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 1;
    param.components_required = 1;
    param.method = "correlation";
    param.svd_solver = "gesvdx";
    // For this test various combinations or 0s and 1s are valid to don't test scores or components or u or vt
    std::vector<double> expected_variance{0.0};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{0.0};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)0.0;
    param.expected_n_components = 1;
    std::vector<double> expected_means{2.1, 0.0, -0.3, 1.0, 1.0};
    param.expected_means = convert_vector<double, T>(expected_means);
    std::vector<double> expected_sdevs{0.0, 0.0, 0.0, 0.0, 0.0};
    param.expected_sdevs = convert_vector<double, T>(expected_sdevs);
    std::vector<double> expected_rinfo{1.0, 5.0, 1.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);

    param.epsilon = 10 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void Get5by1Data(std::vector<PCAParamType<T>> &params) {
    // Test with a 5 x 1 data matrix
    PCAParamType<T> param;
    param.test_name = "5 by 1 data matrix";
    param.n = 5;
    param.p = 1;
    std::vector<double> A{1.0, 2.0, 3.0, 4.0, 5.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 5;
    param.components_required = 1;
    param.method = "covariance";
    param.svd_solver = "gesvdx";
    param.degrees_of_freedom = "unbiased";
    std::vector<double> expected_scores{2.0, 1.0, 0.0, -1.0, -2.0};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{0.63245553203367599, 0.31622776601683789, 0.0,
                                   -0.31622776601683789, -0.63245553203367599};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{-1.0};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{-1.0};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{2.5};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{3.1622776601683795};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)2.5;
    param.expected_n_components = 1;
    std::vector<double> expected_means{3.0};
    param.expected_means = convert_vector<double, T>(expected_means);
    std::vector<double> expected_rinfo{5.0, 1.0, 1.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);

    param.epsilon = 100 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetDiagonalData(std::vector<PCAParamType<T>> &params) {
    // Test with a diagonal data matrix
    PCAParamType<T> param;
    param.test_name = "Diagonal data matrix";
    param.n = 4;
    param.p = 4;
    std::vector<double> A{1.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 4.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 4;
    param.components_required = 2;
    param.method = "svd";
    param.svd_solver = "gesvdx";
    std::vector<double> expected_scores{0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 3.0, 0.0};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{5.33333333333333333333, 3.0};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{4.0, 3.0};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)10.0;
    param.expected_n_components = 2;
    std::vector<double> Xinv{0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 3.0, 0.0};
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector<double> expected_Xinv_transform{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                                0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 4.0};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.ldxinv_transform = 4;
    param.k = 4;
    param.ldxinv = 4;
    std::vector<double> expected_rinfo{4.0, 4.0, 2.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    param.epsilon = 100 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetIdentityData(std::vector<PCAParamType<T>> &params) {
    // Test with an identity data matrix
    PCAParamType<T> param;
    param.test_name = "Identity data matrix";
    param.n = 4;
    param.p = 4;
    std::vector<double> A{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 4;
    param.components_required = 2;
    param.method = "svd";
    param.svd_solver = "gesvdx";
    // Various permutations for scores and components, u and vt are equally valid so don't test here
    std::vector<double> expected_variance{0.333333333333333, 0.333333333333333};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{1.0, 1.0};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)1.33333333333333333;
    param.expected_n_components = 2;
    std::vector<double> expected_rinfo{4.0, 4.0, 2.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);

    param.epsilon = 100 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetZeroData(std::vector<PCAParamType<T>> &params) {
    // Test with an empty data matrix
    PCAParamType<T> param;
    param.test_name = "Empty data matrix";
    param.n = 4;
    param.p = 4;
    std::vector<double> A{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 4;
    param.components_required = 4;
    param.method = "svd";
    param.svd_solver = "gesvdx";
    // Various permutations for scores and components, u and vt are equally valid so don't test here
    std::vector<double> expected_variance{0.0, 0.0, 0.0, 0.0};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{0.0, 0.0, 0.0, 0.0};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)0.0;
    param.expected_n_components = 4;
    std::vector<double> expected_rinfo{4.0, 4.0, 4.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);

    param.epsilon = 100 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetSquareData1(std::vector<PCAParamType<T>> &params) {
    // Test with a square data matrix
    PCAParamType<T> param;
    param.test_name = "Square data matrix 1";
    param.n = 5;
    param.p = 5;
    std::vector<double> A{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0, 2.0, 3.0, 0.2, 0.1,
                          0.8, 6.0, 4.0, 1.0, 0.9, 3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1};
    param.A = convert_vector<double, T>(A);
    param.lda = 5;
    param.components_required = 5;
    param.method = "covariance";
    param.svd_solver = "gesvdx";
    std::vector<double> expected_scores{-3.8337591301763827,
                                        6.682121109823703,
                                        -3.044191200205261,
                                        0.30775924641459096,
                                        -0.11193002585665025,
                                        0.32820531546508325,
                                        -0.9977540120299913,
                                        -2.832508506891677,
                                        -0.7275355056511974,
                                        4.2295927091077825,
                                        -2.0235465771330334,
                                        0.06578021185658105,
                                        2.4410999256583756,
                                        -1.9544605670700543,
                                        1.4711270066881295,
                                        -1.0142146976172512,
                                        -0.5985126304888071,
                                        0.08733068149179224,
                                        1.3049368666841228,
                                        0.22045977993014249,
                                        0.0,
                                        0.0,
                                        0.0,
                                        0.0,
                                        0.0};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        -0.4624602544702841,  0.8060536209817003,    -0.3672159333177392,
        0.03712450745593046,  -0.013501940649607386, 0.06253504179970557,
        -0.19010840442880456, -0.5396958231023441,   -0.13862195739333716,
        0.8058911431247803,   -0.5052213362240204,   0.016423425537536396,
        0.6094723888415844,   -0.4879725480256997,   0.3672980698705989,
        -0.5718140283399882,  -0.3374412923182887,   0.0492370194386037,
        0.735723124720095,    0.12429517649957769,   0.44721359549995804,
        0.44721359549995804,  0.447213595499958,     0.44721359549995787,
        0.447213595499958};
    param.expected_u = convert_vector<double, T>(expected_u);
    // Vanishing singular value means vt is very sensitive to sign flip in last row so we won't test it here
    std::vector<double> expected_variance{17.180698935602862, 6.886274659039021,
                                          4.010541852796752, 0.7864845525613813, 0.0};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{8.289921335116, 5.2483424655938835,
                                       4.005267458133977, 1.7736792861860695, 0.0};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)28.864;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{5.0, 5.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0, 2.0, 3.0, 0.2, 0.1,
                          0.8, 6.0, 4.0, 1.0, 0.9, 3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1};
    param.X = convert_vector<double, T>(X);
    param.m = 5;
    param.ldx = 5;
    param.ldx_transform = 5;
    std::vector<double> expected_X_transform{-3.8337591301763827,
                                             6.682121109823703,
                                             -3.044191200205261,
                                             0.30775924641459096,
                                             -0.11193002585665025,
                                             0.32820531546508325,
                                             -0.9977540120299913,
                                             -2.832508506891677,
                                             -0.7275355056511974,
                                             4.2295927091077825,
                                             -2.0235465771330334,
                                             0.06578021185658105,
                                             2.4410999256583756,
                                             -1.9544605670700543,
                                             1.4711270066881295,
                                             -1.0142146976172512,
                                             -0.5985126304888071,
                                             0.08733068149179224,
                                             1.3049368666841228,
                                             0.22045977993014249,
                                             0.0,
                                             0.0,
                                             0.0,
                                             0.0,
                                             0.0};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    param.Xinv = convert_vector<double, T>(expected_scores);
    param.expected_Xinv_transform = convert_vector<double, T>(A);
    param.k = 5;
    param.ldxinv_transform = 5;
    param.ldxinv = 5;

    param.epsilon = 100 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetSquareData2(std::vector<PCAParamType<T>> &params) {
    // Test with a square data matrix
    PCAParamType<T> param;
    param.test_name = "Square data matrix 2";
    param.n = 5;
    param.p = 5;
    std::vector<double> A{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0, 2.0, 3.0, 0.2, 0.1,
                          0.8, 6.0, 4.0, 1.0, 0.9, 3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1};
    param.A = convert_vector<double, T>(A);
    param.lda = 5;
    param.components_required = 0;
    param.method = "covariance";
    param.svd_solver = "gesdd";
    std::vector<double> expected_scores{-3.8337591301763827,
                                        6.682121109823703,
                                        -3.044191200205261,
                                        0.30775924641459096,
                                        -0.11193002585665025,
                                        0.32820531546508325,
                                        -0.9977540120299913,
                                        -2.832508506891677,
                                        -0.7275355056511974,
                                        4.2295927091077825,
                                        -2.0235465771330334,
                                        0.06578021185658105,
                                        2.4410999256583756,
                                        -1.9544605670700543,
                                        1.4711270066881295,
                                        -1.0142146976172512,
                                        -0.5985126304888071,
                                        0.08733068149179224,
                                        1.3049368666841228,
                                        0.22045977993014249,
                                        0.0,
                                        0.0,
                                        0.0,
                                        0.0,
                                        0.0};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        -0.4624602544702841,  0.8060536209817003,    -0.3672159333177392,
        0.03712450745593046,  -0.013501940649607386, 0.06253504179970557,
        -0.19010840442880456, -0.5396958231023441,   -0.13862195739333716,
        0.8058911431247803,   -0.5052213362240204,   0.016423425537536396,
        0.6094723888415844,   -0.4879725480256997,   0.3672980698705989,
        -0.5718140283399882,  -0.3374412923182887,   0.0492370194386037,
        0.735723124720095,    0.12429517649957769,   0.44721359549995804,
        0.44721359549995804,  0.447213595499958,     0.44721359549995787,
        0.447213595499958};
    param.expected_u = convert_vector<double, T>(expected_u);
    // Vanishing singular value means vt is very sensitive to sign flip in last row so we won't test it here
    std::vector<double> expected_variance{17.180698935602862, 6.886274659039021,
                                          4.010541852796752, 0.7864845525613813, 0.0};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{8.289921335116, 5.2483424655938835,
                                       4.005267458133977, 1.7736792861860695, 0.0};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)28.864;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{5.0, 5.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0, 2.0, 3.0, 0.2, 0.1,
                          0.8, 6.0, 4.0, 1.0, 0.9, 3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1};
    param.X = convert_vector<double, T>(X);
    param.m = 5;
    param.ldx = 5;
    param.ldx_transform = 5;
    std::vector<double> expected_X_transform{-3.8337591301763827,
                                             6.682121109823703,
                                             -3.044191200205261,
                                             0.30775924641459096,
                                             -0.11193002585665025,
                                             0.32820531546508325,
                                             -0.9977540120299913,
                                             -2.832508506891677,
                                             -0.7275355056511974,
                                             4.2295927091077825,
                                             -2.0235465771330334,
                                             0.06578021185658105,
                                             2.4410999256583756,
                                             -1.9544605670700543,
                                             1.4711270066881295,
                                             -1.0142146976172512,
                                             -0.5985126304888071,
                                             0.08733068149179224,
                                             1.3049368666841228,
                                             0.22045977993014249,
                                             0.0,
                                             0.0,
                                             0.0,
                                             0.0,
                                             0.0};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    param.Xinv = convert_vector<double, T>(expected_scores);
    param.expected_Xinv_transform = convert_vector<double, T>(A);
    param.k = 5;
    param.ldxinv_transform = 5;
    param.ldxinv = 5;

    param.epsilon = 100 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetSquareData3(std::vector<PCAParamType<T>> &params) {
    // Test with a square data matrix
    PCAParamType<T> param;
    param.test_name = "Square data matrix 3";
    param.n = 5;
    param.p = 5;
    std::vector<double> A{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0, 2.0, 3.0, 0.2, 0.1,
                          0.8, 6.0, 4.0, 1.0, 0.9, 3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1};
    param.A = convert_vector<double, T>(A);
    param.lda = 5;
    param.components_required = 5;
    param.method = "covariance";
    param.svd_solver = "gesvd";
    std::vector<double> expected_scores{-3.8337591301763827,
                                        6.682121109823703,
                                        -3.044191200205261,
                                        0.30775924641459096,
                                        -0.11193002585665025,
                                        0.32820531546508325,
                                        -0.9977540120299913,
                                        -2.832508506891677,
                                        -0.7275355056511974,
                                        4.2295927091077825,
                                        -2.0235465771330334,
                                        0.06578021185658105,
                                        2.4410999256583756,
                                        -1.9544605670700543,
                                        1.4711270066881295,
                                        -1.0142146976172512,
                                        -0.5985126304888071,
                                        0.08733068149179224,
                                        1.3049368666841228,
                                        0.22045977993014249,
                                        0.0,
                                        0.0,
                                        0.0,
                                        0.0,
                                        0.0};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        -0.4624602544702841,  0.8060536209817003,    -0.3672159333177392,
        0.03712450745593046,  -0.013501940649607386, 0.06253504179970557,
        -0.19010840442880456, -0.5396958231023441,   -0.13862195739333716,
        0.8058911431247803,   -0.5052213362240204,   0.016423425537536396,
        0.6094723888415844,   -0.4879725480256997,   0.3672980698705989,
        -0.5718140283399882,  -0.3374412923182887,   0.0492370194386037,
        0.735723124720095,    0.12429517649957769,   0.44721359549995804,
        0.44721359549995804,  0.447213595499958,     0.44721359549995787,
        0.447213595499958};
    param.expected_u = convert_vector<double, T>(expected_u);
    // Vanishing singular value means vt is very sensitive to sign flip in last row so we won't test it here
    std::vector<double> expected_variance{17.180698935602862, 6.886274659039021,
                                          4.010541852796752, 0.7864845525613813, 0.0};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{8.289921335116, 5.2483424655938835,
                                       4.005267458133977, 1.7736792861860695, 0.0};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)28.864;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{5.0, 5.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0, 2.0, 3.0, 0.2, 0.1,
                          0.8, 6.0, 4.0, 1.0, 0.9, 3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1};
    param.X = convert_vector<double, T>(X);
    param.m = 5;
    param.ldx = 5;
    param.ldx_transform = 5;
    std::vector<double> expected_X_transform{-3.8337591301763827,
                                             6.682121109823703,
                                             -3.044191200205261,
                                             0.30775924641459096,
                                             -0.11193002585665025,
                                             0.32820531546508325,
                                             -0.9977540120299913,
                                             -2.832508506891677,
                                             -0.7275355056511974,
                                             4.2295927091077825,
                                             -2.0235465771330334,
                                             0.06578021185658105,
                                             2.4410999256583756,
                                             -1.9544605670700543,
                                             1.4711270066881295,
                                             -1.0142146976172512,
                                             -0.5985126304888071,
                                             0.08733068149179224,
                                             1.3049368666841228,
                                             0.22045977993014249,
                                             0.0,
                                             0.0,
                                             0.0,
                                             0.0,
                                             0.0};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    param.Xinv = convert_vector<double, T>(expected_scores);
    param.expected_Xinv_transform = convert_vector<double, T>(A);
    param.k = 5;
    param.ldxinv_transform = 5;
    param.ldxinv = 5;

    param.epsilon = 100 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetTallThinData1(std::vector<PCAParamType<T>> &params) {
    // Test with a tall thin data matrix
    PCAParamType<T> param;
    param.test_name = "Tall thin data matrix 1";
    param.n = 8;
    param.p = 5;
    std::vector<double> A{1.0,  3.0, 0.0, -7.0, 1.0,  2.0, 2.0,  5.5, 1.0, 2.0,
                          3.0,  0.2, 0.1, 0.8,  6.0,  4.0, 1.0,  0.9, 3.1, 0.0,
                          -7.8, 9.8, 0.7, 4.0,  4.1,  1.1, 3.0,  2.1, 6.2, 0.6,
                          2.0,  2.0, 5.5, 1.0,  -2.0, 3.0, -0.2, 0.1, 0.8, 6.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 8;
    param.components_required = 3;
    param.method = "correlation";
    param.svd_solver = "gesvdx";
    std::vector<double> expected_scores{
        0.6267979871027662,   -0.5840661299986506, -0.029499040897586454,
        1.170456209881111,    2.7265278206031027,  -1.4135093389270788,
        -0.9926217969782473,  -1.504085710785417,  -0.4195179292574332,
        0.0435634967928295,   0.31139177600800805, 1.7483581905660086,
        -0.9310388558598546,  1.3826222180401109,  -0.9145007073597778,
        -1.2208781889298912,  1.396660839144611,   -0.28269291608599634,
        -1.3489020461078889,  0.8035118542854264,  -0.5671754542756104,
        -0.24748367799132698, -0.8877960299693047, 1.133877431000091};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.16301441689926877,  -0.15190093390764553, -0.007671959786015266,
        0.30440626882328325,  0.7091014202034718,   -0.36761828437252125,
        -0.2581560036334061,  -0.3911749242264357,  -0.1444301519035428,
        0.014997886908849872, 0.10720486151797456,  0.601918586631605,
        -0.3205347709881148,  0.4760042969562981,   -0.3148410755975569,
        -0.42031963352551305, 0.5296868400048295,   -0.10721193951786444,
        -0.5115742077486266,  0.3047337213689863,   -0.2151025973403451,
        -0.09385875487730042, -0.3366986891185997,  0.43002562722892085};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{2.1120544797846836, 1.205278130973026,
                                          0.9932201143631529};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{3.8450463402269657, 2.9046423044518215,
                                       2.6367671115481683};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)5.0;
    param.expected_n_components = 3;
    std::vector<double> expected_rinfo{8.0, 5.0, 3.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{0.1, 1.2, 3.1, 0.6, 5.1, -0.4, 0.1, -0.9, 12.3, 1.1};
    param.X = convert_vector<double, T>(X);
    param.m = 2;
    param.ldx = 2;
    param.ldx_transform = 2;
    std::vector<double> expected_X_transform{
        -1.7253499234437553, -0.6034681631460469, -0.046898565475484856,
        1.0339127620777953,  3.5129486393005163,  -0.035833080502485085};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{1.1, 1.5, 4.1, 3.6, 5.2, -1.4};
    param.k = 2;
    param.ldxinv_transform = 2;
    param.ldxinv = 2;
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector<double> expected_Xinv_transform{
        -11.12944237741829, -8.929412948858758, -5.998571520336921, -1.965471274648701,
        8.213628377799804,  3.4138252479100837, 0.64114064861087,   1.5397586079913463,
        12.838600918423584, -4.694421617930349};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetTallThinData2(std::vector<PCAParamType<T>> &params) {
    // Test with a tall thin data matrix
    PCAParamType<T> param;
    param.test_name = "Tall thin data matrix 2";
    param.n = 8;
    param.p = 5;
    std::vector<double> A{1.0,  3.0, 0.0, -7.0, 1.0,  2.0, 2.0,  5.5, 1.0, 2.0,
                          3.0,  0.2, 0.1, 0.8,  6.0,  4.0, 1.0,  0.9, 3.1, 0.0,
                          -7.8, 9.8, 0.7, 4.0,  4.1,  1.1, 3.0,  2.1, 6.2, 0.6,
                          2.0,  2.0, 5.5, 1.0,  -2.0, 3.0, -0.2, 0.1, 0.8, 6.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 8;
    param.components_required = 3;
    param.method = "correlation";
    param.svd_solver = "auto";
    std::vector<double> expected_scores{
        0.6267979871027662,   -0.5840661299986506, -0.029499040897586454,
        1.170456209881111,    2.7265278206031027,  -1.4135093389270788,
        -0.9926217969782473,  -1.504085710785417,  -0.4195179292574332,
        0.0435634967928295,   0.31139177600800805, 1.7483581905660086,
        -0.9310388558598546,  1.3826222180401109,  -0.9145007073597778,
        -1.2208781889298912,  1.396660839144611,   -0.28269291608599634,
        -1.3489020461078889,  0.8035118542854264,  -0.5671754542756104,
        -0.24748367799132698, -0.8877960299693047, 1.133877431000091};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.16301441689926877,  -0.15190093390764553, -0.007671959786015266,
        0.30440626882328325,  0.7091014202034718,   -0.36761828437252125,
        -0.2581560036334061,  -0.3911749242264357,  -0.1444301519035428,
        0.014997886908849872, 0.10720486151797456,  0.601918586631605,
        -0.3205347709881148,  0.4760042969562981,   -0.3148410755975569,
        -0.42031963352551305, 0.5296868400048295,   -0.10721193951786444,
        -0.5115742077486266,  0.3047337213689863,   -0.2151025973403451,
        -0.09385875487730042, -0.3366986891185997,  0.43002562722892085};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{2.1120544797846836, 1.205278130973026,
                                          0.9932201143631529};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{3.8450463402269657, 2.9046423044518215,
                                       2.6367671115481683};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)5.0;
    param.expected_n_components = 3;
    std::vector<double> expected_rinfo{8.0, 5.0, 3.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{0.1, 1.2, 3.1, 0.6, 5.1, -0.4, 0.1, -0.9, 12.3, 1.1};
    param.X = convert_vector<double, T>(X);
    param.m = 2;
    param.ldx = 2;
    param.ldx_transform = 2;
    std::vector<double> expected_X_transform{
        -1.7253499234437553, -0.6034681631460469, -0.046898565475484856,
        1.0339127620777953,  3.5129486393005163,  -0.035833080502485085};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{1.1, 1.5, 4.1, 3.6, 5.2, -1.4};
    param.k = 2;
    param.ldxinv_transform = 2;
    param.ldxinv = 2;
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector<double> expected_Xinv_transform{
        -11.12944237741829, -8.929412948858758, -5.998571520336921, -1.965471274648701,
        8.213628377799804,  3.4138252479100837, 0.64114064861087,   1.5397586079913463,
        12.838600918423584, -4.694421617930349};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetTallThinData3(std::vector<PCAParamType<T>> &params) {
    // Test with a tall thin data matrix
    PCAParamType<T> param;
    param.test_name = "Tall thin data matrix 3";
    param.n = 8;
    param.p = 5;
    std::vector<double> A{1.0,  3.0, 0.0, -7.0, 1.0,  2.0, 2.0,  5.5, 1.0, 2.0,
                          3.0,  0.2, 0.1, 0.8,  6.0,  4.0, 1.0,  0.9, 3.1, 0.0,
                          -7.8, 9.8, 0.7, 4.0,  4.1,  1.1, 3.0,  2.1, 6.2, 0.6,
                          2.0,  2.0, 5.5, 1.0,  -2.0, 3.0, -0.2, 0.1, 0.8, 6.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 8;
    param.components_required = 3;
    param.method = "correlation";
    param.svd_solver = "gesdd";
    std::vector<double> expected_scores{
        0.6267979871027662,   -0.5840661299986506, -0.029499040897586454,
        1.170456209881111,    2.7265278206031027,  -1.4135093389270788,
        -0.9926217969782473,  -1.504085710785417,  -0.4195179292574332,
        0.0435634967928295,   0.31139177600800805, 1.7483581905660086,
        -0.9310388558598546,  1.3826222180401109,  -0.9145007073597778,
        -1.2208781889298912,  1.396660839144611,   -0.28269291608599634,
        -1.3489020461078889,  0.8035118542854264,  -0.5671754542756104,
        -0.24748367799132698, -0.8877960299693047, 1.133877431000091};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.16301441689926877,  -0.15190093390764553, -0.007671959786015266,
        0.30440626882328325,  0.7091014202034718,   -0.36761828437252125,
        -0.2581560036334061,  -0.3911749242264357,  -0.1444301519035428,
        0.014997886908849872, 0.10720486151797456,  0.601918586631605,
        -0.3205347709881148,  0.4760042969562981,   -0.3148410755975569,
        -0.42031963352551305, 0.5296868400048295,   -0.10721193951786444,
        -0.5115742077486266,  0.3047337213689863,   -0.2151025973403451,
        -0.09385875487730042, -0.3366986891185997,  0.43002562722892085};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{2.1120544797846836, 1.205278130973026,
                                          0.9932201143631529};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{3.8450463402269657, 2.9046423044518215,
                                       2.6367671115481683};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)5.0;
    param.expected_n_components = 3;
    std::vector<double> expected_rinfo{8.0, 5.0, 3.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{0.1, 1.2, 3.1, 0.6, 5.1, -0.4, 0.1, -0.9, 12.3, 1.1};
    param.X = convert_vector<double, T>(X);
    param.m = 2;
    param.ldx = 2;
    param.ldx_transform = 2;
    std::vector<double> expected_X_transform{
        -1.7253499234437553, -0.6034681631460469, -0.046898565475484856,
        1.0339127620777953,  3.5129486393005163,  -0.035833080502485085};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{1.1, 1.5, 4.1, 3.6, 5.2, -1.4};
    param.k = 2;
    param.ldxinv_transform = 2;
    param.ldxinv = 2;
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector<double> expected_Xinv_transform{
        -11.12944237741829, -8.929412948858758, -5.998571520336921, -1.965471274648701,
        8.213628377799804,  3.4138252479100837, 0.64114064861087,   1.5397586079913463,
        12.838600918423584, -4.694421617930349};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetTallThinData4(std::vector<PCAParamType<T>> &params) {
    // Test with a tall thin data matrix
    PCAParamType<T> param;
    param.test_name = "Tall thin data matrix 4";
    param.n = 8;
    param.p = 5;
    std::vector<double> A{1.0,  3.0, 0.0, -7.0, 1.0,  2.0, 2.0,  5.5, 1.0, 2.0,
                          3.0,  0.2, 0.1, 0.8,  6.0,  4.0, 1.0,  0.9, 3.1, 0.0,
                          -7.8, 9.8, 0.7, 4.0,  4.1,  1.1, 3.0,  2.1, 6.2, 0.6,
                          2.0,  2.0, 5.5, 1.0,  -2.0, 3.0, -0.2, 0.1, 0.8, 6.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 8;
    param.components_required = 3;
    param.method = "correlation";
    param.svd_solver = "gesvd";
    std::vector<double> expected_scores{
        0.6267979871027662,   -0.5840661299986506, -0.029499040897586454,
        1.170456209881111,    2.7265278206031027,  -1.4135093389270788,
        -0.9926217969782473,  -1.504085710785417,  -0.4195179292574332,
        0.0435634967928295,   0.31139177600800805, 1.7483581905660086,
        -0.9310388558598546,  1.3826222180401109,  -0.9145007073597778,
        -1.2208781889298912,  1.396660839144611,   -0.28269291608599634,
        -1.3489020461078889,  0.8035118542854264,  -0.5671754542756104,
        -0.24748367799132698, -0.8877960299693047, 1.133877431000091};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.16301441689926877,  -0.15190093390764553, -0.007671959786015266,
        0.30440626882328325,  0.7091014202034718,   -0.36761828437252125,
        -0.2581560036334061,  -0.3911749242264357,  -0.1444301519035428,
        0.014997886908849872, 0.10720486151797456,  0.601918586631605,
        -0.3205347709881148,  0.4760042969562981,   -0.3148410755975569,
        -0.42031963352551305, 0.5296868400048295,   -0.10721193951786444,
        -0.5115742077486266,  0.3047337213689863,   -0.2151025973403451,
        -0.09385875487730042, -0.3366986891185997,  0.43002562722892085};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{2.1120544797846836, 1.205278130973026,
                                          0.9932201143631529};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{3.8450463402269657, 2.9046423044518215,
                                       2.6367671115481683};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)5.0;
    param.expected_n_components = 3;
    std::vector<double> expected_rinfo{8.0, 5.0, 3.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{0.1, 1.2, 3.1, 0.6, 5.1, -0.4, 0.1, -0.9, 12.3, 1.1};
    param.X = convert_vector<double, T>(X);
    param.m = 2;
    param.ldx = 2;
    param.ldx_transform = 2;
    std::vector<double> expected_X_transform{
        -1.7253499234437553, -0.6034681631460469, -0.046898565475484856,
        1.0339127620777953,  3.5129486393005163,  -0.035833080502485085};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{1.1, 1.5, 4.1, 3.6, 5.2, -1.4};
    param.k = 2;
    param.ldxinv_transform = 2;
    param.ldxinv = 2;
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector<double> expected_Xinv_transform{
        -11.12944237741829, -8.929412948858758, -5.998571520336921, -1.965471274648701,
        8.213628377799804,  3.4138252479100837, 0.64114064861087,   1.5397586079913463,
        12.838600918423584, -4.694421617930349};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetShortFatData(std::vector<PCAParamType<T>> &params) {
    // Test with a short wide data matrix
    PCAParamType<T> param;
    param.test_name = "Short fat data matrix";
    param.n = 6;
    param.p = 9;
    std::vector<double> A{
        1.06, 2.,  3.1,  3.,    3.,   0.2, -3.,  3.,   0.3,  2.,  0.27, 2.5,  2.1,  -0.25,
        0.08, 0.5, 0.15, 9.34,  3.1,  0.1, -9.8, 4.7,  0.86, 3.7, 1.,   0.86, 3.74, -2.9,
        7.2,  4.1, 2.,   -6.,   4.07, 2.,  4.2,  4.18, 2.,   4.6, 4.1,  5.5,  1.4,  -8.1,
        3.5,  1.4, 1.25, -1.34, 5.97, 2.1, -1.8, 4.9,  -1.,  2.1, 3.1,  -1.1};
    param.A = convert_vector<double, T>(A);
    param.lda = 6;
    param.components_required = 5;
    param.method = "covariance";
    param.svd_solver = "gesvdx";
    std::vector<double> expected_scores{
        1.3009267154927153e+00,  -6.5877230052241789e+00, -4.9809285397160119e+00,
        -3.5612426175979017e+00, 8.4719195235654121e-01,  1.2981775494688833e+01,
        -1.3805024563423629e+00, -4.4708351808125579e+00, 9.6493825675264251e+00,
        -6.4663830059584644e+00, 3.0707218487930961e+00,  -4.0238377320613716e-01,
        -3.2762216402250757e+00, 5.2165275979890122e+00,  -1.6410981315213631e+00,
        -4.1062116812116276e+00, 2.7682791641683364e+00,  1.0387246908007142e+00,
        2.4695323178734241e+00,  -2.1055988235567842e+00, -2.8228074618597740e+00,
        -3.8644094562644249e-01, 5.7239240030457763e+00,  -2.8786090898762011e+00,
        3.7817648862879909e+00,  1.2037550625224958e+00,  -1.7354062403349974e-01,
        -2.6701233382108560e+00, -1.6846024390261165e+00, -4.5725354754001774e-01};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.08197885180547565,   -0.4151302003020628,  -0.3138768677345451,
        -0.2244143173590145,   0.053386422683098,    0.8180561109070486,
        -0.10701566447228755,  -0.34657627403893654, 0.748013944101999,
        -0.5012698607928714,   0.23804038706952385,  -0.031192531867427452,
        -0.4025368318978638,   0.6409348094831617,   -0.2016354554848524,
        -0.5045145361848522,   0.34012788111509884,  0.12762413296930747,
        0.31961686718746113,   -0.2725151214535824,  -0.3653391660854761,
        -0.050014751173794614, 0.7408134101472293,   -0.3725612386218375,
        0.7422732690340185,    0.2362693695249406,   -0.03406202399800373,
        -0.5240836589720419,   -0.3306486249244244,  -0.08974833066448971};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{50.36536640208734, 33.28201854675696,
                                          13.248456875741875, 11.939882226186866,
                                          5.191482615893646};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{1.5869052650061903e+01, 1.2900003594332244e+01,
                                       8.1389363174010256e+00, 7.7265394020178482e+00,
                                       5.0948418110347875e+00};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)114.02720666666669;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{6.0, 9.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0,
                          2.0, 3.0, 0.2, 0.1, 0.8, 6.0, 4.0, 1.0, 0.9,
                          3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1, 0.1, 2.2};
    param.X = convert_vector<double, T>(X);
    param.m = 3;
    param.ldx = 3;
    param.ldx_transform = 3;
    std::vector<double> expected_X_transform{
        0.06883314625917261, 3.067184077786804,  1.187570713636344,   -0.7293564433902677,
        -2.8435147124468876, 1.4002843875775814, 0.4948410127545904,  -1.3134710855645326,
        4.089344441106332,   5.500616391996301,  -1.6234071319427146, 1.4456803905297901,
        0.33017954870069866, 0.8380569485849144, 0.13563129080749659};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    param.Xinv = convert_vector<double, T>(expected_X_transform);
    std::vector expected_Xinv_transform{
        2.3727464809747536,  1.167704861326678,    1.9491100025678718,
        -1.0410548955121248, 0.8795015790116714,   1.3394734576143197,
        0.16368327363466295, 4.246551067854854,    1.9975875461887138,
        3.4925903140035346,  2.977830659903403,    0.0060815050263896,
        4.146195117872927,   0.4575615602370817,   5.555985870600144,
        2.6255627330279974,  1.3563351426138448,   0.6747389661282728,
        2.6405060011039225,  -0.3622556749930357,  -0.031135412980347166,
        5.100726104247336,   1.0421504520475111,   4.393430813484434,
        2.0127690616095415,  -0.21397680461528235, 2.6382802438058706};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.k = 3;
    param.ldxinv_transform = 3;
    param.ldxinv = 3;

    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetSubarrayData1(std::vector<PCAParamType<T>> &params) {
    // Test with data matrices in subarrays
    PCAParamType<T> param;
    param.test_name = "Subarray data matrices 1";
    param.n = 6;
    param.p = 9;
    std::vector<double> A{
        1.06, 2.,  3.1,  3.,  3.,   0.2,   0.0,  0.0, -3.,  3.,   0.3,  2.,
        0.27, 2.5, 0.0,  0.0, 2.1,  -0.25, 0.08, 0.5, 0.15, 9.34, 0.0,  0.0,
        3.1,  0.1, -9.8, 4.7, 0.86, 3.7,   0.0,  0.0, 1.,   0.86, 3.74, -2.9,
        7.2,  4.1, 0.0,  0.0, 2.,   -6.,   4.07, 2.,  4.2,  4.18, 0.0,  0.0,
        2.,   4.6, 4.1,  5.5, 1.4,  -8.1,  0.0,  0.0, 3.5,  1.4,  1.25, -1.34,
        5.97, 2.1, 0.0,  0.0, -1.8, 4.9,   -1.,  2.1, 3.1,  -1.1, 0.0,  0.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 8;
    param.components_required = 5;
    param.method = "covariance";
    param.svd_solver = "gesvdx";
    std::vector<double> expected_scores{
        1.3009267154927153e+00,  -6.5877230052241789e+00, -4.9809285397160119e+00,
        -3.5612426175979017e+00, 8.4719195235654121e-01,  1.2981775494688833e+01,
        -1.3805024563423629e+00, -4.4708351808125579e+00, 9.6493825675264251e+00,
        -6.4663830059584644e+00, 3.0707218487930961e+00,  -4.0238377320613716e-01,
        -3.2762216402250757e+00, 5.2165275979890122e+00,  -1.6410981315213631e+00,
        -4.1062116812116276e+00, 2.7682791641683364e+00,  1.0387246908007142e+00,
        2.4695323178734241e+00,  -2.1055988235567842e+00, -2.8228074618597740e+00,
        -3.8644094562644249e-01, 5.7239240030457763e+00,  -2.8786090898762011e+00,
        3.7817648862879909e+00,  1.2037550625224958e+00,  -1.7354062403349974e-01,
        -2.6701233382108560e+00, -1.6846024390261165e+00, -4.5725354754001774e-01};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.08197885180547565,   -0.4151302003020628,  -0.3138768677345451,
        -0.2244143173590145,   0.053386422683098,    0.8180561109070486,
        -0.10701566447228755,  -0.34657627403893654, 0.748013944101999,
        -0.5012698607928714,   0.23804038706952385,  -0.031192531867427452,
        -0.4025368318978638,   0.6409348094831617,   -0.2016354554848524,
        -0.5045145361848522,   0.34012788111509884,  0.12762413296930747,
        0.31961686718746113,   -0.2725151214535824,  -0.3653391660854761,
        -0.050014751173794614, 0.7408134101472293,   -0.3725612386218375,
        0.7422732690340185,    0.2362693695249406,   -0.03406202399800373,
        -0.5240836589720419,   -0.3306486249244244,  -0.08974833066448971};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{50.36536640208734, 33.28201854675696,
                                          13.248456875741875, 11.939882226186866,
                                          5.191482615893646};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{1.5869052650061903e+01, 1.2900003594332244e+01,
                                       8.1389363174010256e+00, 7.7265394020178482e+00,
                                       5.0948418110347875e+00};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)114.02720666666669;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{6.0, 9.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 2.0, 5.5, 1.0, 0.0,
                          2.0, 3.0, 0.2, 0.0, 0.1, 0.8, 6.0, 0.0, 4.0, 1.0, 0.9, 0.0,
                          3.1, 0.0, 0.0, 0.0, 9.8, 0.7, 4.0, 0.0, 4.1, 0.1, 2.2, 0.0};
    param.X = convert_vector<double, T>(X);
    param.m = 3;
    param.ldx = 4;
    param.ldx_transform = 4;
    std::vector<double> expected_X_transform{
        0.06883314625917261, 3.067184077786804,   1.187570713636344,   0.0,
        -0.7293564433902677, -2.8435147124468876, 1.4002843875775814,  0.0,
        0.4948410127545904,  -1.3134710855645326, 4.089344441106332,   0.0,
        5.500616391996301,   -1.6234071319427146, 1.4456803905297901,  0.0,
        0.33017954870069866, 0.8380569485849144,  0.13563129080749659, 0.0};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{
        0.06883314625917261, 3.067184077786804,   1.187570713636344,   0.0, 0.0,
        -0.7293564433902677, -2.8435147124468876, 1.4002843875775814,  0.0, 0.0,
        0.4948410127545904,  -1.3134710855645326, 4.089344441106332,   0.0, 0.0,
        5.500616391996301,   -1.6234071319427146, 1.4456803905297901,  0.0, 0.0,
        0.33017954870069866, 0.8380569485849144,  0.13563129080749659, 0.0, 0.0};
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector expected_Xinv_transform{
        2.3727464809747536,  1.167704861326678,    1.9491100025678718,    0.0, 0.0,
        -1.0410548955121248, 0.8795015790116714,   1.3394734576143197,    0.0, 0.0,
        0.16368327363466295, 4.246551067854854,    1.9975875461887138,    0.0, 0.0,
        3.4925903140035346,  2.977830659903403,    0.0060815050263896,    0.0, 0.0,
        4.146195117872927,   0.4575615602370817,   5.555985870600144,     0.0, 0.0,
        2.6255627330279974,  1.3563351426138448,   0.6747389661282728,    0.0, 0.0,
        2.6405060011039225,  -0.3622556749930357,  -0.031135412980347166, 0.0, 0.0,
        5.100726104247336,   1.0421504520475111,   4.393430813484434,     0.0, 0.0,
        2.0127690616095415,  -0.21397680461528235, 2.6382802438058706,    0.0, 0.0};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.k = 3;
    param.ldxinv = 5;
    param.ldxinv_transform = 5;

    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetSubarrayData2(std::vector<PCAParamType<T>> &params) {
    // Test with data matrices in subarrays
    PCAParamType<T> param;
    param.test_name = "Subarray data matrices 2";
    param.n = 6;
    param.p = 9;
    std::vector<double> A{
        1.06, 2.,  3.1,  3.,  3.,   0.2,   0.0,  0.0, -3.,  3.,   0.3,  2.,
        0.27, 2.5, 0.0,  0.0, 2.1,  -0.25, 0.08, 0.5, 0.15, 9.34, 0.0,  0.0,
        3.1,  0.1, -9.8, 4.7, 0.86, 3.7,   0.0,  0.0, 1.,   0.86, 3.74, -2.9,
        7.2,  4.1, 0.0,  0.0, 2.,   -6.,   4.07, 2.,  4.2,  4.18, 0.0,  0.0,
        2.,   4.6, 4.1,  5.5, 1.4,  -8.1,  0.0,  0.0, 3.5,  1.4,  1.25, -1.34,
        5.97, 2.1, 0.0,  0.0, -1.8, 4.9,   -1.,  2.1, 3.1,  -1.1, 0.0,  0.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 8;
    param.components_required = 5;
    param.method = "covariance";
    param.svd_solver = "auto";
    std::vector<double> expected_scores{
        1.3009267154927153e+00,  -6.5877230052241789e+00, -4.9809285397160119e+00,
        -3.5612426175979017e+00, 8.4719195235654121e-01,  1.2981775494688833e+01,
        -1.3805024563423629e+00, -4.4708351808125579e+00, 9.6493825675264251e+00,
        -6.4663830059584644e+00, 3.0707218487930961e+00,  -4.0238377320613716e-01,
        -3.2762216402250757e+00, 5.2165275979890122e+00,  -1.6410981315213631e+00,
        -4.1062116812116276e+00, 2.7682791641683364e+00,  1.0387246908007142e+00,
        2.4695323178734241e+00,  -2.1055988235567842e+00, -2.8228074618597740e+00,
        -3.8644094562644249e-01, 5.7239240030457763e+00,  -2.8786090898762011e+00,
        3.7817648862879909e+00,  1.2037550625224958e+00,  -1.7354062403349974e-01,
        -2.6701233382108560e+00, -1.6846024390261165e+00, -4.5725354754001774e-01};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.08197885180547565,   -0.4151302003020628,  -0.3138768677345451,
        -0.2244143173590145,   0.053386422683098,    0.8180561109070486,
        -0.10701566447228755,  -0.34657627403893654, 0.748013944101999,
        -0.5012698607928714,   0.23804038706952385,  -0.031192531867427452,
        -0.4025368318978638,   0.6409348094831617,   -0.2016354554848524,
        -0.5045145361848522,   0.34012788111509884,  0.12762413296930747,
        0.31961686718746113,   -0.2725151214535824,  -0.3653391660854761,
        -0.050014751173794614, 0.7408134101472293,   -0.3725612386218375,
        0.7422732690340185,    0.2362693695249406,   -0.03406202399800373,
        -0.5240836589720419,   -0.3306486249244244,  -0.08974833066448971};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{50.36536640208734, 33.28201854675696,
                                          13.248456875741875, 11.939882226186866,
                                          5.191482615893646};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{1.5869052650061903e+01, 1.2900003594332244e+01,
                                       8.1389363174010256e+00, 7.7265394020178482e+00,
                                       5.0948418110347875e+00};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)114.02720666666669;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{6.0, 9.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 2.0, 5.5, 1.0, 0.0,
                          2.0, 3.0, 0.2, 0.0, 0.1, 0.8, 6.0, 0.0, 4.0, 1.0, 0.9, 0.0,
                          3.1, 0.0, 0.0, 0.0, 9.8, 0.7, 4.0, 0.0, 4.1, 0.1, 2.2, 0.0};
    param.X = convert_vector<double, T>(X);
    param.m = 3;
    param.ldx = 4;
    param.ldx_transform = 4;
    std::vector<double> expected_X_transform{
        0.06883314625917261, 3.067184077786804,   1.187570713636344,   0.0,
        -0.7293564433902677, -2.8435147124468876, 1.4002843875775814,  0.0,
        0.4948410127545904,  -1.3134710855645326, 4.089344441106332,   0.0,
        5.500616391996301,   -1.6234071319427146, 1.4456803905297901,  0.0,
        0.33017954870069866, 0.8380569485849144,  0.13563129080749659, 0.0};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{
        0.06883314625917261, 3.067184077786804,   1.187570713636344,   0.0, 0.0,
        -0.7293564433902677, -2.8435147124468876, 1.4002843875775814,  0.0, 0.0,
        0.4948410127545904,  -1.3134710855645326, 4.089344441106332,   0.0, 0.0,
        5.500616391996301,   -1.6234071319427146, 1.4456803905297901,  0.0, 0.0,
        0.33017954870069866, 0.8380569485849144,  0.13563129080749659, 0.0, 0.0};
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector expected_Xinv_transform{
        2.3727464809747536,  1.167704861326678,    1.9491100025678718,    0.0, 0.0,
        -1.0410548955121248, 0.8795015790116714,   1.3394734576143197,    0.0, 0.0,
        0.16368327363466295, 4.246551067854854,    1.9975875461887138,    0.0, 0.0,
        3.4925903140035346,  2.977830659903403,    0.0060815050263896,    0.0, 0.0,
        4.146195117872927,   0.4575615602370817,   5.555985870600144,     0.0, 0.0,
        2.6255627330279974,  1.3563351426138448,   0.6747389661282728,    0.0, 0.0,
        2.6405060011039225,  -0.3622556749930357,  -0.031135412980347166, 0.0, 0.0,
        5.100726104247336,   1.0421504520475111,   4.393430813484434,     0.0, 0.0,
        2.0127690616095415,  -0.21397680461528235, 2.6382802438058706,    0.0, 0.0};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.k = 3;
    param.ldxinv = 5;
    param.ldxinv_transform = 5;

    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetSubarrayData3(std::vector<PCAParamType<T>> &params) {
    // Test with data matrices in subarrays
    PCAParamType<T> param;
    param.test_name = "Subarray data matrices 3";
    param.n = 6;
    param.p = 9;
    std::vector<double> A{
        1.06, 2.,  3.1,  3.,  3.,   0.2,   0.0,  0.0, -3.,  3.,   0.3,  2.,
        0.27, 2.5, 0.0,  0.0, 2.1,  -0.25, 0.08, 0.5, 0.15, 9.34, 0.0,  0.0,
        3.1,  0.1, -9.8, 4.7, 0.86, 3.7,   0.0,  0.0, 1.,   0.86, 3.74, -2.9,
        7.2,  4.1, 0.0,  0.0, 2.,   -6.,   4.07, 2.,  4.2,  4.18, 0.0,  0.0,
        2.,   4.6, 4.1,  5.5, 1.4,  -8.1,  0.0,  0.0, 3.5,  1.4,  1.25, -1.34,
        5.97, 2.1, 0.0,  0.0, -1.8, 4.9,   -1.,  2.1, 3.1,  -1.1, 0.0,  0.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 8;
    param.components_required = 5;
    param.method = "covariance";
    param.svd_solver = "gesdd";
    std::vector<double> expected_scores{
        1.3009267154927153e+00,  -6.5877230052241789e+00, -4.9809285397160119e+00,
        -3.5612426175979017e+00, 8.4719195235654121e-01,  1.2981775494688833e+01,
        -1.3805024563423629e+00, -4.4708351808125579e+00, 9.6493825675264251e+00,
        -6.4663830059584644e+00, 3.0707218487930961e+00,  -4.0238377320613716e-01,
        -3.2762216402250757e+00, 5.2165275979890122e+00,  -1.6410981315213631e+00,
        -4.1062116812116276e+00, 2.7682791641683364e+00,  1.0387246908007142e+00,
        2.4695323178734241e+00,  -2.1055988235567842e+00, -2.8228074618597740e+00,
        -3.8644094562644249e-01, 5.7239240030457763e+00,  -2.8786090898762011e+00,
        3.7817648862879909e+00,  1.2037550625224958e+00,  -1.7354062403349974e-01,
        -2.6701233382108560e+00, -1.6846024390261165e+00, -4.5725354754001774e-01};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.08197885180547565,   -0.4151302003020628,  -0.3138768677345451,
        -0.2244143173590145,   0.053386422683098,    0.8180561109070486,
        -0.10701566447228755,  -0.34657627403893654, 0.748013944101999,
        -0.5012698607928714,   0.23804038706952385,  -0.031192531867427452,
        -0.4025368318978638,   0.6409348094831617,   -0.2016354554848524,
        -0.5045145361848522,   0.34012788111509884,  0.12762413296930747,
        0.31961686718746113,   -0.2725151214535824,  -0.3653391660854761,
        -0.050014751173794614, 0.7408134101472293,   -0.3725612386218375,
        0.7422732690340185,    0.2362693695249406,   -0.03406202399800373,
        -0.5240836589720419,   -0.3306486249244244,  -0.08974833066448971};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{50.36536640208734, 33.28201854675696,
                                          13.248456875741875, 11.939882226186866,
                                          5.191482615893646};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{1.5869052650061903e+01, 1.2900003594332244e+01,
                                       8.1389363174010256e+00, 7.7265394020178482e+00,
                                       5.0948418110347875e+00};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)114.02720666666669;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{6.0, 9.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 2.0, 5.5, 1.0, 0.0,
                          2.0, 3.0, 0.2, 0.0, 0.1, 0.8, 6.0, 0.0, 4.0, 1.0, 0.9, 0.0,
                          3.1, 0.0, 0.0, 0.0, 9.8, 0.7, 4.0, 0.0, 4.1, 0.1, 2.2, 0.0};
    param.X = convert_vector<double, T>(X);
    param.m = 3;
    param.ldx = 4;
    param.ldx_transform = 4;
    std::vector<double> expected_X_transform{
        0.06883314625917261, 3.067184077786804,   1.187570713636344,   0.0,
        -0.7293564433902677, -2.8435147124468876, 1.4002843875775814,  0.0,
        0.4948410127545904,  -1.3134710855645326, 4.089344441106332,   0.0,
        5.500616391996301,   -1.6234071319427146, 1.4456803905297901,  0.0,
        0.33017954870069866, 0.8380569485849144,  0.13563129080749659, 0.0};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{
        0.06883314625917261, 3.067184077786804,   1.187570713636344,   0.0, 0.0,
        -0.7293564433902677, -2.8435147124468876, 1.4002843875775814,  0.0, 0.0,
        0.4948410127545904,  -1.3134710855645326, 4.089344441106332,   0.0, 0.0,
        5.500616391996301,   -1.6234071319427146, 1.4456803905297901,  0.0, 0.0,
        0.33017954870069866, 0.8380569485849144,  0.13563129080749659, 0.0, 0.0};
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector expected_Xinv_transform{
        2.3727464809747536,  1.167704861326678,    1.9491100025678718,    0.0, 0.0,
        -1.0410548955121248, 0.8795015790116714,   1.3394734576143197,    0.0, 0.0,
        0.16368327363466295, 4.246551067854854,    1.9975875461887138,    0.0, 0.0,
        3.4925903140035346,  2.977830659903403,    0.0060815050263896,    0.0, 0.0,
        4.146195117872927,   0.4575615602370817,   5.555985870600144,     0.0, 0.0,
        2.6255627330279974,  1.3563351426138448,   0.6747389661282728,    0.0, 0.0,
        2.6405060011039225,  -0.3622556749930357,  -0.031135412980347166, 0.0, 0.0,
        5.100726104247336,   1.0421504520475111,   4.393430813484434,     0.0, 0.0,
        2.0127690616095415,  -0.21397680461528235, 2.6382802438058706,    0.0, 0.0};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.k = 3;
    param.ldxinv = 5;
    param.ldxinv_transform = 5;

    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetSubarrayData4(std::vector<PCAParamType<T>> &params) {
    // Test with data matrices in subarrays
    PCAParamType<T> param;
    param.test_name = "Subarray data matrices 4";
    param.n = 6;
    param.p = 9;
    std::vector<double> A{
        1.06, 2.,  3.1,  3.,  3.,   0.2,   0.0,  0.0, -3.,  3.,   0.3,  2.,
        0.27, 2.5, 0.0,  0.0, 2.1,  -0.25, 0.08, 0.5, 0.15, 9.34, 0.0,  0.0,
        3.1,  0.1, -9.8, 4.7, 0.86, 3.7,   0.0,  0.0, 1.,   0.86, 3.74, -2.9,
        7.2,  4.1, 0.0,  0.0, 2.,   -6.,   4.07, 2.,  4.2,  4.18, 0.0,  0.0,
        2.,   4.6, 4.1,  5.5, 1.4,  -8.1,  0.0,  0.0, 3.5,  1.4,  1.25, -1.34,
        5.97, 2.1, 0.0,  0.0, -1.8, 4.9,   -1.,  2.1, 3.1,  -1.1, 0.0,  0.0};
    param.A = convert_vector<double, T>(A);
    param.lda = 8;
    param.components_required = 5;
    param.method = "covariance";
    param.svd_solver = "gesvd";
    std::vector<double> expected_scores{
        1.3009267154927153e+00,  -6.5877230052241789e+00, -4.9809285397160119e+00,
        -3.5612426175979017e+00, 8.4719195235654121e-01,  1.2981775494688833e+01,
        -1.3805024563423629e+00, -4.4708351808125579e+00, 9.6493825675264251e+00,
        -6.4663830059584644e+00, 3.0707218487930961e+00,  -4.0238377320613716e-01,
        -3.2762216402250757e+00, 5.2165275979890122e+00,  -1.6410981315213631e+00,
        -4.1062116812116276e+00, 2.7682791641683364e+00,  1.0387246908007142e+00,
        2.4695323178734241e+00,  -2.1055988235567842e+00, -2.8228074618597740e+00,
        -3.8644094562644249e-01, 5.7239240030457763e+00,  -2.8786090898762011e+00,
        3.7817648862879909e+00,  1.2037550625224958e+00,  -1.7354062403349974e-01,
        -2.6701233382108560e+00, -1.6846024390261165e+00, -4.5725354754001774e-01};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.08197885180547565,   -0.4151302003020628,  -0.3138768677345451,
        -0.2244143173590145,   0.053386422683098,    0.8180561109070486,
        -0.10701566447228755,  -0.34657627403893654, 0.748013944101999,
        -0.5012698607928714,   0.23804038706952385,  -0.031192531867427452,
        -0.4025368318978638,   0.6409348094831617,   -0.2016354554848524,
        -0.5045145361848522,   0.34012788111509884,  0.12762413296930747,
        0.31961686718746113,   -0.2725151214535824,  -0.3653391660854761,
        -0.050014751173794614, 0.7408134101472293,   -0.3725612386218375,
        0.7422732690340185,    0.2362693695249406,   -0.03406202399800373,
        -0.5240836589720419,   -0.3306486249244244,  -0.08974833066448971};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{50.36536640208734, 33.28201854675696,
                                          13.248456875741875, 11.939882226186866,
                                          5.191482615893646};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{1.5869052650061903e+01, 1.2900003594332244e+01,
                                       8.1389363174010256e+00, 7.7265394020178482e+00,
                                       5.0948418110347875e+00};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)114.02720666666669;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{6.0, 9.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 2.0, 5.5, 1.0, 0.0,
                          2.0, 3.0, 0.2, 0.0, 0.1, 0.8, 6.0, 0.0, 4.0, 1.0, 0.9, 0.0,
                          3.1, 0.0, 0.0, 0.0, 9.8, 0.7, 4.0, 0.0, 4.1, 0.1, 2.2, 0.0};
    param.X = convert_vector<double, T>(X);
    param.m = 3;
    param.ldx = 4;
    param.ldx_transform = 4;
    std::vector<double> expected_X_transform{
        0.06883314625917261, 3.067184077786804,   1.187570713636344,   0.0,
        -0.7293564433902677, -2.8435147124468876, 1.4002843875775814,  0.0,
        0.4948410127545904,  -1.3134710855645326, 4.089344441106332,   0.0,
        5.500616391996301,   -1.6234071319427146, 1.4456803905297901,  0.0,
        0.33017954870069866, 0.8380569485849144,  0.13563129080749659, 0.0};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{
        0.06883314625917261, 3.067184077786804,   1.187570713636344,   0.0, 0.0,
        -0.7293564433902677, -2.8435147124468876, 1.4002843875775814,  0.0, 0.0,
        0.4948410127545904,  -1.3134710855645326, 4.089344441106332,   0.0, 0.0,
        5.500616391996301,   -1.6234071319427146, 1.4456803905297901,  0.0, 0.0,
        0.33017954870069866, 0.8380569485849144,  0.13563129080749659, 0.0, 0.0};
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector expected_Xinv_transform{
        2.3727464809747536,  1.167704861326678,    1.9491100025678718,    0.0, 0.0,
        -1.0410548955121248, 0.8795015790116714,   1.3394734576143197,    0.0, 0.0,
        0.16368327363466295, 4.246551067854854,    1.9975875461887138,    0.0, 0.0,
        3.4925903140035346,  2.977830659903403,    0.0060815050263896,    0.0, 0.0,
        4.146195117872927,   0.4575615602370817,   5.555985870600144,     0.0, 0.0,
        2.6255627330279974,  1.3563351426138448,   0.6747389661282728,    0.0, 0.0,
        2.6405060011039225,  -0.3622556749930357,  -0.031135412980347166, 0.0, 0.0,
        5.100726104247336,   1.0421504520475111,   4.393430813484434,     0.0, 0.0,
        2.0127690616095415,  -0.21397680461528235, 2.6382802438058706,    0.0, 0.0};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.k = 3;
    param.ldxinv = 5;
    param.ldxinv_transform = 5;

    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetBiasedData(std::vector<PCAParamType<T>> &params) {
    // Test with biased standard deviation estimators
    PCAParamType<T> param;
    param.test_name = "Test with biased standard deviation estimators";
    param.n = 6;
    param.p = 9;
    std::vector<double> A{
        1.06, 2.,  3.1,  3.,    3.,   0.2, -3.,  3.,   0.3,  2.,  0.27, 2.5,  2.1,  -0.25,
        0.08, 0.5, 0.15, 9.34,  3.1,  0.1, -9.8, 4.7,  0.86, 3.7, 1.,   0.86, 3.74, -2.9,
        7.2,  4.1, 2.,   -6.,   4.07, 2.,  4.2,  4.18, 2.,   4.6, 4.1,  5.5,  1.4,  -8.1,
        3.5,  1.4, 1.25, -1.34, 5.97, 2.1, -1.8, 4.9,  -1.,  2.1, 3.1,  -1.1};
    param.A = convert_vector<double, T>(A);
    param.lda = 6;
    param.components_required = 5;
    param.method = "covariance";
    param.degrees_of_freedom = "biased";
    param.svd_solver = "gesvdx";
    std::vector<double> expected_scores{
        1.3009267154927153e+00,  -6.5877230052241789e+00, -4.9809285397160119e+00,
        -3.5612426175979017e+00, 8.4719195235654121e-01,  1.2981775494688833e+01,
        -1.3805024563423629e+00, -4.4708351808125579e+00, 9.6493825675264251e+00,
        -6.4663830059584644e+00, 3.0707218487930961e+00,  -4.0238377320613716e-01,
        -3.2762216402250757e+00, 5.2165275979890122e+00,  -1.6410981315213631e+00,
        -4.1062116812116276e+00, 2.7682791641683364e+00,  1.0387246908007142e+00,
        2.4695323178734241e+00,  -2.1055988235567842e+00, -2.8228074618597740e+00,
        -3.8644094562644249e-01, 5.7239240030457763e+00,  -2.8786090898762011e+00,
        3.7817648862879909e+00,  1.2037550625224958e+00,  -1.7354062403349974e-01,
        -2.6701233382108560e+00, -1.6846024390261165e+00, -4.5725354754001774e-01};
    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.08197885180547565,   -0.4151302003020628,  -0.3138768677345451,
        -0.2244143173590145,   0.053386422683098,    0.8180561109070486,
        -0.10701566447228755,  -0.34657627403893654, 0.748013944101999,
        -0.5012698607928714,   0.23804038706952385,  -0.031192531867427452,
        -0.4025368318978638,   0.6409348094831617,   -0.2016354554848524,
        -0.5045145361848522,   0.34012788111509884,  0.12762413296930747,
        0.31961686718746113,   -0.2725151214535824,  -0.3653391660854761,
        -0.050014751173794614, 0.7408134101472293,   -0.3725612386218375,
        0.7422732690340185,    0.2362693695249406,   -0.03406202399800373,
        -0.5240836589720419,   -0.3306486249244244,  -0.08974833066448971};
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.13018127893763679,  0.0555292207103647,   -0.029183724285447132,
        0.08530286540315635,   -0.2803601055672529,  0.001590249940405325,
        -0.11709514506480873,  0.30369939990408595,  -0.3516982898193114,
        -0.5672489325972923,   0.4907207154557312,   -0.045311214609932704,
        -0.003799092237743996, -0.34731075906009634, 0.06812636032930122,
        0.3343984977339394,    -0.7723725615550512,  -0.10004283763297586,
        0.46171229284752713,   -0.08265966786479861, 0.18528397866178742,
        0.4210994868036967,    0.4705556347326492,   0.34559891508413954,
        -0.08061539539830952,  0.28811606078251584,  0.3702858029266968,
        -0.5551538609206681,   0.2901007531747837,   -0.566012015390133,
        -0.6817249186902983,   -0.07073588750344964, -0.24768412253798605,
        0.21582459928821646,   -0.03664007407703801, 0.10402307019919266,
        0.16298865444422567,   0.27165823936121647,  0.5161121847794891,
        0.2798878551432686,    -0.19367614393273275, -0.1964370936029322,
        0.4817965795096736,    0.1366734331575607,   -0.4261512501260775};
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{41.97113866840612, 27.735015455630798,
                                          11.040380729784895, 9.949901855155721,
                                          4.326235513244705};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{15.869052650061903, 12.900003594332244,
                                       8.138936317401026, 7.726539402017848,
                                       5.0948418110347875};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)95.02267222222224;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{6.0, 9.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0,
                          2.0, 3.0, 0.2, 0.1, 0.8, 6.0, 4.0, 1.0, 0.9,
                          3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1, 0.1, 2.2};
    param.X = convert_vector<double, T>(X);
    param.m = 3;
    param.ldx = 3;
    param.ldx_transform = 3;
    std::vector<double> expected_X_transform{
        0.06883314625917261, 3.067184077786804,  1.187570713636344,   -0.7293564433902677,
        -2.8435147124468876, 1.4002843875775814, 0.4948410127545904,  -1.3134710855645326,
        4.089344441106332,   5.500616391996301,  -1.6234071319427146, 1.4456803905297901,
        0.33017954870069866, 0.8380569485849144, 0.13563129080749659};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    param.Xinv = convert_vector<double, T>(expected_X_transform);
    std::vector expected_Xinv_transform{
        2.3727464809747536,  1.167704861326678,    1.9491100025678718,
        -1.0410548955121248, 0.8795015790116714,   1.3394734576143197,
        0.16368327363466295, 4.246551067854854,    1.9975875461887138,
        3.4925903140035346,  2.977830659903403,    0.0060815050263896,
        4.146195117872927,   0.4575615602370817,   5.555985870600144,
        2.6255627330279974,  1.3563351426138448,   0.6747389661282728,
        2.6405060011039225,  -0.3622556749930357,  -0.031135412980347166,
        5.100726104247336,   1.0421504520475111,   4.393430813484434,
        2.0127690616095415,  -0.21397680461528235, 2.6382802438058706};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.k = 3;
    param.ldxinv_transform = 3;
    param.ldxinv = 3;

    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetBiasedData2(std::vector<PCAParamType<T>> &params) {
    // Test with biased standard deviation estimators
    PCAParamType<T> param;
    param.test_name = "Test with biased standard deviation estimators";
    param.n = 6;
    param.p = 9;
    std::vector<double> A{
        1.06, 2.,  3.1,  3.,    3.,   0.2, -3.,  3.,   0.3,  2.,  0.27, 2.5,  2.1,  -0.25,
        0.08, 0.5, 0.15, 9.34,  3.1,  0.1, -9.8, 4.7,  0.86, 3.7, 1.,   0.86, 3.74, -2.9,
        7.2,  4.1, 2.,   -6.,   4.07, 2.,  4.2,  4.18, 2.,   4.6, 4.1,  5.5,  1.4,  -8.1,
        3.5,  1.4, 1.25, -1.34, 5.97, 2.1, -1.8, 4.9,  -1.,  2.1, 3.1,  -1.1};
    param.A = convert_vector<double, T>(A);
    param.lda = 6;
    param.components_required = 5;
    param.method = "correlation";
    param.degrees_of_freedom = "biased";
    param.svd_solver = "gesvdx";
    std::vector<double> expected_components{
        -0.40635250744475376, 0.31778380450970445,  0.002007932392598666,
        -0.22816887619160775, 0.44802918080699444,  -0.1204520779874455,
        -0.4094386167172589,  0.39251376960613765,  -0.5550476718820487,
        0.1234881546130405,   0.46898925153399174,  -0.2835297198211863,
        0.011398990234394675, -0.18018307386084714, -0.0016759389359247683,
        0.13232671321700468,  -0.4124695723116655,  0.0628834959609175,
        0.5579793691376701,   0.5748019272935684,   0.2532613632920828,
        0.4142231829844192,   0.4884548821445952,   -0.16603890345156205,
        -0.05225524952723594, 0.31134786552739774,  0.3175946923662307,
        -0.2508653193089363,  -0.25976017652692707, 0.6432041265541919,
        -0.49277276186355445, 0.15537817805692902,  -0.19477713556526488,
        0.1605971375273296,   -0.01659378302939952, 0.21928772173344097,
        0.40517336339950466,  0.4348717100470041,   0.40659348730145506,
        -0.02017359074850547, -0.36182419525776555, -0.14176796556854757,
        0.5597066381358866,   0.0994995382900446,   0.19103920410625638};
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_variance{3.548146755863272, 2.2968628144014827,
                                          1.4547763969332148, 1.0703009336692777,
                                          0.6299130991327534};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{4.613987487540428, 3.7123007537656343,
                                       2.9544302972991745, 2.534128173951678,
                                       1.944088113948676};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)9;
    param.expected_n_components = 5;
    std::vector<double> expected_rinfo{6.0, 9.0, 5.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{1.0, 3.0, 0.0, 0.0, 0.0, 2.0, 2.0, 5.5, 1.0,
                          2.0, 3.0, 0.2, 0.1, 0.8, 6.0, 4.0, 1.0, 0.9,
                          3.1, 0.0, 0.0, 9.8, 0.7, 4.0, 4.1, 0.1, 2.2};
    param.X = convert_vector<double, T>(X);
    param.m = 3;
    param.ldx = 3;
    param.ldx_transform = 3;
    std::vector<double> expected_X_transform{
        0.6429034123729505,  0.24096546653427056,  0.9486871853283014,
        0.9054189248061256,  -0.5988370987336112,  -0.10428390914306929,
        1.465532698605996,   -0.7274897159666929,  1.5313368163487433,
        2.1538367434798773,  -0.07601632393903783, 0.32738886162785436,
        0.30451771323422944, 0.4705163595014952,   -0.9257726579998535};
    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    param.Xinv = convert_vector<double, T>(expected_X_transform);
    std::vector expected_Xinv_transform{
        1.7005806627235573,  1.9920763829564059,  1.06089766239757,
        -1.2220993538396985, 0.9069823728445536,  1.3131685754995583,
        0.8830498925136328,  2.9564204362327033,  3.452728700709806,
        6.166545812927968,   2.678985692441437,   0.033154810793458334,
        5.1195055466382975,  0.5786086289139383,  5.30593097182525,
        0.8665114197881643,  3.1421116629773262,  -1.1417493195612296,
        1.0328415859749787,  1.172477641714265,   -1.6724259464188527,
        6.649512957341614,   0.9251417217892692,  4.344754619764865,
        2.852325028107134,   0.22012802246620866, 1.988705729024391};
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.k = 3;
    param.ldxinv_transform = 3;
    param.ldxinv = 3;

    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetRowMajorData(std::vector<PCAParamType<T>> &params) {
    // Test with a tall thin data matrix
    PCAParamType<T> param;
    param.test_name = "Tall thin data matrix in row major order";
    param.n = 8;
    param.p = 5;
    std::vector<double> A{1.0,  3.0, 0.0, -7.0, 1.0,  2.0, 2.0,  5.5, 1.0, 2.0,
                          3.0,  0.2, 0.1, 0.8,  6.0,  4.0, 1.0,  0.9, 3.1, 0.0,
                          -7.8, 9.8, 0.7, 4.0,  4.1,  1.1, 3.0,  2.1, 6.2, 0.6,
                          2.0,  2.0, 5.5, 1.0,  -2.0, 3.0, -0.2, 0.1, 0.8, 6.0};
    datest_blas::imatcopy('T', param.n, param.p, 1.0, A.data(), param.n, param.p);
    param.A = convert_vector<double, T>(A);
    param.lda = 5;
    param.components_required = 3;
    param.method = "correlation";
    param.order = "row-major";
    param.svd_solver = "gesvdx";
    std::vector<double> expected_scores{
        0.6267979871027662,   -0.5840661299986506, -0.029499040897586454,
        1.170456209881111,    2.7265278206031027,  -1.4135093389270788,
        -0.9926217969782473,  -1.504085710785417,  -0.4195179292574332,
        0.0435634967928295,   0.31139177600800805, 1.7483581905660086,
        -0.9310388558598546,  1.3826222180401109,  -0.9145007073597778,
        -1.2208781889298912,  1.396660839144611,   -0.28269291608599634,
        -1.3489020461078889,  0.8035118542854264,  -0.5671754542756104,
        -0.24748367799132698, -0.8877960299693047, 1.133877431000091};
    datest_blas::imatcopy('T', param.n, param.components_required, 1.0,
                          expected_scores.data(), param.n, param.components_required);

    param.expected_scores = convert_vector<double, T>(expected_scores);
    std::vector<double> expected_u{
        0.16301441689926877,  -0.15190093390764553, -0.007671959786015266,
        0.30440626882328325,  0.7091014202034718,   -0.36761828437252125,
        -0.2581560036334061,  -0.3911749242264357,  -0.1444301519035428,
        0.014997886908849872, 0.10720486151797456,  0.601918586631605,
        -0.3205347709881148,  0.4760042969562981,   -0.3148410755975569,
        -0.42031963352551305, 0.5296868400048295,   -0.10721193951786444,
        -0.5115742077486266,  0.3047337213689863,   -0.2151025973403451,
        -0.09385875487730042, -0.3366986891185997,  0.43002562722892085};
    datest_blas::imatcopy('T', param.n, param.components_required, 1.0, expected_u.data(),
                          param.n, param.components_required);
    param.expected_u = convert_vector<double, T>(expected_u);
    std::vector<double> expected_components{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    datest_blas::imatcopy('T', param.components_required, param.p, 1.0,
                          expected_components.data(), param.components_required, param.p);
    param.expected_components = convert_vector<double, T>(expected_components);
    std::vector<double> expected_vt{
        -0.36868737869699797, -0.6316232406874217, -0.06683479410503197,
        -0.43113246622806944, -0.4801687992018557, -0.28425022864703875,
        -0.5779618011659566,  0.3844037107350251,  0.08444048886635638,
        0.5784712910192207,   -0.4144512975032958, -0.009353960170916515,
        -0.09759884842147158, -0.2257297394397662, 0.9526369849424157};
    datest_blas::imatcopy('T', param.components_required, param.p, 1.0,
                          expected_vt.data(), param.components_required, param.p);
    param.expected_vt = convert_vector<double, T>(expected_vt);
    std::vector<double> expected_variance{2.1120544797846836, 1.205278130973026,
                                          0.9932201143631529};
    param.expected_variance = convert_vector<double, T>(expected_variance);
    std::vector<double> expected_sigma{3.8450463402269657, 2.9046423044518215,
                                       2.6367671115481683};
    param.expected_sigma = convert_vector<double, T>(expected_sigma);
    param.expected_total_variance = (T)5.0;
    param.expected_n_components = 3;
    std::vector<double> expected_rinfo{8.0, 5.0, 3.0};
    param.expected_rinfo = convert_vector<double, T>(expected_rinfo);
    std::vector<double> X{0.1, 1.2, 3.1, 0.6, 5.1, -0.4, 0.1, -0.9, 12.3, 1.1};
    param.m = 2;
    datest_blas::imatcopy('T', param.m, param.p, 1.0, X.data(), param.m, param.p);
    param.X = convert_vector<double, T>(X);
    param.ldx = 5;

    param.ldx_transform = 3;
    std::vector<double> expected_X_transform{
        -1.7253499234437553, -0.6034681631460469, -0.046898565475484856,
        1.0339127620777953,  3.5129486393005163,  -0.035833080502485085};
    datest_blas::imatcopy('T', param.m, param.components_required, 1.0,
                          expected_X_transform.data(), param.m,
                          param.components_required);

    param.expected_X_transform = convert_vector<double, T>(expected_X_transform);
    std::vector<double> Xinv{1.1, 1.5, 4.1, 3.6, 5.2, -1.4};
    param.k = 2;
    datest_blas::imatcopy('T', param.k, param.components_required, 1.0, Xinv.data(),
                          param.k, param.components_required);

    param.ldxinv_transform = 5;
    param.ldxinv = 3;
    param.Xinv = convert_vector<double, T>(Xinv);
    std::vector<double> expected_Xinv_transform{
        -11.12944237741829, -8.929412948858758, -5.998571520336921, -1.965471274648701,
        8.213628377799804,  3.4138252479100837, 0.64114064861087,   1.5397586079913463,
        12.838600918423584, -4.694421617930349};
    datest_blas::imatcopy('T', param.k, param.p, 1.0, expected_Xinv_transform.data(),
                          param.k, param.p);
    param.expected_Xinv_transform = convert_vector<double, T>(expected_Xinv_transform);
    param.epsilon = 1000 * std::numeric_limits<T>::epsilon();

    param.expected_status = da_status_success;

    params.push_back(param);
}

template <typename T> void GetPCAData(std::vector<PCAParamType<T>> &params) {

    Get1by1Data(params);
    Get1by5Data(params);
    Get5by1Data(params);
    GetDiagonalData(params);
    GetIdentityData(params);
    GetZeroData(params);
    GetSquareData1(params);
    GetSquareData2(params);
    GetSquareData3(params);
    GetTallThinData1(params);
    GetTallThinData2(params);
    GetTallThinData3(params);
    GetTallThinData4(params);
    GetRowMajorData(params);
    GetShortFatData(params);
    GetSubarrayData1(params);
    GetSubarrayData2(params);
    GetSubarrayData3(params);
    GetSubarrayData4(params);
    GetBiasedData(params);
    GetBiasedData2(params);
}
