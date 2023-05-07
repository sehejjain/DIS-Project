#define pieceSize 128000
#include <cstddef>
#include <stdio.h>
#include <math.h>
#include<vector>
#include<cstdint>
using namespace std;
struct file_header
{

    int32_t  position_in_file;
};

struct file_section
{
    struct file_header header;
    char *databuf;
    uint32_t size_of_databuf;
};

size_t findSizeOfFile(FILE *fp);
size_t copy_into_buffer(FILE *fp, char **bufp, int index);
void reconstruct_from_sections(vector<file_section> sections, int numberOfPieces);
