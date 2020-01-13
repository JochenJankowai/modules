/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef IVW_SHANNONENTROPY2D_H
#define IVW_SHANNONENTROPY2D_H

#include <inviwo/integrallinefiltering/integrallinefilteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/histogram.h>

#include <inviwo/integrallinefiltering/utils/sparsehistogram.h>
#include <inviwo/integrallinefiltering/utils/directionalhistogram.h>

namespace inviwo {

namespace util {
enum PerformNormalize { Yes, No };

inline double shannonEntropyMax(size_t N) { return std::log2(static_cast<double>(N)); }

namespace detail {
template <typename T, typename = void>
struct is_pair : public std::false_type {};

template <typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : public std::true_type {};
}  // namespace detail

template <typename Histogram>
double shannonEntropy(const Histogram& histogram) {
    size_t c = std::accumulate(histogram.begin(), histogram.end(), 0, [](size_t a, auto bin) {
        if constexpr (detail::is_pair<decltype(bin)>::value) {
            return a + bin.second;
        } else {
            return a + bin;
        }
    });
    double ent = 0;
    for (auto bin : histogram) {
        double h;
        if constexpr (detail::is_pair<decltype(bin)>::value) {
            h = static_cast<double>(bin.second);
        } else {
            h = static_cast<double>(bin);
        }
        if (h != 0) {  // log(0) = 0 when calculating entropy
            h /= c;
            ent += h * std::log2(h);
        }
    }

    return -ent;
}

template <typename T>
double shannonEntropyScalars(const std::vector<T>& data, size_t numBins = 8) {
    auto minmax = std::minmax_element(data.begin(), data.end());
    T min = *minmax->first;
    T max = *minmax->second;

    SparseHistogram<size_t> histogram;

    for (const auto& v : data) {
        auto x = (v - min) / (max - min);
        auto i = static_cast<size_t>(x * (numBins - 1));
        if (i >= numBins) {
            i = numBins - 1;
        }
        histogram[i]++;
    }

    return shannonEntropy(histogram);
}

template <size_t Dims, typename T>
double shannonEntropyDirectional(const std::vector<glm::vec<Dims, T>>& values, const size_t subdivs,
                                 PerformNormalize normalize = PerformNormalize::Yes) {
    static_assert(std::is_floating_point_v<T>);
    static_assert(Dims == 2 || Dims == 3);
    auto histogram = [subdivs] {
        if constexpr (Dims == 2) {
            return DirectionalHistogram<2, T>::createFromCircle(subdivs);

        } else if constexpr (Dims == 3) {
            return DirectionalHistogram<3, T>::createFromIcosahedron(subdivs);
        }
    }();

    for (const auto& val : values) {
        histogram.inc(val);
    }
    if (normalize == PerformNormalize::Yes) {
        return shannonEntropy(histogram) / shannonEntropyMax(histogram.numberOfBins());
    } else {
        return shannonEntropy(histogram);
    }
}

template <size_t Dims, typename T>
double shannonEntropyEuclidean(const std::vector<glm::vec<Dims, T>>& values,
                               const glm::vec<Dims, T>& binSize) {
    static_assert(std::is_floating_point_v<T>);
    using bin_t = glm::vec<Dims, glm::i64>;
    SparseHistogram<bin_t> histogram;

    for (const auto& v : values) {
        const auto bin = static_cast<bin_t>(glm::ceil(v / binSize));
        histogram[bin]++;
    }

    return shannonEntropy(histogram);
}

template <size_t Dims, typename T>
double shannonEntropyEuclidean(const std::vector<glm::vec<Dims, T>>& values, const T& binSize) {
    return shannonEntropyEuclidean(values, glm::vec<Dims, T>{binSize});
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_SHANNONENTROPY2D_H
