#ifndef RADIUSMATH_H
#define RADIUSMATH_H

namespace latlon   // Put it beside the other geometry helpers
{
    /**
     * Circum‑circle radius of a triangle with side lengths a, b, c.
     * Returns 10 000 if any side ≤ 0 or if the triangle is degenerate.
     *
     * Formula source: mathopenref.com/trianglecircumcircle.html
     */
    double circumCircleRadius(double a, double b, double c);
} // namespace latlon

#endif // RADIUSMATH_H
