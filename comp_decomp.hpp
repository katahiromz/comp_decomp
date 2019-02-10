#ifndef COMP_DECOMP_HPP_
#define COMP_DECOMP_HPP_

#undef min

#include <cstdlib>
#include <cstdint>
#include <string>
#include <algorithm>

#ifndef COMP_DECOMP_BUFFSIZE
    #define COMP_DECOMP_BUFFSIZE (8 * 1024)
#endif
#ifndef COMP_DECOM_MIN_TEST
    #define COMP_DECOM_MIN_TEST 0
#endif
#ifndef COMP_DECOM_MAX_TEST
    #define COMP_DECOM_MAX_TEST 100
#endif
#ifndef COMP_DECOM_TEST_STEP
    #define COMP_DECOM_TEST_STEP 10
#endif
#ifndef COMP_DECOM_COMPRESSION_RATE
    #define COMP_DECOM_COMPRESSION_RATE 9
#endif

#ifdef HAVE_ZLIB
    #include "comp_decomp_zlib.hpp"
#endif  // def HAVE_ZLIB

#ifdef HAVE_BZLIB
    #include "comp_decomp_bzlib.hpp"
#endif  // def HAVE_BZLIB

#ifdef HAVE_LZMA
    #include "comp_decomp_lzma.hpp"
#endif  // def HAVE_LZMA

#endif  // ndef COMP_DECOMP_HPP_
