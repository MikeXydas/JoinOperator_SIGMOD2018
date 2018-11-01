#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "basicStructs.h"
#include "indexManip.h"
#include "hashing.h"

relation * getStartOfBucket(reorderedR * ror, uint32_t whichPsum) {  		//computes and returns the start index of a bucket

	relation * newRelation = (relation *) malloc(sizeof(relation));
	uint32_t index;
	index = ror->pSumArr.psum[whichPsum].offset; 								//startIndex of specific bucket
	tuple * tempTuple;
	tempTuple =	&(ror->rel->tuples[index]); 
	newRelation->tuples = tempTuple;
	if(whichPsum + 1 == ror->pSumArr.psumSize ) { 					//compute size of bucket if it is the last psum value

		newRelation->size = ror->rel->size - ror->pSumArr.psum[whichPsum].offset; 

	} else {

		newRelation->size = ror->pSumArr.psum[whichPsum + 1].offset - ror->pSumArr.psum[whichPsum].offset;
	}

	return newRelation;
}


void updateChain(uint32_t * chain, uint32_t * buckets, uint32_t hash2Index, uint32_t i) {

	uint32_t temp;
	temp = buckets[hash2Index];
	buckets[hash2Index] = i + 1;
	chain[i] = temp;

	return;
}



void buildIndex(relationIndex * oneIndex, uint32_t hash1, uint32_t hash2) {

	uint32_t chainSize = oneIndex->rel->size;							//chainSize is the same with the size of bucket

	uint32_t i, hash2Index;
	for (i = 0; i < chainSize; i++) {									//loop through elements of bucket

		hash2Index = hashing(oneIndex->rel->tuples[i].key, hash1, hash2);
		updateChain(oneIndex->chain, oneIndex->buckets, hash2Index, i);

	}

	return;

}


relation * getStartOfSubBucket(tuple *startOfBuck, uint32_t sizeIndexedSofar, uint32_t eachSize) {

	relation * newRelation = (relation *) malloc(sizeof(relation));
	newRelation->size = eachSize;
	tuple * tempTuple;
	tempTuple =	&(startOfBuck[sizeIndexedSofar]); 
	newRelation->tuples = tempTuple;

	return newRelation;
}


void buildSubIndex(relationIndex ** oneIndex, uint32_t hash1, uint32_t hash2, uint32_t sizeAll, uint32_t eachSize, uint32_t sizeIndexedSofar, tuple * startOfBuck, uint32_t i) {

	 
	uint32_t bucketSize = hash2Range(hash2);
	

	while(sizeAll > 0) {

		if( sizeAll < eachSize) {
			eachSize = sizeAll;
		}

		relation * rel = getStartOfSubBucket(startOfBuck, sizeIndexedSofar, eachSize);


		*oneIndex = (relationIndex *) malloc(sizeof(relationIndex));
		**oneIndex = initializeIndex(bucketSize, rel, i, NULL);
		buildIndex( *oneIndex, hash1, hash2);
		oneIndex = &((*oneIndex)->next);
		sizeIndexedSofar += eachSize;
		sizeAll -= eachSize;
	}

	return;
}
