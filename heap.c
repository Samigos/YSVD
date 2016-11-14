#include <stdio.h>
#include <string.h>
#include "BF.h"
#include "heap.h"

// ------------------------------------------------
// Create a new heap file, open it,
// allocate its first block and write
// to it the number 216, which will work
// as an ID, for heap files. After that,
// close the file.

int HP_CreateFile(char *fileName) {
    void *block;
    int fileDesc, blockNumber = BF_ID;

    BF_Init();
    if (BF_CreateFile(fileName) < 0) {
        BF_PrintError("Error creating file in HP_CreateFile");
        return -1;
    }
    
    if ((fileDesc = BF_OpenFile(fileName)) < 0) {
        BF_PrintError("Error opening file in HP_CreateFile");
        return -1;
    }
    
    if (BF_AllocateBlock(fileDesc) < 0) {
        BF_PrintError("Error allocating block in HP_CreateFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    if (BF_ReadBlock(fileDesc, 0, &block) < 0) {
        BF_PrintError("Error getting block in HP_CreateFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    memcpy(block, &blockNumber, sizeof(int));
    if (BF_WriteBlock(fileDesc, 0) < 0) {
        BF_PrintError("Error writing to block in HP_CreateFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    if (BF_CloseFile(fileDesc) < 0) {
        BF_PrintError("Error closing file in HP_CreateFile");
        return -1;
    }
    
    return 0;
}

// ------------------------------------------------
// Open the heap file with name fileName,
// get its first block, read its content
// and check if the read value, equals 216.

int HP_OpenFile(char *fileName) {
    void *block;
    int fileDesc, blockNumber;
    
    if ((fileDesc = BF_OpenFile(fileName)) < 0) {
        BF_PrintError("Error opening file in HP_OpenFile");
        return -1;
    }
    
    if (BF_ReadBlock(fileDesc, 0, &block) < 0) {
        BF_PrintError("Error getting block in HP_OpenFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
    memcpy(&blockNumber, block, sizeof(int));
    if (blockNumber != BF_ID) {
        printf("\n\nError: The file you opened, isn't a heap file!\n\n");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
        
    return fileDesc;
}

// ------------------------------------------------

int HP_CloseFile(int fileDesc) {
    if (BF_CloseFile(fileDesc) == 0) {
        return 0;
    }
    
    BF_PrintError("Error closing file in main");

    return -1;
}

// ------------------------------------------------
// At first, check if the current number of blocks,
// equals 1. If it does, create a new block,
// because we don't need the first one for storing
// entries. After that, update the blockNumber variable,
// that holds the current amount of blocks. Then,
// increase the static int recordsCounter, since a
// new entry is about to be stored in heap file.
// If recordsCounter, exceeds the maximum number of
// entries the current block can store, make the
// recordsCounter to equal 1 and create a new block.
// In the end, read the last created block and
// calculate the position, that the new entry will
// get in this block.

int HP_InsertEntry(int fileDesc, Record record) {
    void *block;
    int blockNumber;
    
    if ((blockNumber = BF_GetBlockCounter(fileDesc)) < 0) {
        BF_PrintError("Error getting block counter in HP_InsertEntry");
        return -1;
    }
    else if (blockNumber == 1) {
        if (BF_AllocateBlock(fileDesc) < 0) {
            BF_PrintError("Error allocating block in HP_InsertEntry");
            BF_CloseFile(fileDesc);
            
            return -1;
        }
        else blockNumber = 2;
    }
    
    // -------------------------------------
    
    ++recordsCounter;
    if (recordsCounter > (int)(BLOCK_SIZE / sizeof(record))) {
        recordsCounter = 1;
        
        if (BF_AllocateBlock(fileDesc) < 0) {
            BF_PrintError("Error allocating block in HP_InsertEntry");
            BF_CloseFile(fileDesc);
            
            return -1;
        }
        else ++blockNumber;
    }
    
    // -------------------------------------
    
    if (BF_ReadBlock(fileDesc, blockNumber - 1, &block) < 0) {
        BF_PrintError("Error getting block in HP_InsertEntry");
        return -1;
    }
    
    memcpy(block + ((recordsCounter - 1) * sizeof(record)), &record, sizeof(record));
    if (BF_WriteBlock(fileDesc, blockNumber - 1) < 0) {
        BF_PrintError("Error writing to block in HP_CreateFile");
        BF_CloseFile(fileDesc);
        
        return -1;
    }
    
	return 0;
}

// ------------------------------------------------
// At first, get the current number of blocks.
// After that, check the fieldName and act as following;
// For every entry, of every block, read the current block and
// pass the read data, to struct rec. Then, having in mind the
// fieldName, print the corresponding entries.

void HP_GetAllEntries(int fileDesc, char *fieldName, void *value) {
    void *block;
    int numOfBlocks;
    Record rec;
    
    // -------------------------------------
    
    if ((numOfBlocks = BF_GetBlockCounter(fileDesc)) < 0) {
        BF_PrintError("Error getting block counter in HP_GetAllEntries");
        return;
    }
    
    // -------------------------------------
    
    int b, r;
    if (strcmp(fieldName, "id") == 0) {
        int valueFound = FALSE;
        
        for (b = 1; b < numOfBlocks; b++) {
            if (BF_ReadBlock(fileDesc, b, &block) < 0) {
                BF_PrintError("Error getting block in HP_GetAllEntries");
                return;
            }
            
            for (r = 1; r <= (int)(BLOCK_SIZE / sizeof(Record)); r++) {
                memcpy(&rec, block + ((r - 1) * sizeof(Record)), sizeof(Record));
                
                if (rec.id == (int)value) {
                    printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                    printf("%d blocks were read\n\n------------------------\n", b);
                    
                    valueFound = TRUE;
                    
                    break;
                }
            }
            
            if (valueFound == TRUE)
                break;
        }
    }
    else {
        for (b = 1; b < numOfBlocks; b++) {
            if (BF_ReadBlock(fileDesc, b, &block) < 0) {
                BF_PrintError("Error getting block in HP_GetAllEntries");
                return;
            }
            
            for (r = 1; r <= (int)(BLOCK_SIZE / sizeof(Record)); r++) {
                memcpy(&rec, block + ((r - 1) * sizeof(Record)), sizeof(Record));
                
                if (strcmp(fieldName, "all") == 0 && rec.id > 0)
                    printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                else if (strcmp(fieldName, "name") == 0){
                    if (strcmp(rec.name, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
                else if (strcmp(fieldName, "surname") == 0) {
                    if (strcmp(rec.surname, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
                else if (strcmp(fieldName, "city") == 0) {
                    if (strcmp(rec.city, (char*)value) == 0)
                        printf("%d,\n%s,\n%s,\n%s\n\n", rec.id, rec.name, rec.surname, rec.city);
                }
            }
        }
        
        printf("%d blocks were read\n\n------------------------\n", numOfBlocks);
    }
}
