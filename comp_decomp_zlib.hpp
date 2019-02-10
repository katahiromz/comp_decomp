#ifndef COMP_DECOMP_ZLIB_HPP_
#define COMP_DECOMP_ZLIB_HPP_

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

#ifdef HAVE_ZLIB
    #include <zlib.h>

    inline bool zlib_compress(std::string& output, const void *input, size_t input_size)
    {
        const Bytef *ptr = (const Bytef *)input;
        size_t remainder = input_size;

        output.clear();
        output.reserve(input_size * 2 / 3);
        static Bytef inbuf[COMP_DECOMP_BUFFSIZE];
        static Bytef outbuf[COMP_DECOMP_BUFFSIZE];

        z_stream strm;
        memset(&strm, 0, sizeof(strm));
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        int ret = deflateInit(&strm, COMP_DECOMP_COMPRESSION_RATE);

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

        return deflateEnd(&strm) == Z_OK;
    }

    inline bool zlib_decompress(std::string& output, const void *input, size_t input_size)
    {
        const Bytef *ptr = (const Bytef *)input;
        size_t remainder = input_size;

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

        return inflateEnd(&strm) == Z_OK;
    }

    inline bool zlib_test_entry(const std::string& original)
    {
        std::string encoded, decoded;
        if (!zlib_compress(encoded, original.c_str(), original.size()))
        {
            printf("zlib_compress failed\n");
            return false;
        }
        if (!zlib_decompress(decoded, encoded.c_str(), encoded.size()))
        {
            printf("zlib_decompress failed\n");
            return false;
        }
        if (!(original == decoded))
        {
            printf("zlib mismatch\n");
            return false;
        }
        return true;
    }

    inline bool zlib_test(void)
    {
        std::string original;
        if (!zlib_test_entry(original))
            return false;

        original.assign(COMP_DECOMP_MAX_TEST, 'A');
        if (!zlib_test_entry(original))
            return false;

        for (size_t i = 0; i < COMP_DECOMP_TEST_COUNT; ++i)
        {
            int len = std::rand() % COMP_DECOMP_MAX_TEST;
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
