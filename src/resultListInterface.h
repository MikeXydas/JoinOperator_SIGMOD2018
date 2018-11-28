#ifndef RESULT_LIST_INTERFACE_H
#define RESULT_LIST_INTERFACE_H

#include "basicStructs.h"
#include "queryStructs.h"

#define MB 1048576

headResult * initialiseResultHead();
resultNode * initialiseResultNode();
void pushResult(headResult * head, rowTuple * t);
void freeResultList(headResult * head);
void freeResultNode(resultNode * node);
int checkResults(headResult * head);
myint_t countSizeOfList(headResult * head);

#endif