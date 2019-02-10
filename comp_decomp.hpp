// comp_decomp.hpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#ifndef COMP_DECOMP_HPP_
#define COMP_DECOMP_HPP_    3   // Version 3

// bool zlib_compress(std::string& output, const void *input, size_t input_size, int rate = 9);
// bool zlib_decompress(std::string& output, const void *input, size_t input_size);
// bool zlib_unittest(void);
#ifdef HAVE_ZLIB
    #include "comp_decomp_zlib.hpp"
#endif  // def HAVE_ZLIB

// bool bzlib_compress(std::string& output, const void *input, size_t input_size, int rate = 9);
// bool bzlib_decompress(std::string& output, const void *input, size_t input_size);
// bool bzlib_unittest(void);
#ifdef HAVE_BZLIB
    #include "comp_decomp_bzlib.hpp"
#endif  // def HAVE_BZLIB

// bool lzma_compress(std::string& output, const void *input, size_t input_size, int rate = 9);
// bool lzma_decompress(std::string& output, const void *input, size_t input_size);
// bool lzma_unittest(void);
#ifdef HAVE_LZMA
    #include "comp_decomp_lzma.hpp"
#endif  // def HAVE_LZMA

#endif  // ndef COMP_DECOMP_HPP_
