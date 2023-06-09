#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
using namespace std;

#define POOLSIZE 62;
void generateUniqueID(char *ID, int size)
{

    srand(time(NULL));
    char pool[62];
    int index = 0;

    for (int i = 0; i < 26; i++)
    {
        pool[index++] = i + 'A';
    }
    
    for (int i = 0; i < 9; i++)
    {
        pool[index++] = i + '0';
    }

    for (int i = 0; i < 26; i++)
    {
        pool[index++] = i + 'a';
    }

    for (int i = 0; i < size; i++)
    {

        int poolIndex = rand() % (61);
        ID[i] = pool[poolIndex];
    }

    ID[size - 1] = '\0';
    // temp
    cout<<"Generated ID: "<<ID<<endl;
}