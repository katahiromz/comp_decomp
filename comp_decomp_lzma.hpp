// comp_decomp_lzma.hpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#ifndef COMP_DECOMP_LZMA_HPP_
#define COMP_DECOMP_LZMA_HPP_

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>

#ifndef COMP_DECOMP_BUFFSIZE
    #define COMP_DECOMP_BUFFSIZE (8 * 1024)
#endif

// lzma_ret lzma_comp(std::string& output, const void *input,
//                    size_t input_size, int rate = 9);
// lzma_ret lzma_decomp(std::string& output, const void *input, size_t input_size);
// const char *lzma_errmsg(lzma_ret ret);
// bool lzma_unittest(void);

#ifdef HAVE_LZMA
    #include <lzma.h>

    inline lzma_ret lzma_comp(std::string& output, const void *input,
                              size_t input_size, int rate = 9)
    {
        const uint8_t *ptr = (const uint8_t *)input;
        size_t remainder = input_size;
        assert(1 <= rate && rate <= 9);

        output.clear();
        output.reserve(input_size * 2 / 3);
        static uint8_t inbuf[COMP_DECOMP_BUFFSIZE];
        static uint8_t outbuf[COMP_DECOMP_BUFFSIZE];

        lzma_stream strm = LZMA_STREAM_INIT;
        lzma_ret ret = lzma_easy_encoder(&strm, rate, LZMA_CHECK_CRC64);
        if (ret != LZMA_OK)
            return ret;

        strm.next_in = NULL;
        strm.avail_in = 0;
        strm.next_out = outbuf;
        strm.avail_out = sizeof(outbuf);

        lzma_action action = LZMA_RUN;
        while (ret == LZMA_OK)
        {
            if (strm.avail_in == 0)
            {
                strm.next_in = (const uint8_t *)inbuf;
                if (remainder < COMP_DECOMP_BUFFSIZE)
                    strm.avail_in = remainder;
                else
                    strm.avail_in = COMP_DECOMP_BUFFSIZE;
                memcpy(inbuf, ptr, strm.avail_in);
                ptr += strm.avail_in;
                remainder -= strm.avail_in;

                if (remainder == 0)
                    action = LZMA_FINISH;
            }

            ret = lzma_code(&strm, action);

            if (strm.avail_out == 0 || ret == LZMA_STREAM_END)
            {
                size_t write_size = sizeof(outbuf) - strm.avail_out;
                output.insert(output.size(), (char *)outbuf, write_size);
                strm.next_out = outbuf;
                strm.avail_out = sizeof(outbuf);
            }
        }

        lzma_end(&strm);
        if (ret == LZMA_STREAM_END)
            ret = LZMA_OK;
        return ret;
    }

    inline lzma_ret lzma_decomp(std::string& output, const void *input,
                                size_t input_size)
    {
        const uint8_t *ptr = (const uint8_t *)input;
        size_t remainder = input_size;

        output.clear();
        output.reserve(input_size * 3 / 2);
        static uint8_t inbuf[COMP_DECOMP_BUFFSIZE];
        static uint8_t outbuf[COMP_DECOMP_BUFFSIZE];

        lzma_stream strm = LZMA_STREAM_INIT;
        lzma_ret ret = lzma_stream_decoder(&strm, UINT64_MAX, LZMA_CONCATENATED);
        if (ret != LZMA_OK)
            return ret;

        strm.next_in = NULL;
        strm.avail_in = 0;
        strm.next_out = outbuf;
        strm.avail_out = sizeof(outbuf);

        lzma_action action = LZMA_RUN;
        while (ret == LZMA_OK)
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
                    action = LZMA_FINISH;
            }

            ret = lzma_code(&strm, action);

            if (strm.avail_out == 0 || ret == LZMA_STREAM_END)
            {
                size_t write_size = sizeof(outbuf) - strm.avail_out;

                output.insert(output.size(), (char *)outbuf, write_size);
                strm.next_out = outbuf;
                strm.avail_out = sizeof(outbuf);
            }
        }

        lzma_end(&strm);
        if (ret == LZMA_STREAM_END)
            ret = LZMA_OK;
        return ret;
    }

    inline const char *lzma_errmsg(lzma_ret ret)
    {
        switch (ret)
        {
        case LZMA_OK: return "Operation completed successfully (LZMA_OK)";
        case LZMA_STREAM_END: return "End of stream was reached (LZMA_STREAM_END)";
        case LZMA_NO_CHECK: return "Input stream has no integrity check (LZMA_NO_CHECK)";
        case LZMA_UNSUPPORTED_CHECK: return "Cannot calculate the integrity check (LZMA_UNSUPPORTED_CHECK)";
        case LZMA_GET_CHECK: return "Integrity check type is now available (LZMA_GET_CHECK)";
        case LZMA_MEM_ERROR: return "Cannot allocate memory (LZMA_MEM_ERROR)";
        case LZMA_MEMLIMIT_ERROR: return "Memory usage limit was reached (LZMA_MEMLIMIT_ERROR)";
        case LZMA_FORMAT_ERROR: return "File format not recognized (LZMA_FORMAT_ERROR)";
        case LZMA_OPTIONS_ERROR: return "Invalid or unsupported options (LZMA_OPTIONS_ERROR)";
        case LZMA_DATA_ERROR: return "Data is corrupt (LZMA_DATA_ERROR)";
        case LZMA_BUF_ERROR: return "No progress is possible (LZMA_BUF_ERROR)";
        case LZMA_PROG_ERROR: return "Programming error (LZMA_PROG_ERROR)";
        }
        return "Unknown error";
    }

    inline bool lzma_test_entry(const std::string& original)
    {
        std::string encoded, decoded;
        if (lzma_ret ret = lzma_comp(encoded, original.c_str(), original.size()))
        {
            printf("lzma_comp failed: %s\n", lzma_errmsg(ret));
            return false;
        }
        if (lzma_ret ret = lzma_decomp(decoded, encoded.c_str(), encoded.size()))
        {
            printf("lzma_decomp failed: %s\n", lzma_errmsg(ret));
            return false;
        }
        if (!(original == decoded))
        {
            printf("lzma mismatch\n");
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

    inline bool lzma_unittest(void)
    {
        std::string original;
        if (!lzma_test_entry(original))
            return false;

        original.assign(COMP_DECOMP_MAX_TEST, 'A');
        if (!lzma_test_entry(original))
            return false;

        for (size_t i = 0; i < COMP_DECOMP_TEST_COUNT; ++i)
        {
            size_t len = std::rand() % COMP_DECOMP_MAX_TEST;
            original.resize(len);
            for (size_t k = 0; k < len; ++k)
            {
                original[k] = (char)(std::rand() & 0xFF);
            }
            if (!lzma_test_entry(original))
                return false;
        }
        return true;
    }
#endif  // def HAVE_LZMA

#endif  // ndef COMP_DECOMP_LZMA_HPP_
