# NumCy C++ Library
`Numcy` is a `C++` library aimed at providing functionalities similar to the popular `Python` `NumPy` library. It offers a set of classes and functions to work with multi-dimensional arrays, perform mathematical operations, and support numerical computing in `C++`.

## Dependencies

Ensure you have the following dependencies cloned into the `./lib` directory before using `NumCy`:

- [string](https://github.com/KHAAdotPK/string.git)
- [ala_exception](https://github.com/KHAAdotPK/ala_exception.git)

## Classes

## `DimensionsOfArray` Class
The `DimensionsOfArray` class is a utility class that helps manage dynamically allocated arrays for storing dimensions.
#### Example Usage:
```C++
/*
    Creating and comparing instances of DimensionsOfArray for two three-dimensional arrays.

    1. Creating array 'arrayA' with dimensions [2][3][10]. A three dimensional array, with six inner arrays(two three-dimensional arrays), and then each inner array is a single line array(the innermost array)  of 10 columns.
    2. Initializing DimensionsOfArray 'dimsA' with 'arrayA' dimensions.
    3. Displaying the size (number of dimensions) and the number of inner arrays for 'arrayA'.
    
    4. Creating array 'arrayB' with dimensions [2][3][8]. A three dimensional array, with six inner arrays(two  three-dimensional arrays), and then each inner array is a single line array(the innermost array) of 8 columns.
    5. Initializing DimensionsOfArray 'dimsB' with 'arrayB' dimensions.
    6. Comparing the number of inner arrays between 'arrayA' and 'arrayB'.
        - If equal, output a message indicating the similarity.
*/
cc_tokenizer::string_character_traits<char>::size_type arrayA[] = {2, 3, 10};
DimensionsOfArray dimsA(arrayA, sizeof(arrayA)/sizeof(cc_tokenizer::string_character_traits<char>::size_type));
// Displaying the dimensions of the array and the number of inner arrays.
std::cout << "Size (number of dimensions) = " << dimsA.size() << std::endl;
std::cout<< dimsA.getNumberOfInnerArrays() << std::endl;
cc_tokenizer::string_character_traits<char>::size_type arrayB[] = {2, 3, 8};
DimensionsOfArray dimsB(arrayB, sizeof(arrayB)/sizeof(cc_tokenizer::string_character_traits<char>::size_type));    
if ( dimsB.compare(dimsA) ) 
{
    std::cout<< "They have same number of inner arrays..." << std::endl;
}
/*
    Outputting the dimensions of the first three-dimensional array 'arrayA' using the overloaded operator.

    Iterating through each dimension using a loop:
    - 'dimsA.size()' provides the total number of dimensions.
    - Printing the ith dimension value using the overloaded 'operator[]'.

    Example:
    For the array 'arrayA' with dimensions [2][3][10], the output would be: 2 3 10
*/
for (size_t i = 0; i < dimsA.size(); i++)
{
    std::cout << dimsA[i] << " ";
}
std::cout << std::endl;
```

## `Dimensions` Class
The `Dimensions` class represents the shape and size of a multi-dimensional array. It holds information about the number of dimensions, size along each dimension, and pointers to previous and next `Dimensions` instances.
#### Example Usage:
```C++
/*    
    Manually initializing a doubly linked list to represent the dimensions
    of a multi-dimensional array in C/C++.

    The array has the shape [9][78][10][10][3].

    The DIMENSIONS structure is used to define each dimension node,
    where 'size' represents the size of the dimension, and 'prev' and 'next'
    pointers establish the doubly linked list connections.

    The linked list is constructed as follows:
        dim -> dim1 -> dim2 -> dim3

    Each dimension node is initialized with its respective size,
    and the 'prev' pointers are set to establish the backward connections.

    Example:
        dim (0, 9) -> dim1 (0, 78) -> dim2 (0, 10) -> dim3 (10, 3)
*/
cc_tokenizer::string_character_traits<char>::size_type *ptr = cc_tokenizer::allocator<cc_tokenizer::string_character_traits<char>::size_type>().allocate(5);
   
    // I died a little
    ptr[0] = 9;
    ptr[1] = 78;
    ptr[2] = 10;
    ptr[3] = 3;
    ptr[4] = 10;

    DIMENSIONSOFARRAY dimensionsOfArray(ptr, 5);

    DIMENSIONS dim(dimensionsOfArray);

    std::cout<< "Number of inner arrays = " << dimensionsOfArray.getNumberOfInnerArrays() << std::endl;
    std::cout<< "Size of inner most array = " << dimensionsOfArray[dimensionsOfArray.size() - 1] << std::endl;
    std::cout<< "Size = " << dimensionsOfArray.size() << std::endl;
    
    for (int i = 0; i < dimensionsOfArray.size(); i++)
    {
        std::cout<< dimensionsOfArray[i] << " ";
    }
    
    DIMENSIONS_PTR current = dim.copy();

    std::cout<< "\nDimensions... " << std::endl;
    while (current != NULL)
    {
        std::cout<< "Columns: " << current->columns << ", Rows: " << current->rows << std::endl;        
        current = current->next;
    }

    std::cout<< "Size of array: " << dim.getN() << std::endl; 

    cc_tokenizer::allocator<cc_tokenizer::string_character_traits<char>::size_type>().deallocate(ptr);
```

## `Collective` Class
The `Collective` class is a wrapper for multi-dimensional arrays. It encapsulates the data array along with its shape and provides convenient accessors and operations.
#### Example Usage:
```C++
float* data = new float[10];
Dimensions shape = {10, 1, NULL, NULL};
Collective<float> array(data, shape);
```
## `Numcy` Class
The `Numcy` class contains static methods that provide various mathematical and array operations, such as creating arrays filled with zeros, computing dot products, performing concatenation, and more.
### Example Usage:
```C++
Collective<float> zerosArray = Numcy::zeros<float>({0, 1, &shape, NULL});
```

## Getting Started
1. Include the `NumCy` library in your `C++` project. You can do this by adding the necessary source files to your project or linking against the compiled Numcy library.
2. Use the provided classes and functions to perform numerical computations similar to `NumPy` in `Python`. The library offers a range of functionalities, from creating multi-dimensional arrays to performing mathematical operations.
3. Reference the `NumCy` documentation for detailed information on each function. The documentation, although currently in progress, will include examples and explanations to help you understand and utilize the capabilities of the `NumCy` library effectively.
4. Explore the [USAGE.md](USAGE.md) file for a comprehensive example code that demonstrates the usage of various `NumCy` functions. This example code serves as a practical guide to implementing numerical computations using the `NumCy` library. 

## Contributing
If you find issues or have suggestions for improvement, feel free to contribute!

## License
This project is governed by a license, the details of which can be located in the accompanying file named 'LICENSE.' Please refer to this file for comprehensive information.
