// comp_decomp.hpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#ifndef COMP_DECOMP_HPP_
#define COMP_DECOMP_HPP_    4   // Version 4

// int zlib_comp(std::string& output, const void *input, uInt input_size, int rate = 9);
// int zlib_decomp(std::string& output, const void *input, uInt input_size);
// bool zlib_unittest(void);
#ifdef HAVE_ZLIB
    #include "comp_decomp_zlib.hpp"
#endif  // def HAVE_ZLIB

// int bzlib_comp(std::string& output, const void *input,
//                unsigned int input_size, int rate = 9);
// int bzlib_decomp(std::string& output, const void *input,
//                  unsigned int input_size);
// bool bzlib_unittest(void);
#ifdef HAVE_BZLIB
    #include "comp_decomp_bzlib.hpp"
#endif  // def HAVE_BZLIB

// lzma_ret lzma_comp(std::string& output, const void *input, size_t input_size, int rate = 9);
// lzma_ret lzma_decomp(std::string& output, const void *input, size_t input_size);
// bool lzma_unittest(void);
#ifdef HAVE_LZMA
    #include "comp_decomp_lzma.hpp"
#endif  // def HAVE_LZMA

#endif  // ndef COMP_DECOMP_HPP_
