#include "HT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int HT_CreateIndex(char *fileName, char attrType, char* attrName, int attrLength, int depth) {
	HT_info info;
	void *block;
	int i, *index, counter;

	if (BF_CreateFile(fileName) < 0)
		return -1;
	if ((info.fileDesc = BF_OpenFile(fileName)) < 0)
		return -1;
	info.attrLength = attrLength;
	info.attrName = attrName;
	info.attrType = attrType;
	info.depth = depth;

	//ftiakse block gia plirofories
	if (BF_AllocateBlock(info.fileDesc) < 0)
		return -1;
	if (BF_ReadBlock(info.fileDesc, 0, &block) < 0)
		return -1;
	memcpy(block, &info, sizeof(HT_info));
	if (BF_WriteBlock(info.fileDesc, 0) < 0)
		return -1;
	//ftiakse block euretiriou
	if (BF_AllocateBlock(info.fileDesc) < 0)
		return -1;
	//ftiakse upoloia blocks
	//stin arxi enas int me posses eggrafes,
	//meta int me deixti ston kado uperxeilisis,
	//mpenoun oi eggrafes
	for (i = 0; i < calc_depth(info.depth); i++) {
		if (BF_AllocateBlock(info.fileDesc) < 0)
			return -1;
		BF_ReadBlock(info.fileDesc, i + 2, &block);
		counter = 0;
		memcpy(block, &counter, sizeof(int));
		counter = -1;
		memcpy(block + sizeof(int), &counter, sizeof(int));
		counter = info.depth;
		memcpy(block + 2 * sizeof(int), &counter, sizeof(int));
		BF_WriteBlock(info.fileDesc, i + 2);
	}

	//kataskeui euretiriou
	if (BF_ReadBlock(info.fileDesc, 1, &block) < 0)
		return -1;
	index = malloc(calc_depth(info.depth) * sizeof(int));
	for (i = 0; i < 8; i++)
		index[i] = i + 2;
	memcpy(block, index, calc_depth(info.depth) * sizeof(int));
	if (BF_WriteBlock(info.fileDesc, 1) < 0)
		return -1;

	BF_CloseFile(info.fileDesc);
	free(index);
	return 0;
}

HT_info* HT_OpenIndex(char *fileName) {
	HT_info *info;
	void *block;

	info = malloc(sizeof(HT_info));

	BF_ReadBlock(BF_OpenFile(fileName), 0, &block);
	memcpy(info, block, sizeof(HT_info));
	return info;
}

int HT_CloseIndex(HT_info* header_info) {
	if (BF_CloseFile(header_info->fileDesc) < 0)
		return -1;
	free(header_info);
	return 0;
}

int HT_InsertEntry(HT_info* header_info, Record record) {
	unsigned long hash_key;
	void *block;
	int N, counter, local_d, block_num, *index, *index2, i, tmp_key;
	char string[20];

	N = calc_depth(header_info->depth);
	index = malloc(N * sizeof(int));
	sprintf(string, "%d", record.id);
	hash_key = hash(string);
	//printf("%d \n", hash_key);
	tmp_key = hash_key % N;

	//fortose ton pinaka
	BF_ReadBlock(header_info->fileDesc, 1, &block);
	memcpy(index, block, N * sizeof(int));
	//diavase to block
	BF_ReadBlock(header_info->fileDesc, index[tmp_key], &block);
	memcpy(&counter, block, sizeof(int));
	memcpy(&local_d, block + 2 * sizeof(int), sizeof(int));
	//printf("counter: %d \n", counter);

	if (counter < BLOCK_SIZE / sizeof(Record)) {
		memcpy(block + 3 * sizeof(int) + counter * sizeof(Record), &record, sizeof(Record));
		BF_WriteBlock(header_info->fileDesc, index[tmp_key]);
	} else if (counter == BLOCK_SIZE && local_d < header_info->depth) {
		printf("kati \n");
	} else if (counter == BLOCK_SIZE && local_d < header_info->depth) {
		//diplasiase pinaka, auksise topiko vathos
		//perna deiktes kainourgio pinaka
		index2 = malloc(2 * N * sizeof(int));
		local_d++;
		(header_info->depth)++;
		N = N * 2;
		for (i = 0; i < 2 * N; i++)
			if (i < N)
				index2[i] = index[i];
			else
				index2[i] = -1;

		tmp_key = hash_key % N;
		//an dn uparxei block diomiourgise to
		if (index2[tmp_key] == -1) {
			BF_AllocateBlock(header_info->fileDesc);
			BF_ReadBlock(header_info->fileDesc, BF_GetBlockCounter(header_info->fileDesc) - 1, &block);
			counter = 0;
			memcpy(block, &counter, sizeof(int));
			counter = 1;		//gia uperxeilisi
			memcpy(block + sizeof(int), &counter, sizeof(int));
			counter = header_info->depth;
			memcpy(block + 2 * sizeof(int), &counter, sizeof(int));
			BF_WriteBlock(header_info->fileDesc, BF_GetBlockCounter(header_info->fileDesc) - 1);
			index2[tmp_key] = BF_GetBlockCounter(header_info->fileDesc) - 1;
		}
		BF_ReadBlock(header_info->fileDesc, index2[tmp_key], &block);
		memcpy(block + 3 * sizeof(int), &record, sizeof(Record));
		BF_WriteBlock(header_info->fileDesc, index[tmp_key]);

		BF_ReadBlock(header_info->fileDesc, 1, &block);
		memcpy(block, index2, N*sizeof(int));
		BF_WriteBlock(header_info->fileDesc, 1);
	}

	return 0;
}

//http://www.cse.yorku.ca/~oz/hash.html
unsigned long hash(char *str) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;
	return hash;
}

int calc_depth(int n) {
	int i = 0;

	int result = 1;
	while (i < n) {
		result = result * 2;
		i++;
	}
	return result;
}
