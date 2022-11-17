#include <stdio.h>
#include "tie_packer.h"

int main()
{
    TiePacker tpacker("test.tie");

    if(!tpacker.write("file1.txt", "this is file1.txt", 17))
        return 1;
    if(!tpacker.write("d1_1/d2_1/file1.txt", "this is d1_1/d2_1/file1.txt", 27))
        return 1;
    if(!tpacker.write("file2.txt", "this is file2.txt", 17))
        return 1;
    if(!tpacker.write("d1_1/d2_2/file1.txt", "this is d1_1/d2_2/file1.txt", 27))
        return 1;
    if(!tpacker.write("d1_1/d2_2/file2.txt", "this is d1_1/d2_2/file2.txt", 27))
        return 1;
    if(!tpacker.write("d1_2/d2_1/file1.txt", "this is d1_2/d2_1/file1.txt", 27))
        return 1;
    if(!tpacker.write("d1_2/file1.txt", "this is d1_2/file1.txt", 22))
        return 1;

    printf("done!\r\n");
    return 0;
}