```C++
    Column_And_Row_Major_Arrays.md  
    Written by, Sohail Qayum Malik  
```

> **Note to Readers:** This document is a work in progress. You may encounter occasional typos and formatting inconsistencies as the content is being actively developed and refined. The focus at this stage is on technical accuracy and conceptual clarity. A thorough editorial review will be conducted in future revisions. Thank you for your understanding.

`"Readers should be aware that this article represents an ongoing project. The information and code contained herein are preliminary and will be expanded upon in future revisions."`

**Column Major.**

A column-major matrix is also called Fortran-order or F-order. <i>In column-major layout, the first index varies fastest in memory</i>, so elements are stored down columns first, then across rows, with higher axes spanning outer dimensions.

An array with dimensions `(axis0=2, axis1=3, axis2=4)` contains 24 total elements. `axis0` is the number of rows, `axis1` is the number of columns, and `axis2` is the size of the third dimension (depth or slices). If there are more than 3 dimensions, each additional axis corresponds to another outermost dimension, adding another level of depth or higher-order slices. 

<i>Array Stride.</i> <br>
Let's assume that our array holds values of type `double`; on x64, a `double` is 8 bytes long.

In a contiguous array, the stride for an axis is the byte distance between successive elements along that axis. In column-major order, the first axis (`axis0`) varies fastest, so its stride is the element size. The next axis's stride is the size of `axis0` times the element size, and each further axis multiplies by the size of all previous axes.

For example, a 3D array with shape `(axis0=2, axis1=3, axis2=4)` and `double` elements has:

- stride along `axis0` = `1 * 8 = 8` bytes
- stride along `axis1` = `2 * 8 = 16` bytes
- stride along `axis2` = `2 * 3 * 8 = 48` bytes

This means consecutive/adjacent rows are 8 bytes apart, consecutive/adjacent columns are 16 bytes apart, and consecutive/adjacent depth slices are 48 bytes apart in memory.

Those stride values are what the offset formula uses. In column-major order, the flat offset is built as:

`offset = i * stride_axis0 + j * stride_axis1 + k * stride_axis2`

Substituting the stride values gives:

`offset = i * 8 + j * 16 + k * 48`

Factoring out the element size yields the familiar formula:

`offset = (i + j * axis0 + k * axis0 * axis1) * element_size`

Here, `i` is the row index along `axis0`, `j` is the column index along `axis1`, and `k` is the depth/slice index along `axis2`. The term `i + j * axis0` selects the element inside a slice, and `k * axis0 * axis1` jumps whole slices in the linear buffer.

For our example data, the flat buffer contains the values in this exact order:

`1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24`

So element `(0, 0, 0)` is `1` at byte offset `0`, element `(1, 0, 0)` is `2` at offset `8`, and element `(0, 1, 0)` is `3` at offset `16`. Because the buffer is linear, strides are required to interpret how multi-dimensional coordinates walk through that one-dimensional memory region.

These strides are what define the physical layout of the array in memory and determine how efficiently operations can traverse it.

To draw a (2, 3, 4) column-major array on paper, visualize four separate 2D grids representing each depth layer (Axis 2). Notice how numbers count straight down each column before moving right.

```
Slice 0 (Depth index = 0)

        Col 0    Col 1    Col 2 
Row 0:    1        3        5
Row 1:    2        4        6    
```
```
Slice 1 (Depth index = 1)

        Col 0    Col 1    Col 2
Row 0:    7        9       11
Row 1:    8       10       12

```
```
Slice 2 (Depth index = 2)

        Col 0    Col 1    Col 2
Row 0:   13       15       17
Row 1:   14       16       18

```
```
Slice 3 (Depth index = 3)

        Col 0    Col 1    Col 2
Row 0:   19       21       23
Row 1:   20       22       24
```

**Row Major**

A row-major matrix is also called C-order. <i>In row-major layout, the last index varies fastest in memory</i>, so elements are stored across rows first, then down columns, with outer axes spanning larger jumps in the linear buffer.

For the same array shape `(axis0=2, axis1=3, axis2=4)` and the same logical values, row-major storage changes the memory order. Here, each 2D slice is stored row by row rather than column by column.

If we keep the same logical values shown above, the row-major flat buffer contains:

`1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24`

Because the buffer is linear, the flat offset for element `(i, j, k)` becomes:

`offset = (i * axis1 * axis2 + j * axis2 + k) * element_size`

With `double` elements, the strides are:

- stride along `axis2` = `1 * 8 = 8` bytes
- stride along `axis1` = `4 * 8 = 32` bytes
- stride along `axis0` = `3 * 4 * 8 = 96` bytes

This means consecutive/adjacent depth elements/slices are 96 bytes apart, consecutive/adjacent columns are 8 bytes apart, and consecutive/adjacent rows are 32 bytes apart in memory.

The row-major offset formula uses those strides directly:

