/*
    numcy/header.hh
    Q@Khaa.pk
 */

#include <complex>
#include <cstdarg>
#include <functional>
#include <iostream>
#include <random>

/*#include <type_traits>*/ // For std::is_same and static_assert

#include "../ala_exception/ala_exception.hh"
#include "../allocator/allocator.hh"
#include "../String/src/string_character_traits.hh"

#include "macros.hh"
//#include "DimensionsOfArray.hh"
//#include "Dimensions.hh"

#ifndef KHAA_PK_NUMCY_HEADER_HH
#define KHAA_PK_NUMCY_HEADER_HH

#define NUMCY_DEFAULT_REFERENCE_COUNT 1
#define NUMCY_DEFAULT_REFERENCE_COUNT_BEFORE_INCREMENT 0

#define NUMCY_DEFAULT_MEAN                0.0
#define NUMCY_DEFAULT_STANDARD_DEVIATION  1.0
#define NUMCY_DEFAULT_RANDOM_NUMBER_GENERATOR_SEED 10

/* 
    DIMENSIONSOFARRAY::ptr array at index DIMENSIONSOFARRAY::size() - 2 stores the number of rows of inner most two dimension array.
    DIMENSIONSOFARRAY::ptr array at index (DIMENSIONSOFARRAY::size() - 2) + 1  stores the number of columns of inner most two dimension array.
    These two indices comes from the last link of DIMENSIONS linkedlist.

    NUMBER_OF_INDEXES_OF_ROWS_COLUMNS_OF_LAST_LINK represent the above mentioned two indices.
 */
#define NUMBER_OF_INDEXES_OF_ROWS_COLUMNS_OF_LAST_LINK 2

/*
    Macro to get the number of rows in the innermost two-dimensional array from the DIMENSIONSOFARRAY::ptr array.
    @param m The DIMENSIONSOFARRAY::ptr array.
    @return The number of rows in the innermost two-dimensional array.
 */
#define NUMBER_OF_ROWS_IN_INNER_MOST_ARRAY(m) m.getShape().getDimensionsOfArray()[m.getShape().getDimensionsOfArray().size() - NUMBER_OF_INDEXES_OF_ROWS_COLUMNS_OF_LAST_LINK]
/*
    Macro to get the number of columns in the innermost two-dimensional arrays from the DIMENSIONSOFARRAY::ptr array.
    @param m The DIMENSIONSOFARRAY::ptr array.
    @return The number of columns in the innermost two-dimensional array.
*/
#define NUMBER_OF_COLUMNS_IN_INNER_MOST_ARRAYS(m) m.getShape().getDimensionsOfArray()[(m.getShape().getDimensionsOfArray().size() - NUMBER_OF_INDEXES_OF_ROWS_COLUMNS_OF_LAST_LINK) + 1]

/* ALL THREE ARE P FOR DELETION */
#define IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray 2
#define IMPLIED_NUMBER_OF_ROWS_IN_INNER_MOST_ARRAY(p, n)     p[(n - IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray)]
#define IMPLIED_NUMBER_OF_COLUMNS_IN_INNER_MOST_ARRAYS(p, n) p[(n - IMPLIED_ROWS_COLUMNS_OF_LAST_LINK_RETURNED_BY_METHOD_getDimensionsOfArray + 1)]

/* 
    Mental Model:
    - axis=0 -> Going down the rows
    - axis=1 -> Going across the columns
    - axis=-1 -> The last axis (columns in 2D array)

    The axis along which a particular operation should be performed. 
    The axis=-1, is the last axis. 
    The axis=1, second axis(in two dimentional array it means the columns)
    The axis=0, first axis(in two dimentional array it means rows)
 */
enum AXIS 
{
    /*
        In Python, using "axis=-1" implies an operation along the last axis. In the context of our DIMENSIONS array, this corresponds to the number of columns in the last dimension.
        In Numcy, AXIS_COLUMNS is equivalent to Python's "axis=-1". 
        This means that only the last axis of the two arrays can have different sizes.

        For instance, in the scenario "a[10][256] concatenate b[10][10]", the result will be a new array with a shape of [10][266].
        However, attempting "a[5][256] concatenate b[10][10]" would result in an error since the number of rows is different.
    */  
    AXIS_COLUMN = 1,// Number of rows in a must match number of columns in b. Stacking horizontally
    AXIS_LAST = -1, // In two dimentional aray, the number of rows in a must match number of columns in b 

    /*
        Specifies that the summation should be done across rows.
        The resulting array will have the same number of columns as the input vectors.
     */
    AXIS_ROWS = 0, // Horizontal stack of rows, summation along the rows. 
    AXIS_NONE =  2, // [1][a_rs*a_column + b_rs*b_columns] when rs is total number of rows. Results in one horizontal line
};

/*
    Whether to store multi-dimensional data in row-major (C-style) or column-major (Fortran-style)
    order in memory
    C-stype : In row-major order, elements of the array are stored in memory row-by-row.
              For a 2-dimensional array, this means that the elements of each row are contiguous in memory.
              [1,  2,  3,  4,  5,  6,  7,  8,  9]

    Fortran-style: In column-major order, elements of the array are stored in memory column-by-column.
                   For a 2-dimensional array, this means that the elements of each column are contiguous in memory.
                   [1,  4,  7,  2,  5,  8,  3,  6,  9]
 */
/*
    This is just my way of understanding the difference...
    Array[2][2][3]

    C-Style
    1 2 3 
    4 5 6

    7 8 9
    10 11 12

    Fortran-Style
    1 4
    2 5
    3 6

    7 10
    8 11
    9 12
 */
enum REPLIKA_ROW_MAJOR_OR_COLUMN_MAJOR 
{   
    REPLIKA_ROW_MAJOR = 0, // Row-major (C-style) 
    REPLIKA_COLUMN_MAJOR = 1 // Column-major (Fortran-style)
};

/*
enum NUMCY_DTYPE 
{
    NUMCY_DTYPE_INT = 0, // int
    NUMCY_DTYPE_FLOAT = 1 // float
};
 */

/*
    Used with Dimensions::operator []
    This operator overloading would return the number of rows for NUMCY_DIMENSIONS_SHAPE_ROWS and number of columns for the NUMCY_DIMENSIONS_SHAPE_COLUMNS
 */
#define NUMCY_DIMENSIONS_SHAPE_ROWS     0
#define NUMCY_DIMENSIONS_SHAPE_COLUMNS  1

#include "DimensionsOfArray.hh"
#include "Dimensions.hh"
#include "Collective.hh"
#include "numcy.hh"

#endif
