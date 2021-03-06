#include <stdio.h>
#include <math.h>
#include "sideFunctions.h"
#include "hashing.h"
#include "indexManip.h"

indexArray * indexing(reorderedR * ror, myint_t hash1) {

	myint_t sizeOfIndexArray = hash1Range(hash1);
	indexArray * mainIndexArray = initializeIndexArray(sizeOfIndexArray);

	myint_t range = (CACHE_SIZE - AVAILABLE_CACHE_SIZE) / sizeof(myint_t);		//computing hash2 range so that the bucketArray in indexing occupies
	myint_t hash2 = (myint_t) log2(range);										//no more than CACHE_SIZE - AVAILABLE_CACHE_SIZE
	myint_t hash2Var;

	myint_t i, whichPsum = 0;			//whichPsum stores index to PsumArray , increases every time an indexArray entry is constructed

	for( i = 0; i < sizeOfIndexArray; i++) {

		if( whichPsum >= (ror->pSumArr).psumSize) { 				//no index is created if we have examined the whole psumArray		

			mainIndexArray->indexes[i] = initializeIndex(0, NULL, i, NULL, 0);
			continue;
		}

		hash2Var = hash2;

		if(i == (ror->pSumArr).psum[whichPsum].h1Res ){ 			//index is created when there is a bucket of i's

			relation * rel = getStartOfBucket(ror, whichPsum);
			myint_t bucketArrSize = hash2Range(hash2Var);
			myint_t sizeOfIndex = sizeof(myint_t) + rel->size * (sizeof(tuple) + sizeof(myint_t)) + bucketArrSize * sizeof(myint_t); //size + size of a bucket + size of chain + size of bucketIndexArray
			
			if((double) sizeOfIndex / (double)CACHE_SIZE <= 1 ) {				//if indexStruct fits in cache

				mainIndexArray->indexes[i] = initializeIndex(bucketArrSize, rel, i, NULL, hash2Var);

				buildIndex( &(mainIndexArray->indexes[i]), hash1, hash2Var);  

			} else {												//indexStruct doesn't fit in cache

				myint_t sizeAll = rel->size;					//size of the whole bucket
				myint_t eachSize;
				
				recomputeSizes(&eachSize, &hash2Var, sizeAll);  //re-compute size of each subBucket and hash2 range so as to fit in cache
				bucketArrSize = hash2Range(hash2Var);
				myint_t sizeIndexedSoFar = 0;

				tuple * startOfBuck = rel->tuples;				//store start of initial bucket
				rel->size =  eachSize;

				mainIndexArray->indexes[i] = initializeIndex(bucketArrSize, rel, i, NULL, hash2Var);

				buildIndex(&(mainIndexArray->indexes[i]), hash1, hash2Var);

				sizeAll -= eachSize;
				sizeIndexedSoFar += eachSize;
				
				buildSubIndex(&(mainIndexArray->indexes[i].next), hash1, hash2Var, sizeAll, eachSize, sizeIndexedSoFar, startOfBuck, i);

				
			}
			whichPsum++;
			
		}else { 												//otherwise no index is created for bucket of i's 
			mainIndexArray->indexes[i] = initializeIndex(0, NULL, i, NULL, 0);
		}
	}

	return mainIndexArray;

}