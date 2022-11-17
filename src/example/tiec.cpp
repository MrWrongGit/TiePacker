#include "libtie/libtiec.h"

void printHelp()
{
    printf("tiec  [folder path]  [output .tie file path]\r\n");
}

int main(int argc, char **argv)
{
    if(argc < 3) {
        printHelp();
        return -1;
    }
    bool ret = LibTieC::folderPack(argv[1], argv[2]);
    if(!ret) {
        printf("create tie file fail\r\n");
        return -2;
    }
    printf("create tie file success\r\n");
    return 0;
}