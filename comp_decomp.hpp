#ifndef COMP_DECOMP_HPP_
#define COMP_DECOMP_HPP_

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
