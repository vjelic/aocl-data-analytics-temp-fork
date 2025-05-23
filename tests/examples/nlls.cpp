/* Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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

/* Fit the convolution model y_i = [Al * lognormal(a, b)]_i + [Ag * normal(mu, sigma)]_i
 * given the density observations at the measured diameter sizes.
 */

#include "aoclda.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

struct udata_t {
    const da_int *diameter;
    const double *density;
};

const double pi = 3.14159265358979323846;

// Empirical data
const da_int diameter[64]{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                          17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
                          33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
                          49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64};
const double density[64]{
    0.0722713864, 0.0575221239, 0.0604719764, 0.0405604720, 0.0317109145, 0.0309734513,
    0.0258112094, 0.0228613569, 0.0213864307, 0.0213864307, 0.0147492625, 0.0213864307,
    0.0243362832, 0.0169616519, 0.0095870206, 0.0147492625, 0.0140117994, 0.0132743363,
    0.0147492625, 0.0140117994, 0.0140117994, 0.0132743363, 0.0117994100, 0.0132743363,
    0.0110619469, 0.0103244838, 0.0117994100, 0.0117994100, 0.0147492625, 0.0110619469,
    0.0132743363, 0.0206489676, 0.0169616519, 0.0169616519, 0.0280235988, 0.0221238938,
    0.0235988201, 0.0221238938, 0.0206489676, 0.0228613569, 0.0184365782, 0.0176991150,
    0.0132743363, 0.0132743363, 0.0088495575, 0.0095870206, 0.0073746313, 0.0110619469,
    0.0036873156, 0.0051622419, 0.0058997050, 0.0014749263, 0.0022123894, 0.0029498525,
    0.0014749263, 0.0007374631, 0.0014749263, 0.0014749263, 0.0007374631, 0.0000000000,
    0.0000000000, 0.0000000000, 0.0000000000, 0.0000000000};

const struct udata_t udata = {diameter, density};

// Scaled log-normal density distribution Al amplitude * log-normal(a, b)
double lognormal(double d, double a, double b, double Al) {
    return Al / (d * b * sqrt(2.0 * pi)) *
           std::exp(-(pow(std::log(d) - a, 2.0)) / (2.0 * pow(b, 2.0)));
}

// Scaled normal density distribution Ag amplitude * normal(mu, sigma)
double gaussian(double d, double mu, double sigma, double Ag) {
    return Ag * exp(-0.5 * pow((d - mu) / sigma, 2)) / (sigma * sqrt(2.0 * pi));
};

// Residuals for the convolution model
da_int eval_r([[maybe_unused]] da_int n_coef, da_int n_res, void *udata, double const *x,
              double *r) {
    double const a = x[0];
    double const b = x[1];
    double const Al = x[2];
    double const mu = x[3];
    double const sigma = x[4];
    double const Ag = x[5];
    da_int const *d = ((struct udata_t *)udata)->diameter;
    double const *y = ((struct udata_t *)udata)->density;
    for (da_int i = 0; i < n_res; ++i)
        r[i] = lognormal(d[i], a, b, Al) + gaussian(d[i], mu, sigma, Ag) - y[i];
    return 0;
}