`offset = i * stride_axis0 + j * stride_axis1 + k * stride_axis2`

Substituting the row-major strides gives:

`offset = i * 96 + j * 32 + k * 8`

Which simplifies to:

`offset = (i * axis1 * axis2 + j * axis2 + k) * element_size`

In row-major order, the 3D index components are combined with the sizes of the later axes because the later axes are the fastest-moving axes in the linear buffer. The term `j * axis2` moves between columns inside a row, and `i * axis1 * axis2` jumps whole rows across all columns and depths.

Using the same logical slice layout, the row-major slices look the same as the logical array but are packed differently in memory:

```
Slice 0 (Depth index = 0)

        Col 0    Col 1    Col 2    Col 3 
Row 0:    1        2        3        4
Row 1:    5        6        7        8
Row 2:    9        10       11       12   
```
```
Slice 1 (Depth index = 1)

        Col 0    Col 1    Col 2    Col 3 
Row 0:   13       14       15       16
Row 1:   17       18       19       20
ROW 2:   21       22       23       24   

```

The difference is not in the logical positions of the values, but in how the contiguous linear buffer is traversed. Row-major and column-major layouts interpret the same multi-dimensional coordinates with different stride patterns, which is why knowing the stride formula is essential for mapping indices into memory correctly.

**Transpose of Matrices**

In a 2D matrix, a transpose has only one meaning: swap rows and columns. In a 3D tensor (or higher), there are multiple ways to shuffle the axes. For example, you could swap just the first two dimensions, swap the last two, or completely reverse all three. You must specify which axes are changing.

Logical vs. Physical Changes:
1. Logical (Both): A transpose changes the index lookups. An element originally at coordinates [i, j, k] moves to a new coordinate pattern like [k, j, i]. This logical mapping is identical whether your system uses row-major or column-major format

2. Physical (Row-Major): In row-major, a standard transpose swaps rows and columns, so the original last dimension becomes the new first dimension and the original first dimension becomes the new last dimension. In row-major layout, the last axis is the fastest-moving axis in memory, and after transpose the new last dimension corresponds to the original first axis.

```
A standard transpose of a 3×4 matrix:

        fast-moving index = original last dimension
        Col 0    Col 1    Col 2    Col 3
Row 0:    1        2        3        4
Row 1:    5        6        7        8
Row 2:    9       10       11       12   

        new shape after transpose: 4 rows × 3 columns
        new last dimension = original first dimension
        Col 0    Col 1    Col 2
Row 0:    1       5        9     
Row 1:    2       6       10  
Row 2:    3       7       11
Row 3:    4       8       12
```

3. Physical (Column-Major): In column-major, a standard transpose swaps rows and columns, so the original first dimension becomes the new last dimension and the original last dimension becomes the new first dimension. In column-major layout, the first axis is the fastest-moving axis in memory, and after transpose the new first dimension corresponds to the original last axis.

```
A standard transpose of a 4x3 matrix stored column-major:

        fast-moving index = original first dimension
        Col 0    Col 1    Col 2
Row 0:    1       5        9     
Row 1:    2       6       10  
Row 2:    3       7       11
Row 3:    4       8       12

        new shape after transpose: 3 rows × 4 columns
        new first dimension = original last dimension
        Col 0    Col 1    Col 2    Col 3
Row 0:    1        2        3        4
Row 1:    5        6        7        8
Row 2:    9       10       11       12
```

**Transposing a 3D column major matrix**

In column-major order, the first axis is the fastest-moving index in memory, followed by the second and then the third. For a tensor with shape `(axis0, axis1, axis2) = (2, 3, 4)`, the flat offset for element `[i, j, k]` is:

`offset = (i + j * axis0 + k * axis0 * axis1) * element_size`

With `double` values (8 bytes), the strides are:

- `stride_axis0 = 8` bytes
- `stride_axis1 = 2 * 8 = 16` bytes
- `stride_axis2 = 2 * 3 * 8 = 48` bytes

A 3D transpose is not a single operation. You must specify which axes are swapped. The standard 3D permutation is to swap the first and last axes, which turns the shape from `(2, 3, 4)` into `(4, 3, 2)`. In index notation, the logical mapping is:

`old [i, j, k] -> new [k, j, i]`

This means the tensor data is reinterpreted with a different axis order, while the underlying column-major rule still says that the first axis varies fastest. If you create a new contiguous tensor in the transposed shape, the new stride tuple becomes:

- `stride_axis0 = 8` bytes
- `stride_axis1 = 4 * 8 = 32` bytes
- `stride_axis2 = 4 * 3 * 8 = 96` bytes

The original layout is shown below for reference:

