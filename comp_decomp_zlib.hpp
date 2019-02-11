// comp_decomp_zlib.hpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#ifndef COMP_DECOMP_ZLIB_HPP_
#define COMP_DECOMP_ZLIB_HPP_

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>

#ifndef COMP_DECOMP_BUFFSIZE
    #define COMP_DECOMP_BUFFSIZE (8 * 1024)
#endif

// int zlib_comp(std::string& output, const void *input, uInt input_size, int rate = 9);
// int zlib_decomp(std::string& output, const void *input, uInt input_size);
// const char *zlib_errmsg(int ret);
// bool zlib_unittest(void);

#ifdef HAVE_ZLIB
    #include <zlib.h>

    inline int zlib_comp(std::string& output, const void *input, uInt input_size, int rate = 9)
    {
        const Bytef *ptr = (const Bytef *)input;
        uInt remainder = input_size;
        assert(1 <= rate && rate <= 9);

        output.clear();
        output.reserve(input_size * 2 / 3);
        static Bytef inbuf[COMP_DECOMP_BUFFSIZE];
        static Bytef outbuf[COMP_DECOMP_BUFFSIZE];

        z_stream strm;
        memset(&strm, 0, sizeof(strm));
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        int ret = deflateInit(&strm, rate);
        if (ret != Z_OK)
            return ret;

        strm.next_in = NULL;
        strm.avail_in = 0;
        strm.next_out = outbuf;
        strm.avail_out = sizeof(outbuf);

        int action = Z_NO_FLUSH;
        while (ret == Z_OK || ret == Z_BUF_ERROR)
        {
            if (strm.avail_in == 0)
            {
                strm.next_in = inbuf;
                if (remainder < COMP_DECOMP_BUFFSIZE)
                    strm.avail_in = remainder;
                else
                    strm.avail_in = COMP_DECOMP_BUFFSIZE;
                memcpy(inbuf, ptr, strm.avail_in);
                ptr += strm.avail_in;
                remainder -= strm.avail_in;

                if (remainder == 0)
                    action = Z_FINISH;
            }

            ret = deflate(&strm, action);

            if (strm.avail_out == 0 || ret == Z_STREAM_END || ret == Z_BUF_ERROR)
            {
                size_t write_size = sizeof(outbuf) - strm.avail_out;
                output.insert(output.size(), (char *)outbuf, write_size);
                strm.next_out = outbuf;
                strm.avail_out = sizeof(outbuf);
            }
        }

        return deflateEnd(&strm);
    }

    inline int zlib_decomp(std::string& output, const void *input, uInt input_size)
    {
        const Bytef *ptr = (const Bytef *)input;
        uInt remainder = input_size;

        output.clear();
        output.reserve(input_size * 3 / 2);
        static Bytef inbuf[COMP_DECOMP_BUFFSIZE];
        static Bytef outbuf[COMP_DECOMP_BUFFSIZE];

        z_stream strm;
        memset(&strm, 0, sizeof(strm));
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        int ret = inflateInit(&strm);
        if (ret != Z_OK)
            return ret;

        strm.next_in = NULL;
        strm.avail_in = 0;
        strm.next_out = outbuf;
        strm.avail_out = sizeof(outbuf);

        int action = Z_NO_FLUSH;
        while (ret == Z_OK || ret == Z_BUF_ERROR)
        {
            if (strm.avail_in == 0)
            {
                strm.next_in = inbuf;
                if (remainder < COMP_DECOMP_BUFFSIZE)
                    strm.avail_in = remainder;
                else
                    strm.avail_in = COMP_DECOMP_BUFFSIZE;
                memcpy(inbuf, ptr, strm.avail_in);
                ptr += strm.avail_in;
                remainder -= strm.avail_in;

                if (remainder == 0)
                    action = Z_FINISH;
            }

            ret = inflate(&strm, action);

            if (strm.avail_out == 0 || ret == Z_STREAM_END || ret == Z_BUF_ERROR)
            {
                size_t write_size = sizeof(outbuf) - strm.avail_out;
                output.insert(output.size(), (const char *)outbuf, write_size);
                strm.next_out = outbuf;
                strm.avail_out = sizeof(outbuf);
            }
        }

        return inflateEnd(&strm);
    }

    inline const char *zlib_errmsg(int ret)
    {
        switch (ret)
        {
        case Z_OK: return "success (Z_OK)";
        case Z_ERRNO:
            if (ferror(stdin))
                return "error reading stdin (Z_ERRNO)";
            if (ferror(stdout))
                return "error writing stdout (Z_ERRNO)";
            return "error writing file (Z_ERRNO)";
        case Z_STREAM_ERROR: return "invalid compression level (Z_STREAM_ERROR)";
        case Z_DATA_ERROR: return "invalid or incomplete deflate data (Z_DATA_ERROR)";
        case Z_MEM_ERROR: return "out of memory (Z_MEM_ERROR)";
        case Z_VERSION_ERROR: return "zlib version mismatch! (Z_VERSION_ERROR)";
        }
        return "unknown error";
    }

    inline bool zlib_test_entry(const std::string& original)
    {
        std::string encoded, decoded;
        if (int ret = zlib_comp(encoded, original.c_str(), (uInt)original.size()))
        {
            printf("zlib_comp failed: %s\n", zlib_errmsg(ret));
            return false;
        }
        if (int ret = zlib_decomp(decoded, encoded.c_str(), (uInt)encoded.size()))
        {
            printf("zlib_decomp failed: %s\n", zlib_errmsg(ret));
            return false;
        }
        if (!(original == decoded))
        {
            printf("zlib mismatch\n");
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

    inline bool zlib_unittest(void)
    {
        std::string original;
        if (!zlib_test_entry(original))
            return false;

        original.assign(COMP_DECOMP_MAX_TEST, 'A');
        if (!zlib_test_entry(original))
            return false;

        for (size_t i = 0; i < COMP_DECOMP_TEST_COUNT; ++i)
        {
            size_t len = std::rand() % COMP_DECOMP_MAX_TEST;
            original.resize(len);
            for (size_t k = 0; k < len; ++k)
            {
                original[k] = (char)(std::rand() & 0xFF);
            }
            if (!zlib_test_entry(original))
                return false;
        }
        return true;
    }
#endif  // def HAVE_ZLIB

#endif  // ndef COMP_DECOMP_ZLIB_HPP_
