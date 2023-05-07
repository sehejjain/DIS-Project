#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <map>
#include <algorithm>
#include "fileutils.h"


using namespace std;

size_t findSizeOfFile(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return sz;
}

size_t copy_into_buffer(FILE *fp, char **bufp, int index)
{
    *bufp = (char *)malloc(pieceSize);
    int position = index * pieceSize;
    // fseek(fp, SEEK_SET, index*pieceSize+1);
    size_t readsz = fread(*bufp, 1, pieceSize, fp);

    return readsz;
}


void reconstruct_from_sections(vector<file_section> sections, int numberOfPieces)
{
    FILE *fp = fopen("test.jpg", "wb");
    for (int i = 0; i < numberOfPieces; i++)
    {
        fwrite(sections[i].databuf, 1, sections[i].size_of_databuf, fp);
    }
}

