#ifndef COMP_DECOMP_BZLIB_HPP_
#define COMP_DECOMP_BZLIB_HPP_

#include <cstdlib>
#include <string>

#ifndef COMP_DECOMP_BUFFSIZE
    #define COMP_DECOMP_BUFFSIZE (8 * 1024)
#endif
#ifndef COMP_DECOMP_COMPRESSION_RATE
    #define COMP_DECOMP_COMPRESSION_RATE 9
#endif

#ifndef COMP_DECOMP_MAX_TEST
    #define COMP_DECOMP_MAX_TEST 100
#endif
#ifndef COMP_DECOMP_TEST_COUNT
    #define COMP_DECOMP_TEST_COUNT 100
#endif

#if !(1 <= COMP_DECOMP_COMPRESSION_RATE && COMP_DECOMP_COMPRESSION_RATE <= 9)
    #error compression rate is invalid.
#endif

#ifdef HAVE_BZLIB
    #include <bzlib.h>

    inline bool bzlib_compress(std::string& output, const void *input, size_t input_size)
    {
        output.clear();
        output.reserve(input_size * 2 / 3);
        static char inbuf[COMP_DECOMP_BUFFSIZE];

        bz_stream strm;
        memset(&strm, 0, sizeof(strm));
        strm.bzalloc = NULL;
        strm.bzfree = NULL;
        strm.opaque = NULL;
        strm.next_in = (char *)input;
        strm.avail_in = input_size;

        int ret = BZ2_bzCompressInit(&strm, COMP_DECOMP_COMPRESSION_RATE, 0, 0);
        if (ret != BZ_OK)
            return false;

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
            return false;
        }

        output.resize(output.size() - strm.avail_out);
        return BZ2_bzCompressEnd(&strm) == BZ_OK;
    }

    inline bool bzlib_decompress(std::string& output, const void *input, size_t input_size)
    {
        output.clear();
        output.reserve(input_size * 3 / 2);
        static uint8_t inbuf[COMP_DECOMP_BUFFSIZE];

        bz_stream strm;
        memset(&strm, 0, sizeof(strm));
        strm.bzalloc = Z_NULL;
        strm.bzfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = (char *)input;
        strm.avail_in = input_size;
        int ret = BZ2_bzDecompressInit(&strm, 0, 0);
        if (ret != BZ_OK)
            return false;

        output.resize(COMP_DECOMP_BUFFSIZE);
        strm.next_out  = (char *)(output.data());
        strm.avail_out = output.size();

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
            return false;
        }

        output.resize(output.size() - strm.avail_out);

        return BZ2_bzDecompressEnd(&strm) == Z_OK;
    }

    inline bool bzlib_test_entry(const std::string& original)
    {
        std::string encoded, decoded;
        if (!bzlib_compress(encoded, original.c_str(), original.size()))
        {
            printf("bzlib_compress failed\n");
            return false;
        }
        if (!bzlib_decompress(decoded, encoded.c_str(), encoded.size()))
        {
            printf("bzlib_decompress failed\n");
            return false;
        }
        if (!(original == decoded))
        {
            printf("bzlib mismatch\n");
            return false;
        }
        return true;
    }

    inline bool bzlib_test(void)
    {
        std::string original;
        if (!bzlib_test_entry(original))
            return false;

        original.assign(COMP_DECOMP_MAX_TEST, 'A');
        if (!bzlib_test_entry(original))
            return false;

        for (size_t i = 0; i < COMP_DECOMP_TEST_COUNT; ++i)
        {
            int len = std::rand() % COMP_DECOMP_MAX_TEST;
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
