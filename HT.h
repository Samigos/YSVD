#include "BF.h"

typedef struct {
	int id;
	char name[15];
	char surname[20];
	char status[1];
	char dateOfBirth[11];
	int salary;
	char section[1];
	int daysOff;
	int prevYears;
} Record;

typedef struct {
	int fileDesc;
	char attrType;
	char* attrName;
	int attrLength;
	int depth;
} HT_info;

int HT_CreateIndex(char *fileName, char attrType, char* attrName, int attrLength, int depth);
HT_info* HT_OpenIndex(char *fileName);
int HT_CloseIndex(HT_info* header_info);
int calc_depth(int n);
int HT_InsertEntry (HT_info* header_info, Record record);

unsigned long hash(char *str);
