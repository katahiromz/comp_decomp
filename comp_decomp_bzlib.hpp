#ifndef COMP_DECOMP_BZLIB_HPP_
#define COMP_DECOMP_BZLIB_HPP_

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

        int ret = BZ2_bzCompressInit(&strm, COMP_DECOM_COMPRESSION_RATE, 0, 0);
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

    inline bool bzlib_test(void)
    {
        std::string original, encoded, decoded;
        for (size_t len = COMP_DECOM_MIN_TEST;
             len < COMP_DECOM_MAX_TEST;
             len += COMP_DECOM_TEST_STEP)
        {
            original.resize(len);
            for (size_t k = 0; k < len; ++k)
            {
                original[k] = (char)(std::rand() & 0xFF);
            }
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
        }
        for (size_t i = 0; i < 20; ++i)
        {
            int len = COMP_DECOM_MIN_TEST + std::rand() % (COMP_DECOM_MAX_TEST - COMP_DECOM_MIN_TEST);
            original.resize(len);
            for (size_t k = 0; k < len; ++k)
            {
                original[k] = (char)(std::rand() & 0xFF);
            }
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
        }
        return true;
    }
#endif  // def HAVE_BZLIB

#endif  // ndef COMP_DECOMP_BZLIB_HPP_
