#include <cassert>
#include <thread>
#include <chrono>
#define COMP_DECOMP_BUFFSIZE 100
#define COMP_DECOMP_COMPRESSION_RATE 9
#include "comp_decomp.hpp"

namespace cr = std::chrono;
typedef cr::high_resolution_clock my_clock;

#ifdef HAVE_ZLIB
void f1(void)
{
    auto time1 = my_clock::now();
    bool ret = zlib_test();
    auto time2 = my_clock::now();
    auto diff = time2 - time1;
    auto ms = cr::duration_cast<cr::milliseconds>(diff);
    if (ret)
        printf("zlib success (%ld ms)\n", (long)ms.count());
    else
        printf("zlib failed\n");
    fflush(stdout);
}
#endif

#ifdef HAVE_BZLIB
void f2(void)
{
    auto time1 = my_clock::now();
    bool ret = bzlib_test();
    auto time2 = my_clock::now();
    auto diff = time2 - time1;
    auto ms = cr::duration_cast<cr::milliseconds>(diff);
    if (ret)
        printf("bzlib success (%ld ms)\n", (long)ms.count());
    else
        printf("bzlib failed\n");
    fflush(stdout);
}
#endif

#ifdef HAVE_LZMA
void f3(void)
{
    auto time1 = my_clock::now();
    bool ret = lzma_test();
    auto time2 = my_clock::now();
    auto diff = time2 - time1;
    auto ms = cr::duration_cast<cr::milliseconds>(diff);
    if (ret)
        printf("lzma success (%ld ms)\n", (long)ms.count());
    else
        printf("lzma failed\n");
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

    printf("done!\n");
    fflush(stdout);
    return 0;
}
