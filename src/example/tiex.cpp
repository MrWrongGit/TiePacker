#include "libtie/libtiex.h"

void printHelp()
{
    printf("tiex  [.tie file path]  [output folder path]\r\n");
}

int main(int argc, char **argv)
{
    if(argc < 3) {
        printHelp();
        return -1;
    }
    bool ret = LibTieX::folderUnpack(argv[1], argv[2]);
    if(!ret) {
        printf("tie file unpack fail\r\n");
        return -2;
    }
    printf("tie file unpack success\r\n");
    return 0;
}