#include "latlonmath.h"
#include <cmath>          // std::sin, std::cos, std::acos, std::fabs, M_PI
#include <algorithm>      // std::clamp

namespace latlon
{
    // ───────────────────────────────────────── Units ──────────────────────────
    Units::Units(const std::string& u)
    {
        if (u == "m")  kind_ = Kind::M;
        else if (u == "km") kind_ = Kind::KM;
        else if (u == "mi") kind_ = Kind::MI;
        else
            throw std::invalid_argument("Units must be \"m\", \"km\", or \"mi\"");
    }

    double Units::convert(double meters) const
    {
        switch (kind_)
        {
            case Kind::M:  return meters;
            case Kind::KM: return meters / 1'000.0;
            case Kind::MI: return meters / 1'609.344; // exact statute
        }
        // Should never reach here
        return meters;
    }

    // ───────────────────────────── Distance helpers ───────────────────────────
    namespace
    {
        constexpr double kDegToRad = M_PI / 180.0;
    }

    double distanceOnUnitSphere(double lat1Deg, double lon1Deg,
                                 double lat2Deg, double lon2Deg)
    {
        // Identical points ⇒ distance 0
        if (std::fabs(lat1Deg - lat2Deg) < 1e-12 &&
            std::fabs(lon1Deg - lon2Deg) < 1e-12)
            return 0.0;

        // Convert to spherical coordinates
        const double phi1 = (90.0 - lat1Deg) * kDegToRad;
        const double phi2 = (90.0 - lat2Deg) * kDegToRad;
        const double theta1 = lon1Deg * kDegToRad;
        const double theta2 = lon2Deg * kDegToRad;

        // cos(arc) on the unit sphere
        double cosArc =  std::sin(phi1) * std::sin(phi2) *
                         std::cos(theta1 - theta2)
                       + std::cos(phi1) * std::cos(phi2);

        // Numerical noise can push slightly outside [–1, 1]
        cosArc = std::clamp(cosArc, -1.0, 1.0);

        return std::acos(cosArc);           // arc length in **radians**
    }

    double distanceOnEarth(double lat1Deg, double lon1Deg,
                           double lat2Deg, double lon2Deg)
    {
        return distanceOnUnitSphere(lat1Deg, lon1Deg, lat2Deg, lon2Deg)
               * kRadiusEarthM;             // -> meters
    }
} // namespace latlon
