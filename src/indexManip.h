#ifndef INDEX_MANIP_H
#define INDEX_MANIP_H

#include "basicStructs.h"

relationIndex initializeIndex(int , relation * , int );
indexArray * initializeIndexArray(int );
void freeIndexArray(indexArray * idxArray);

#endif