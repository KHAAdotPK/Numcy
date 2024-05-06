 ## NumCy
The `Numcy` class is designed for numerical computing operations, and it includes functionality for array creation, manipulation, and mathematical operations.

The following code demonstrates the usage of various `Numcy` functions, such as zeros, triu, concatenate, dot and printing the results of these operations. 
```C++
/*
    src/main.cpp
    Q@khaa.pk
 */

#include "../lib/numcy/numcy.hh"

int main(int argc, char *argv[])
{
    /////////////////////
    // Numcy::zeros() //
    ////////////////////
    std::cout<< "Numcy::zeros()" << std::endl; 
    // Create a 3D array initialized with zeros   
    DIMENSIONS_PTR dim = reinterpret_cast<DIMENSIONS_PTR>(cc_tokenizer::allocator<unsigned char>().allocate(sizeof(DIMENSIONS)));    
    *dim = {10, 2, NULL, NULL};
    Collective<float> ret = Numcy::zeros<float>({0, 1, dim, NULL});
    dim->prev = &(ret.getShape());

    // Display the created array
    cc_tokenizer::string_character_traits<char>::size_type rows = NUMBER_OF_ROWS_IN_INNER_MOST_ARRAY(ret);
    cc_tokenizer::string_character_traits<char>::size_type columns = NUMBER_OF_COLUMNS_IN_INNER_MOST_ARRAYS(ret);
    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (ret.getShape().getN() / (rows*columns)); i++)
    {                     
        for (cc_tokenizer::string_character_traits<char>::size_type q = 0; q < rows; q++)
        {
            for (cc_tokenizer::string_character_traits<char>::size_type r = 0; r < columns; r++)
            {                
                std::cout<< ret[i*rows*columns + q*columns + r] << " ";
            }
            std::cout<<std::endl;
        }                                 
    }

    ////////////////////
    // Numcy::triu() //
    ///////////////////
    std::cout<< "Numcy::triu()" << std::endl;
    // Create an upper triangular matrix    
    ret =  Numcy::triu<float>(Numcy::ones<float>({8, 4, NULL, NULL}), -2, false);   
    
    // Display the upper triangular matrix
    rows = NUMBER_OF_ROWS_IN_INNER_MOST_ARRAY(ret);
    columns = NUMBER_OF_COLUMNS_IN_INNER_MOST_ARRAYS(ret);
    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (ret.getShape().getN() / (rows*columns)); i++)
    {                     
        for (cc_tokenizer::string_character_traits<char>::size_type q = 0; q < rows; q++)
        {
            for (cc_tokenizer::string_character_traits<char>::size_type r = 0; r < columns; r++)
            {                
                std::cout<< ret[i*rows*columns + q*columns + r] << " ";
            }
            std::cout<<std::endl;
        }                                 
    }

    ///////////////////////////
    // Numcy::concatinate() //
    //////////////////////////    
    std::cout<< "Numcy::concatinate()" << std::endl;
    // Create two arrays and concatenate them
    float *ptr = cc_tokenizer::allocator<float>().allocate(10);
    for (int i = 0; i < 10; i++)
    {
        ptr[i] = i + 1;
    }
    Collective<float> is = Collective<float>{ptr, {10, 1, NULL, NULL}};
    ptr = cc_tokenizer::allocator<float>().allocate(10*10);
    for (int i = 200; i < (200 + 10*10); i++)
    {
        ptr[i - 200] = i + 1;
    }
    Collective<float> pe = Collective<float>{ptr, {10, 10, NULL, NULL}};
    Collective<float> ei = Numcy::concatenate(pe, is);

    // Display the concatenated array
    rows = NUMBER_OF_ROWS_IN_INNER_MOST_ARRAY(ei); 
    columns = NUMBER_OF_COLUMNS_IN_INNER_MOST_ARRAYS(ei);    
    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (ei.getShape().getN() / (rows*columns)); i++)
    {                     
        for (cc_tokenizer::string_character_traits<char>::size_type q = 0; q < rows; q++)
        {
            for (cc_tokenizer::string_character_traits<char>::size_type r = 0; r < columns; r++)
            {                
                std::cout<< ei[i*rows*columns + q*columns + r] << " ";
            }
            std::cout<<std::endl;
        }                                 
    }

    ///////////////////
    // Numcy::dot() //
    //////////////////
    std::cout<< "Numcy::dot()" << std::endl;
    // Create two matrices and compute their dot product
    /*
    float multiplicand[] = {2.3, 3.4, 1.2, 1.0};
    float multiplier[][2] = { {2.3, 8.9},
                              {1.0, 1.0},
                              {9.0, 2.3},
                              {3.2, 8.9} };
     */
    float *multiplicand = cc_tokenizer::allocator<float>().allocate(4);
    float *multiplier = cc_tokenizer::allocator<float>().allocate(4*2);
    multiplicand[0] = 2.3, multiplicand[1] = 3.4, multiplicand[2] = 1.2, multiplicand[3] = 1.0;
    ((float (*)[2])multiplier)[0][0] = 2.3, ((float (*)[2])multiplier)[0][1] = 8.9;
    ((float (*)[2])multiplier)[1][0] = 1.0, ((float (*)[2])multiplier)[1][1] = 1.0;
    ((float (*)[2])multiplier)[2][0] = 9.0, ((float (*)[2])multiplier)[2][1] = 2.3;
    ((float (*)[2])multiplier)[3][0] = 3.2, ((float (*)[2])multiplier)[3][1] = 8.9;
    // The left operand of the dot product should have the same number of columns as the number of rows in the right operand.
    Collective<float> a = Collective<float>{multiplicand, {4, 1, NULL, NULL}};
    Collective<float> b = Collective<float>{multiplier, {2, 4, NULL, NULL}};    
    ret = Numcy::dot(a, b); // The product

    // Display the dot product
    rows = NUMBER_OF_ROWS_IN_INNER_MOST_ARRAY(ret); 
    columns = NUMBER_OF_COLUMNS_IN_INNER_MOST_ARRAYS(ret);    
    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < (ret.getShape().getN() / (rows*columns)); i++)
    {                     
        for (cc_tokenizer::string_character_traits<char>::size_type q = 0; q < rows; q++)
        {
            for (cc_tokenizer::string_character_traits<char>::size_type r = 0; r < columns; r++)
            {                
                std::cout<< ret[i*rows*columns + q*columns + r] << " ";
            }
            std::cout<<std::endl;
        }                                 
    }
    
    return 0;
}
```