/*
    lib/mumcy/macros.hh
    Q@spysee.pk
 */

#ifndef REPLIKA_PK_NUMCY_MACROS_HEADER_HH
#define REPLIKA_PK_NUMCY_MACROS_HEADER_HH

/*
    @a, array of type @tp and size @n 
    @m, multiplyer of type @tp
    @n, size of array @a of type @tp
    @p, pointer to @tp, the new array of the same size as the @a. Each value of @a is multiplyed by @m and it is stored in @p
    @tp, type of multiplyer @m and of array @a and @p
 */
#define MULTIPLY_ARRAY_BY_SCALAR(a, m, n, p, tp)  {\
                                                    cc_tokenizer::allocator<char> alloc_obj;\
                                                    p = reinterpret_cast<tp*>(alloc_obj.allocate(sizeof(tp)*n));\
                                                    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < n; i++)\
                                                    {\
                                                       p[i] = reinterpret_cast<tp*>(a)[i]*m;\
                                                    }\
                                                  }\

#endif