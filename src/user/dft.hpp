#pragma once

#include "engine/common/math.hpp"

/// Represents a DFT for a given signal
struct DFT
{
    /// Helper type over STL complex
    using Complex = std::complex<float>;
    /// Represents one DFT coefficient
    struct Coef
    {
        /// The coefficient index
        int32_t         i = 0;
        /// The coefficient itself
        Complex v = 0.0f;

        /// Default constructor
        Coef() = default;

        /// Initializes the coefficient with its rank
        explicit
        Coef(int32_t rank)
            : i{rank}
            , v{0.0f, 0.0f}
        {}

        /// Returns the arg of the coefficient
        [[nodiscard]]
        float getArg() const
        {
            return std::arg(v);
        }

        /// Returns the norm of the coefficient
        [[nodiscard]]
        float getNorm() const
        {
            return sqrt(v.real() * v.real() + v.imag() * v.imag());
        }
    };

    /// The signal associated with this DFT
    std::vector<Complex> const* signal = nullptr;

    /// The coefficient of the DFT
    std::vector<Coef> coefficients;
    /// The next coefficient that will be computed when calling @p addCoefficient
    int32_t next_coef = 0;

    /** Computes the coefficient of rank @p i and adds it in the coefficients list
     *
     * @param i The rank of the coefficient to compute
     */
    void computeCoefficient(int32_t i)
    {
        // Ensure the signal is set
        if (!signal) {
            std::cout << "No signal set" << std::endl;
            return;
        }

        Coef result{i};

        float const dx = Math::ConstantF32::TwoPi / to<float>(signal->size());
        uint32_t k = 0;
        for (auto const& sample : (*signal)) {
            float const t = to<float>(k) * dx;
            float const x = to<float>(i) * t;
            result.v += sample * Complex{cos(x), -sin(x)};
            ++k;
        }

        coefficients.push_back(result);
    }

    /** Adds the next coefficient in the list, alternating between negative and positive coefficient
     *  Coefficients sequence: 0, 1, -1, 2, -2, 3, -3, etc...
     */
    void addCoefficient()
    {
        computeCoefficient(next_coef);
        next_coef = -next_coef + (next_coef <= 0);
    }

    /** Computes the next pair of coefficients, should not be mixed with @p addCoefficient
     *  Coefficients sequence: [0], [1, -1], [2, -2], [3, -3], etc...
     */
    void addCoefficientPair()
    {
        if (!signal) {
            std::cout << "No signal set" << std::endl;
            return;
        }

        if (signal->empty()) {
            std::cout << "Empty signal" << std::endl;
            return;
        }

        if (coefficients.size() > signal->size()) {
            std::cout << "All coefficients have been computed" << std::endl;
            return;
        }

        computeCoefficient(next_coef);
        if (next_coef) {
            computeCoefficient(-next_coef);
        }
        ++next_coef;
    }

    /** Computes the inverse transform at time t
     *
     * @param t The time of the reconstructed sample
     * @return The inverse transform given the currently available coefficients
     */
    [[nodiscard]]
    Complex getReverse(float t) const
    {
        Complex res{0.0f};
        auto const div = to<float>(signal->size());
        for (auto const& c : coefficients) {
            float const x = to<float>(c.i) * t;
            res += c.v * Complex{cos(x), sin(x)};
        }
        return res / div;
    }

    /// Removes all coefficients from the DFT
    void clear()
    {
        next_coef = 0;
        coefficients.clear();
    }

    /// Initializes the signal pointer
    void setSignal(std::vector<Complex> const& sig)
    {
        signal = &sig;
    }
};
