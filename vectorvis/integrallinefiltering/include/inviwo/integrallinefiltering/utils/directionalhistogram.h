/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#pragma once

#include <inviwo/integrallinefiltering/integrallinefilteringmoduledefine.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/foreach.h>

#include <inviwo/integrallinefiltering/utils/sparsehistogram.h>

namespace inviwo {

namespace {
template <typename T>
T V(const T t) {
    const T s = sin(t / 2);
    return 4 * glm::pi<T>() * s * s;
}

template <typename T>
T Theta(const T v) {
    return 2 * std::asin(std::sqrt(v / (4 * glm::pi<T>())));
}

}  // namespace

template <unsigned Dims, typename T>
class DirectionalHistogram;

template <typename T>
class DirectionalHistogram<2, T> {
    static_assert(std::is_floating_point_v<T>);

public:
    using type = glm::vec<2, T>;

    DirectionalHistogram(const size_t segments = 20) : bins_(segments, 0) {}

    DirectionalHistogram(const DirectionalHistogram &) = default;
    DirectionalHistogram(DirectionalHistogram &&) = default;
    DirectionalHistogram &operator=(const DirectionalHistogram &) = default;
    DirectionalHistogram &operator=(DirectionalHistogram &&) = default;

    size_t numberOfBins() const { return bins_.size(); }

    size_t inc(const type &in_dir) {
        const auto dir = glm::normalize(in_dir);
        const auto a = atan2(dir.y, dir.x) / glm::two_pi<T>() + 0.5;
        const auto I = std::min(static_cast<size_t>(a * bins_.size()), bins_.size() - 1);
        bins_[I]++;
        return I;
    }

    auto begin() { return bins_.begin(); }
    auto end() { return bins_.end(); }
    auto begin() const { return bins_.begin(); }
    auto end() const { return bins_.end(); }

private:
    std::vector<size_t> bins_;
};

/*
 * Using Sphere partitioning defined in paper [1].
 *
 * [1] Leopardi, Paul. "A partition of the unit sphere into regions of equal area and small
 * diameter." Electronic Transactions on Numerical Analysis 25.12 (2006): 309-327.
 */
template <typename T>
class DirectionalHistogram<3, T> {
    static_assert(std::is_floating_point_v<T>);
    struct Segment {
        Segment(T endAngle, size_t patches)
            : endAngle_(endAngle), cosAngle_(cos(endAngle)), patches_(patches), startIndex(0) {}

        const T endAngle_;
        const T cosAngle_;
        const size_t patches_;
        size_t startIndex;

        size_t index(T alpha) {
            const auto A = std::min(static_cast<size_t>(alpha * patches_), patches_ - 1);
            return A + startIndex;
        }

        size_t index(T x, T y) {
            if (patches_ == 1) {
                return startIndex;
            }
            const T a = atan2(y, x) / glm::two_pi<T>() + T(0.5);
            return index(a);
        }

        bool operator<(const Segment &rhs) const { return this->endAngle_ < rhs.endAngle_; }
        bool operator<(const T cosAngle) const { return this->cosAngle_ > cosAngle; }
    };

public:
    using type = glm::vec<3, T>;

    DirectionalHistogram(const size_t segments = 20) : bins_(segments, 0) {
        if (segments == 0) {
            throw Exception("Zero segments not allowed", IVW_CONTEXT);
        } else if (segments == 1) {
            segments_.emplace_back(glm::pi<T>(), 1);
            return;
        } else if (segments == 2) {
            segments_.emplace_back(glm::half_pi<T>(), 1);
            segments_.emplace_back(glm::pi<T>(), 1);
            segments_.back().startIndex = 1;
            return;
        }

        const T regionArea = 4 * glm::pi<T>() / segments;
        const T colatPole = Theta(regionArea);

        const T idealCollarAngle = std::sqrt(regionArea);
        const T idealNumberOfCollars = (glm::pi<T>() - 2 * colatPole) / idealCollarAngle;

        const size_t numberOfCollars =
            std::max(size_t(1), static_cast<size_t>(idealNumberOfCollars + 0.5));

        const auto fittingCollarAngle = (glm::pi<T>() - 2 * colatPole) / numberOfCollars;

        auto collatOfCollar = [&](size_t i) { return colatPole + (i - 1) * fittingCollarAngle; };

        std::vector<T> idealNumberOfRegions;
        idealNumberOfRegions.push_back(0);
        for (size_t i = 1; i <= numberOfCollars; i++) {
            auto a = V(collatOfCollar(i + 1));
            auto b = V(collatOfCollar(i));

            idealNumberOfRegions.push_back((a - b) / regionArea);
        }

        std::vector<T> a(1, 0);
        std::vector<size_t> regionsInCollar(1, 0);
        for (size_t i = 1; i <= numberOfCollars; i++) {
            T tmp = idealNumberOfRegions[i] + a[i - 1];
            regionsInCollar.push_back(static_cast<size_t>(tmp + 0.5));
            a.push_back(T(0.0));
            for (size_t j = 1; j <= i; j++) {
                a.back() += idealNumberOfRegions[j] - regionsInCollar[j];
            }
        }

        std::vector<T> collatitudes;

        collatitudes.push_back(0);
        std::vector<T> debug = collatitudes;

        for (size_t i = 1; i <= numberOfCollars + 1; i++) {

            T totM = 0;
            for (size_t j = 1; j < i; j++) {
                totM += regionsInCollar[j];
            }
            const T v = (1 + totM) * regionArea;

            debug.push_back(v);
            collatitudes.push_back(Theta(v));
        }

        collatitudes.push_back(glm::pi<T>());
        debug.push_back(glm::pi<T>());

        segments_.emplace_back(collatitudes[1], 1);
        for (size_t i = 1; i < collatitudes.size() - 2; i++) {
            segments_.emplace_back(collatitudes[i + 1], regionsInCollar[i]);
        }
        segments_.emplace_back(glm::pi<T>(), 1);

        size_t count = 0;
        for (auto &segment : segments_) {
            segment.startIndex = count;
            count += segment.patches_;
        }
        IVW_ASSERT(count == segments, "");
    }

    DirectionalHistogram(const DirectionalHistogram &) = default;
    DirectionalHistogram(DirectionalHistogram &&) = default;
    DirectionalHistogram &operator=(const DirectionalHistogram &) = default;
    DirectionalHistogram &operator=(DirectionalHistogram &&) = default;

    size_t numberOfBins() const { return bins_.size(); }

    size_t inc(const type &in_dir) {
        const auto dir = glm::normalize(in_dir);
        const auto it = std::lower_bound(segments_.begin(), segments_.end(), dir.z);
        auto I = it->index(dir.x, dir.y);
        IVW_ASSERT(I < bins_.size(), "maxindex should not be able to largers than number of bins");
        bins_[I]++;
        return I;
    }

    auto begin() { return bins_.begin(); }
    auto end() { return bins_.end(); }
    auto begin() const { return bins_.begin(); }
    auto end() const { return bins_.end(); }

private:
    std::vector<size_t> bins_;
    std::vector<Segment> segments_;
};

}  // namespace inviwo
