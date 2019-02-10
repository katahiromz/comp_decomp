// comp_decomp_lzma.hpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#ifndef COMP_DECOMP_LZMA_HPP_
#define COMP_DECOMP_LZMA_HPP_

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

#ifndef COMP_DECOMP_BUFFSIZE
    #define COMP_DECOMP_BUFFSIZE (8 * 1024)
#endif

// bool lzma_compress(std::string& output, const void *input, size_t input_size, int rate = 9);
// bool lzma_decompress(std::string& output, const void *input, size_t input_size);
// bool lzma_unittest(void);

#ifdef HAVE_LZMA
    #include <lzma.h>

    inline bool lzma_compress(std::string& output, const void *input, size_t input_size, int rate = 9)
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
        return ret == LZMA_STREAM_END;
    }

    inline bool lzma_decompress(std::string& output, const void *input, size_t input_size)
    {
        const uint8_t *ptr = (const uint8_t *)input;
        size_t remainder = input_size;

        output.clear();
        output.reserve(input_size * 3 / 2);
        static uint8_t inbuf[COMP_DECOMP_BUFFSIZE];
        static uint8_t outbuf[COMP_DECOMP_BUFFSIZE];

        lzma_stream strm = LZMA_STREAM_INIT;
        lzma_ret ret = lzma_stream_decoder(&strm, UINT64_MAX, LZMA_IGNORE_CHECK);

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
        return (ret == LZMA_STREAM_END);
    }

    inline bool lzma_test_entry(const std::string& original)
    {
        std::string encoded, decoded;
        if (!lzma_compress(encoded, original.c_str(), original.size()))
        {
            printf("lzma_compress failed\n");
            return false;
        }
        if (!lzma_decompress(decoded, encoded.c_str(), encoded.size()))
        {
            printf("lzma_decompress failed\n");
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
            int len = std::rand() % COMP_DECOMP_MAX_TEST;
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