```
3D Matrix Mapping (On Paper) with dimensions [2, 3, 4] and stride [8, 16, 48] for type double on x64

       DEPTH k = 0                     DEPTH k = 1
   j=0     j=1     j=2             j=0     j=1     j=2
+-------+-------+-------+       +-------+-------+-------+

|   1   |   3   |   5   | i=0   |   7   |   9   |  11   | i=0
+-------+-------+-------+       +-------+-------+-------+

|   2   |   4   |   6   | i=1   |   8   |  10   |  12   | i=1
+-------+-------+-------+       +-------+-------+-------+

       DEPTH k = 2                     DEPTH k = 3
   j=0     j=1     j=2             j=0     j=1     j=2
+-------+-------+-------+       +-------+-------+-------+

|  13   |  15   |  17   | i=0   |  19   |  21   |  23   | i=0
+-------+-------+-------+       +-------+-------+-------+

|  14   |  16   |  18   | i=1   |  20   |  22   |  24   | i=1
+-------+-------+-------+       +-------+-------+-------+
```

After swapping the first and last axes, the new tensor has shape `(4, 3, 2)`. The data is now interpreted as follows:

```
Transposed 3D Matrix Mapping (On Paper) with dimensions [4, 3, 2] and stride [8, 32, 96] for type double on x64

               DEPTH k = 0                                     DEPTH k = 1
       j=0         j=1         j=2                     j=0         j=1         j=2
  +-----------+-----------+-----------+           +-----------+-----------+-----------+

  |     1     |     3     |     5     | i=0       |     2     |     4     |     6     | i=0
  +-----------+-----------+-----------+           +-----------+-----------+-----------+

  |     7     |     9     |    11     | i=1       |     8     |    10     |    12     | i=1
  +-----------+-----------+-----------+           +-----------+-----------+-----------+

  |    13     |    15     |    17     | i=2       |    14     |    16     |    18     | i=2
  +-----------+-----------+-----------+           +-----------+-----------+-----------+

  |    19     |    21     |    23     | i=3       |    20     |    22     |    24     | i=3
  +-----------+-----------+-----------+           +-----------+-----------+-----------+
```

New layout properties:

- New dimensions: `4 × 3 × 2`
- New logical rule: element `[i, j, k]` in the original tensor moves to `[k, j, i]` in the transposed tensor
- Column-major rule is preserved: the first axis still changes fastest in memory

This is the important distinction: a 3D transpose is an axis permutation, not a special 2D matrix transpose. The same values remain the same; only the coordinate system used to index them changes.

**Transposing a 3D row major matrix**

Here is the visual mapping and stride configuration for a 2 × 3 × 4 array of double data types (8 bytes each) stored in row-major order.

Stride Array:

In row-major order, the last dimension changes fastest. The strides represent how many bytes you must skip in memory to advance by 1 index along each dimension so the stride array is [96, 32, 8]

```
3D Matrix Mapping (On Paper)

       DEPTH i = 0                     DEPTH i = 1
   k=0   k=1   k=2   k=3           k=0   k=1   k=2   k=3
+-----+-----+-----+-----+       +-----+-----+-----+-----+

|  1  |  2  |  3  |  4  | j=0   | 13  | 14  | 15  | 16  | j=0
+-----+-----+-----+-----+       +-----+-----+-----+-----+

|  5  |  6  |  7  |  8  | j=1   | 17  | 18  | 19  | 20  | j=1
+-----+-----+-----+-----+       +-----+-----+-----+-----+

|  9  | 10  | 11  | 12  | j=2   | 21  | 22  | 23  | 24  | j=2
+-----+-----+-----+-----+       +-----+-----+-----+-----+
```

When you transpose a 3D tensor completely, the dimensions reverse from 2 × 3 × 4 to 4 × 3 × 2. The original element at position [i, j, k] now maps to [k, j, i].

New Stride Array (Row-Major) is [48, 16, 8]

```
Transposed 3D Matrix Mapping (On Paper)

       DEPTH k = 0                     DEPTH k = 1
   i=0   i=1                       i=0   i=1
+-----+-----+                     +-----+-----+

|  1  | 13  | j=0                 |  2  | 14  | j=0
+-----+-----+                     +-----+-----+

|  5  | 17  | j=1                 |  6  | 18  | j=1
+-----+-----+                     +-----+-----+

|  9  | 21  | j=2                 | 10  | 22  | j=2
+-----+-----+                     +-----+-----+

       DEPTH k = 2                     DEPTH k = 3
   i=0   i=1                       i=0   i=1
+-----+-----+                     +-----+-----+

|  3  | 15  | j=0                 |  4  | 16  | j=0
+-----+-----+                     +-----+-----+

|  7  | 19  | j=1                 |  8  | 20  | j=1
+-----+-----+                     +-----+-----+

| 11  | 23  | j=2                 | 12  | 24  | j=2
+-----+-----+                     +-----+-----+
```

---

This document is licensed. See the full license text here: https://github.com/KHAAdotPK/LICENSE/blob/main/LICENSE

