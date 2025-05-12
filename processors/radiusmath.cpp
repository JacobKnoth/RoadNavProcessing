#include "radiusmath.h"
#include <cmath>        // std::sqrt, std::fabs
#include <algorithm>    // std::max

namespace latlon
{
    namespace
    {
        constexpr double kInvalidRadius = 10'000.0;
    }

    double circumCircleRadius(double a, double b, double c)
    {
        // Basic validity check
        if (a <= 0.0 || b <= 0.0 || c <= 0.0)
            return kInvalidRadius;

        // Term under the square‑root in the formula
        const double term = (a + b + c) * (b + c - a)
                          * (c + a - b) * (a + b - c);

        // Degenerate or impossible triangle?
        if (term <= 0.0)
            return kInvalidRadius;

        const double divider = std::sqrt(std::fabs(term));

        // Should not happen with the above guard, but stay safe
        if (divider == 0.0)
            return kInvalidRadius;

        return (a * b * c) / divider;
    }
} // namespace latlon