// Jacobian matrix for the convolution model
da_int eval_J(da_int n_coef, da_int n_res, void *udata, double const *x, double *J) {
    double const a = x[0];
    double const b = x[1];
    double const Al = x[2];
    double const mu = x[3];
    double const sigma = x[4];
    double const Ag = x[5];
    da_int const *d = ((struct udata_t *)udata)->diameter;
    for (da_int i = 0; i < n_res; ++i) {
        double l = lognormal(d[i], a, b, Al);
        J[i * n_coef + 0] = (log(d[i]) - a) / pow(b, 2.0) * l;
        J[i * n_coef + 1] = (pow(log(d[i]) - a, 2.0) - pow(b, 2)) / pow(b, 3) * l;
        J[i * n_coef + 2] = lognormal(d[i], a, b, 1.0);
        double g = gaussian(d[i], mu, sigma, Ag);
        J[i * n_coef + 3] = (d[i] - mu) / pow(sigma, 2.0) * g;
        J[i * n_coef + 4] = (pow(d[i] - mu, 2.0) - pow(sigma, 2.0)) / pow(sigma, 3.0) * g;
        J[i * n_coef + 5] = gaussian(d[i], mu, sigma, 1.0);
    }
    return 0;
}
int main(void) {
    std::cout << " ----------------------------------------" << std::endl;
    std::cout << "     Nonlinear Least-Squares example" << std::endl;
    std::cout << " ----------------------------------------" << std::endl;

    const da_int n_coef = 6; /* vector (a, b, Al, mu, sigma, Ag) */
    const da_int n_res = 64;
    std::vector<double> coef({1.65, 0.9, 1.0, 30.0, 1.5, 0.25});
    const double coef_exp[n_coef]{1.99, 1.37, 0.68, 36.64, 7.08, 0.34};

    const double tol{1.0e-2};
    std::vector<double> lower_bounds(n_res, 0.0);
    std::vector<double> weights(n_res, 1.0);
    for (da_int j = 55; j <= 63; ++j)
        weights[j] = 5.0;
    double wsum = 1.0 * (n_res - (63 - 55)) + 5.0 * (63 - 55);
    for (da_int j = 55; j <= 63; ++j)
        weights[j] = 5.0;
    // Normalize weights
    for (da_int j = 0; j < 64; ++j)
        weights[j] /= wsum;

    // Initialize handle for nonlinear regression
    da_handle handle = nullptr;

    bool pass = true;
    pass &= da_handle_init_d(&handle, da_handle_nlls) == da_status_success;
    pass &= da_nlls_define_residuals_d(handle, n_coef, n_res, eval_r, eval_J, nullptr,
                                       nullptr) == da_status_success;
    pass &= da_nlls_define_bounds_d(handle, n_coef, lower_bounds.data(), nullptr) ==
            da_status_success;
    pass &= da_nlls_define_weights_d(handle, n_res, weights.data()) == da_status_success;
    if (!pass) {
        std::cout << "Something unexpected happened in the model definition\n";
        da_handle_destroy(&handle);
        return 1;
    }
    pass &= da_options_set_int(handle, "print level", (da_int)2) == da_status_success;
    pass &=
        da_options_set_string(handle, "storage order", "row-major") == da_status_success;
    pass &= da_options_set_real_d(handle, "finite differences step", 1e-7) ==
            da_status_success;
    pass &= pass &=
        da_options_set_real_d(handle, "derivative test tol", 1e-3) == da_status_success;
    pass &=
        da_options_set_string(handle, "check derivatives", "yes") == da_status_success;
    if (!pass) {
        std::cout << "Something unexpected happened while setting options\n";
        da_handle_destroy(&handle);
        return 2;
    }

    std::cout << "\n ** Computing regression with exact first derivatives **\n"
              << std::endl;
    // Compute regression
    da_status status;
    status = da_nlls_fit_d(handle, n_coef, coef.data(), (void *)&udata);
    bool ok{false};
    const auto default_precision{std::cout.precision()};
    if (status == da_status_success) {
        std::cout << "Regression computed successfully!" << std::endl;
        std::cout << "Coefficients: Idx           x            x*\n";
        ok = true;
        for (auto i = 0; i < n_coef; ++i) {
            double gap = std::abs(coef[i] - coef_exp[i]);
            bool oki = gap <= tol;
            ok &= oki;
            std::cout << std::setprecision(6);
            std::cout << "                " << i << std::setw(12) << coef[i] << " "
                      << std::setw(1) << " " << std::setw(12) << coef_exp[i]
                      << std::setw(8) << (oki ? "PASS (" : "FAIL (") << std::setw(8)
                      << std::setprecision(3) << gap << std::setw(1) << ")\n";
        }
        std::cout << std::setprecision(3);
        std::cout << std::setprecision(default_precision);
    } else {
        std::cout << "Something wrong happened during the fit. Terminating. Message:"
                  << std::endl;
        char *mesg{nullptr};
        da_handle_get_error_message(handle, &mesg);
        std::cout << mesg << std::endl;
        free(mesg);
        da_handle_destroy(&handle);
        return 3;
    }

    // Get info out of handle
    std::vector<double> info(1);
    da_int size = info.size();
    status = da_handle_get_result_d(handle, da_result::da_rinfo, &size, info.data());
    if (status == da_status_operation_failed) {
        info.resize(size);
    }
    status = da_handle_get_result_d(handle, da_result::da_rinfo, &size, info.data());
    if (status == da_status_success) {
        std::cout << "Fit error                : " << info[0] << std::endl;
        std::cout << "Norm of residual gradient: " << info[1] << std::endl;
    }

    // Now we solve again using finite differences
    std::cout << "\n\n ** Computing regression with finite differences **\n" << std::endl;
    coef.assign({1.65, 0.9, 1.0, 30.0, 1.5, 0.25});
    pass &= da_nlls_define_residuals_d(handle, n_coef, n_res, eval_r, nullptr, nullptr,
                                       nullptr) == da_status_success;
    if (!pass) {
        std::cout << "Something unexpected happened while defining residuals\n";
        da_handle_destroy(&handle);
        return 5;
    }
    status = da_nlls_fit_d(handle, n_coef, coef.data(), (void *)&udata);
    if (status == da_status_success) {
        const auto default_precision{std::cout.precision()};
        std::cout << "Regression computed successfully!" << std::endl;
        std::cout << "Coefficients: Idx           x            x*\n";
        for (auto i = 0; i < n_coef; ++i) {
            double gap = std::abs(coef[i] - coef_exp[i]);
            bool oki = gap <= tol;
            ok &= oki;
            std::cout << std::setprecision(6);
            std::cout << "                " << i << std::setw(12) << coef[i] << " "
                      << std::setw(1) << " " << std::setw(12) << coef_exp[i]
                      << std::setw(8) << (oki ? "PASS (" : "FAIL (") << std::setw(8)
                      << std::setprecision(3) << gap << std::setw(1) << ")\n";
        }
        std::cout << std::setprecision(3);
        std::cout << std::setprecision(default_precision);
    } else {
        std::cout << "Something wrong happened during the fit. Terminating. Message:"
                  << std::endl;
        char *mesg{nullptr};
        da_handle_get_error_message(handle, &mesg);
        std::cout << mesg << std::endl;
        free(mesg);
        da_handle_destroy(&handle);
        return 6;
    }

    // Get info out of handle
    status = da_handle_get_result_d(handle, da_result::da_rinfo, &size, info.data());
    if (status == da_status_operation_failed) {
        info.resize(size);
    }
    status = da_handle_get_result_d(handle, da_result::da_rinfo, &size, info.data());
    if (status == da_status_success) {
        std::cout << "Fit error                : " << info[0] << std::endl;
        std::cout << "Norm of residual gradient: " << info[1] << std::endl;
    }

    da_handle_destroy(&handle);

    std::cout << "Regressions were computed " << (ok ? "SUCCESSFULLY" : "with errors")
              << std::endl;

    return ok ? 0 : 4;
}
