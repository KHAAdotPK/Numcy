/*
 * Numcy/axis.hh
 * Q@hackers.pk
 */

#ifndef NUMCY_AXIS_HH
#define NUMCY_AXIS_HH

namespace numcy {

    enum class Axis : int {
        Rows        = 0,  // Vertical (NumPy Axis 0)
        Columns     = 1,  // Horizontal (NumPy Axis 1)
        Slices      = 2,  // Third Dimension (NumPy Axis 2) Depth/Pages
        Last        = -1, // Innermost/last dimension (tail->columns)
        SecondLast  = -2  // Second-to-last dimension (tail->rows)
    };
}

#endif