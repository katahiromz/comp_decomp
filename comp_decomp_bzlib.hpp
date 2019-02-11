// comp_decomp_bzlib.hpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#ifndef COMP_DECOMP_BZLIB_HPP_
#define COMP_DECOMP_BZLIB_HPP_

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>

#ifndef COMP_DECOMP_BUFFSIZE
    #define COMP_DECOMP_BUFFSIZE (8 * 1024)
#endif

// int bzlib_comp(std::string& output, const void *input,
//                unsigned int input_size, int rate = 9);
// int bzlib_decomp(std::string& output, const void *input,
//                  unsigned int input_size);
// const char *bzlib_errmsg(int ret);
// bool bzlib_unittest(void);

#ifdef HAVE_BZLIB
    #include <bzlib.h>

    inline int bzlib_comp(std::string& output, const void *input,
                          unsigned input_size, int rate = 9)
    {
        output.clear();
        output.reserve(input_size * 2 / 3);
        static char inbuf[COMP_DECOMP_BUFFSIZE];
        assert(1 <= rate && rate <= 9);

        bz_stream strm;
        memset(&strm, 0, sizeof(strm));
        strm.bzalloc = NULL;
        strm.bzfree = NULL;
        strm.opaque = NULL;
        strm.next_in = (char *)input;
        strm.avail_in = input_size;

        int ret = BZ2_bzCompressInit(&strm, rate, 0, 0);
        if (ret != BZ_OK)
            return ret;

        output.resize(COMP_DECOMP_BUFFSIZE);
        strm.next_out = (char *)output.data();
        strm.avail_out = COMP_DECOMP_BUFFSIZE;

        int action = BZ_RUN;
        ret = BZ_RUN_OK;
        while (ret == BZ_RUN_OK || ret == BZ_FLUSH_OK || ret == BZ_FINISH_OK)
        {
            if (strm.avail_in == 0)
            {
                action = BZ_FINISH;
            }

            if (strm.avail_out == 0)
            {
                output.resize(output.size() + COMP_DECOMP_BUFFSIZE);
                strm.next_out = (char *)(output.data() + output.size() - COMP_DECOMP_BUFFSIZE);
                strm.avail_out = COMP_DECOMP_BUFFSIZE;
            }

            ret = BZ2_bzCompress(&strm, action);
        }

        if (ret != BZ_STREAM_END)
        {
            BZ2_bzCompressEnd(&strm);
            output.clear();
            return ret;
        }

        output.resize(output.size() - strm.avail_out);
        return BZ2_bzCompressEnd(&strm);
    }

    inline int bzlib_decomp(std::string& output, const void *input,
                            unsigned int input_size)
    {
        output.clear();
        output.reserve(input_size * 3 / 2);
        static uint8_t inbuf[COMP_DECOMP_BUFFSIZE];

        bz_stream strm;
        memset(&strm, 0, sizeof(strm));
        strm.bzalloc = NULL;
        strm.bzfree = NULL;
        strm.opaque = NULL;
        strm.next_in = (char *)input;
        strm.avail_in = input_size;
        int ret = BZ2_bzDecompressInit(&strm, 0, 0);
        if (ret != BZ_OK)
            return ret;

        output.resize(COMP_DECOMP_BUFFSIZE);
        strm.next_out  = (char *)(output.data());
        strm.avail_out = (unsigned)output.size();

        ret = BZ_OK;
        while (ret == BZ_OK || ret == BZ_FLUSH_OK || ret == BZ_FINISH_OK)
        {
            if (strm.avail_out == 0)
            {
                output.resize(output.size() + COMP_DECOMP_BUFFSIZE);
                strm.next_out  = (char *)(output.data() + output.size() - COMP_DECOMP_BUFFSIZE);
                strm.avail_out = COMP_DECOMP_BUFFSIZE;
            }

            ret = BZ2_bzDecompress(&strm);
        }

        if (ret != BZ_STREAM_END)
        {
            output.clear();
            BZ2_bzDecompressEnd(&strm);
            return ret;
        }

        output.resize(output.size() - strm.avail_out);

        return BZ2_bzDecompressEnd(&strm);
    }

    inline const char *bzlib_errmsg(int ret)
    {
        switch (ret)
        {
        case BZ_OK: return "success";
        case BZ_SEQUENCE_ERROR: return "sequence error (BZ_SEQUENCE_ERROR)";
        case BZ_PARAM_ERROR: return "parameter error (BZ_PARAM_ERROR)";
        case BZ_MEM_ERROR: return "insufficient memory (BZ_MEM_ERROR)";
        case BZ_DATA_ERROR: return "data error (BZ_DATA_ERROR)";
        case BZ_DATA_ERROR_MAGIC: return "magic bytes error (BZ_DATA_ERROR_MAGIC)";
        case BZ_IO_ERROR: return "io error (BZ_IO_ERROR)";
        case BZ_UNEXPECTED_EOF: return "unexpected EOF (BZ_UNEXPECTED_EOF )";
        case BZ_OUTBUFF_FULL: return "out buffer full (BZ_OUTBUFF_FULL)";
        case BZ_CONFIG_ERROR: return "config error (BZ_CONFIG_ERROR)";
        }
        return "Unknown error";
    }

    inline bool bzlib_test_entry(const std::string& original)
    {
        std::string encoded, decoded;
        if (int ret = bzlib_comp(encoded, original.c_str(), (unsigned)original.size()))
        {
            printf("bzlib_comp failed: %s\n", bzlib_errmsg(ret));
            return false;
        }
        if (int ret = bzlib_decomp(decoded, encoded.c_str(), (unsigned)encoded.size()))
        {
            printf("bzlib_decomp failed: %s\n", bzlib_errmsg(ret));
            return false;
        }
        if (!(original == decoded))
        {
            printf("bzlib mismatch\n");
            return false;
        }
        return true;
    }

#ifndef COMP_DECOMP_MAX_TEST
    #define COMP_DECOMP_MAX_TEST 100
#endif
#ifndef COMP_DECOMP_TEST_COUNT
    #define COMP_DECOMP_TEST_COUNT 100
#endif

    inline bool bzlib_unittest(void)
    {
        std::string original;
        if (!bzlib_test_entry(original))
            return false;

        original.assign(COMP_DECOMP_MAX_TEST, 'A');
        if (!bzlib_test_entry(original))
            return false;

        for (size_t i = 0; i < COMP_DECOMP_TEST_COUNT; ++i)
        {
            size_t len = std::rand() % COMP_DECOMP_MAX_TEST;
            original.resize(len);
            for (size_t k = 0; k < len; ++k)
            {
                original[k] = (char)(std::rand() & 0xFF);
            }
            if (!bzlib_test_entry(original))
                return false;
        }
        return true;
    }
 #endif  // def HAVE_BZLIB

#endif  // ndef COMP_DECOMP_BZLIB_HPP_
