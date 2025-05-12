#ifndef LATLONMATH_H
#define LATLONMATH_H

#include <stdexcept>   // std::invalid_argument
#include <string>      // std::string

namespace latlon
{
    // Earth‑radius in meters
    constexpr double kRadiusEarthM = 6'378'000.0;

    /**
     * Simple units helper that lets you ask for distances back in
     * metres (“m”), kilometres (“km”) or statute miles (“mi”).
     *
     * Usage:
     *     Units u{"km"};
     *     double km = u.convert(metres);
     */
    class Units
    {
    public:
        explicit Units(const std::string& u = "m");    // default “m”
        double convert(double valueInMetres) const;    // -> chosen units

    private:
        enum class Kind { M, KM, MI } kind_;
    };

    /**
     * Great‑circle distance between two latitude/longitude pairs,
     * returned in **metres**.
     *
     * Latitude ∈ [–90, +90], longitude ∈ [–180, +180] (degrees).
     */
    double distanceOnEarth(double lat1Deg,
                           double lon1Deg,
                           double lat2Deg,
                           double lon2Deg);

    /**
     * Great‑circle distance *on the unit sphere* in **radians**.
     * (Same routine as the original Python helper.)
     */
    double distanceOnUnitSphere(double lat1Deg,
                                double lon1Deg,
                                double lat2Deg,
                                double lon2Deg);
} // namespace latlon

#endif // LATLONMATH_H
