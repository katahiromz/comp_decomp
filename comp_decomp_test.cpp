// compress_test.hpp
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: MIT
#include <thread>
#include <chrono>
#include <ctime>
#define COMP_DECOMP_BUFFSIZE 100
#include "comp_decomp.hpp"

namespace cr = std::chrono;
typedef cr::high_resolution_clock my_clock;

inline void init_rand_gen(void)
{
    std::srand((unsigned)std::hash<std::thread::id>()(std::this_thread::get_id()));
}

bool g_flag = true;

#ifdef HAVE_ZLIB
void f1(void)
{
    init_rand_gen();
    printf("rand(): %d\n", std::rand());

    auto time1 = my_clock::now();
    bool ret = zlib_unittest();
    auto time2 = my_clock::now();
    auto diff = time2 - time1;
    auto ms = cr::duration_cast<cr::milliseconds>(diff);

    if (ret)
    {
        printf("zlib success (%ld ms)\n", (long)ms.count());
    }
    else
    {
        printf("zlib failed\n");
        g_flag = false;
    }

    fflush(stdout);
}
#endif

#ifdef HAVE_BZLIB
void f2(void)
{
    init_rand_gen();
    printf("rand(): %d\n", std::rand());

    auto time1 = my_clock::now();
    bool ret = bzlib_unittest();
    auto time2 = my_clock::now();
    auto diff = time2 - time1;
    auto ms = cr::duration_cast<cr::milliseconds>(diff);

    if (ret)
    {
        printf("bzlib success (%ld ms)\n", (long)ms.count());
    }
    else
    {
        printf("bzlib failed\n");
        g_flag = false;
    }

    fflush(stdout);
}
#endif

#ifdef HAVE_LZMA
void f3(void)
{
    init_rand_gen();
    printf("rand(): %d\n", std::rand());

    auto time1 = my_clock::now();
    bool ret = lzma_unittest();
    auto time2 = my_clock::now();
    auto diff = time2 - time1;
    auto ms = cr::duration_cast<cr::milliseconds>(diff);

    if (ret)
    {
        printf("lzma success (%ld ms)\n", (long)ms.count());
    }
    else
    {
        printf("lzma failed\n");
        g_flag = false;
    }

    fflush(stdout);
}
#endif

int main(void)
{
#ifdef HAVE_ZLIB
    std::thread t1(f1);
#endif
#ifdef HAVE_BZLIB
    std::thread t2(f2);
#endif
#ifdef HAVE_LZMA
    std::thread t3(f3);
#endif

#ifdef HAVE_ZLIB
    t1.join();
#endif
#ifdef HAVE_BZLIB
    t2.join();
#endif
#ifdef HAVE_LZMA
    t3.join();
#endif

    fflush(stdout);

    if (g_flag)
    {
        printf("done!\n");
        return 0;
    }
    else
    {
        printf("failed!\n");
        return 1;
    }
}
