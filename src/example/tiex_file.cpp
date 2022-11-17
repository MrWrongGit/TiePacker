#include "libtie/libtiex.h"

void printHelp()
{
    printf("tiex_file  [.tie file path]  [file path in .tie] [output file path]\r\n");
}

int main(int argc, char **argv)
{
    char buf[4096];
    uint16_t rsize;

    if(argc < 4) {
        printHelp();
        return -1;
    }

    // locate file positon in .tie file
    uint64_t seek = 0, size = 0;
    bool ret = LibTieX::fileLocate(argv[1], argv[2], seek, size);
    if(!ret) {
        printf("file locate fail\r\n");
        return -2;
    }

    // read file content
    FILE* ifp = fopen(argv[1], "rb");
    if (ifp == NULL)
        return false;
    ret = LibTieX::fileRecovery(ifp, argv[3], seek-sizeof(uint64_t));
    if(!ret) {
        printf("file locate fail\r\n");
        fclose(ifp);
        return -2;
    }
    fclose(ifp);
    
    printf("file recovery success\r\n");
    return 0;
}