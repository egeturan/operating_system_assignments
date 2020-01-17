#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "simplefs.h"
#include <string.h>

typedef struct attributes Attributes;
typedef struct directoryEntry DirectoryEntry;
typedef struct fatEntry FatEntry;
typedef struct superBlock SuperBlock;
typedef struct directoryEntryBlock DirectoryEntryBlock;
typedef struct fatBlock FatBlock;
typedef struct fileBlock FileBlock;
typedef struct emptyFileBlock EmptyFileBlock;
typedef struct openFileTable OpenFileTable;
typedef struct fatTable FatTable;

struct attributes
{
    char mode[2];
    char size[8];
    char firstblock[8];
    char isValid[2];
    char openByteOffset[8];
    char dirEntryBlockNum[2];
    char dirEntryNum[3];
    char restOfAttributes[63];
};
struct directoryEntry
{
    char fileName[32];
    Attributes attributes;
};
struct fatEntry
{
    char nextBlock[8];
};
struct superBlock
{
    char file[2];
    char fileSize[2];
    char fat[2];
    char fatSize[5];
    char root[5];
    char nextFree[8];
    char dirEntryCount[3];
    char restOfAttributes[BLOCKSIZE - 27];
};
struct directoryEntryBlock
{
    DirectoryEntry entries[8];
};
struct fatBlock
{
    FatEntry entries[128];
};
struct fileBlock
{
    char data[BLOCKSIZE];
};
struct emptyFileBlock
{
    char nextEmptyBlock[8];
    char restOfData[BLOCKSIZE - 8];
};
struct openFileTable
{
    DirectoryEntry entries[10];
};
struct fatTable
{
    FatEntry entries[1024 * 128];
};


int vdisk_fd; // global virtual disk file descriptor
              // will be assigned with the sfs_mount call
              // any function in this file can use this.
OpenFileTable* openFiles;
FatTable* fatTable;

// This function is simply used to a create a virtual disk
// (a simple Linux file including all zeros) of the specified size.
// You can call this function from an app to create a virtual disk.
// There are other ways of creating a virtual disk (a Linux file)
// of certain size. 
// size = 2^m Bytes
int create_vdisk (char *vdiskname, int m)
{
    char command[BLOCKSIZE]; 
    int size;
    int num = 1;
    int count; 
    size  = num << m;
    count = size / BLOCKSIZE;
    printf ("%d %d\n", m, size);
    sprintf (command, "dd if=/dev/zero of=%s bs=%d count=%d\n",
	     vdiskname, BLOCKSIZE, count);
    printf ("executing command = %s\n", command); 
    system (command); 
    return (0); 
}



// read block k from disk (virtual disk) into buffer block.
// size of the block is BLOCKSIZE.
// space for block must be allocated outside of this function.
// block numbers start from 0 in the virtual disk. 
int read_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vdisk_fd, (off_t) offset, SEEK_SET);
    n = read (vdisk_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
	printf ("read error\n");
	return -1;
    }
    return (0); 
}

// write block k into the virtual disk. 
int write_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vdisk_fd, (off_t) offset, SEEK_SET);
    n = write (vdisk_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
	printf ("write error\n");
	return (-1);
    }
    return 0; 
}


/**********************************************************************
   The following functions are to be called by applications directly. 
***********************************************************************/

int sfs_format (char *vdiskname)
{
    //printf("HELLO FAT ENTRY SIZE: %d\n", sizeof(FatEntry));
    //printf("HELLO DIRECTORY ENTRY SIZE: %d\n",sizeof(DirectoryEntry));
    //printf("HELLO SUPERBLOCK SIZE: %d\n",sizeof(SuperBlock));
    //printf("HELLO DIRECTORYENTRYBLOCK SIZE: %d\n",sizeof(DirectoryEntryBlock));
    //printf("HELLO FATBLOCK SIZE: %d\n",sizeof(FatBlock));
    //printf("HELLO FATENTRY SIZE: %d\n",sizeof(FatEntry));
    //printf("HELLO FATTABLE SIZE: %d\n",sizeof(FatTable));
    vdisk_fd = open(vdiskname, O_RDWR);
    SuperBlock* super = malloc(sizeof(SuperBlock));
    strcpy(super->file, "1");
    strcpy(super->fileSize, "7");
    strcpy(super->fat, "8");
    strcpy(super->fatSize, "1024");
    strcpy(super->root, "1032");
    strcpy(super->nextFree, "0");
    strcpy(super->dirEntryCount, "0");
    write_block(super, 0);
    SuperBlock* buffer = malloc(sizeof(SuperBlock));
    read_block(buffer, 0);
    //printf("Superblock->nextFree = %s\n",buffer->nextFree);
    //printf("OPEN FILE TABLE = %d\n", sizeof(DirectoryEntry) * 10);
    //printf("FAT TABLE = %d\n",sizeof(FatEntry) * ((BLOCKSIZE / sizeof(FatEntry)) * 1024));
    int i;
    DirectoryEntryBlock* tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
    for(i = 0; i < 8; i++)
    {
        strcpy(tempDirBlock->entries[i].attributes.isValid, "0");
    }
    for(i = 0; i < 7; i++)
    {
        write_block(tempDirBlock, atoi(buffer->file) + i);
    }
    FatBlock* tempFatBlock = malloc(sizeof(FatBlock));
    for(i = 0; i < 128; i++)
    {
        strcpy(tempFatBlock->entries[i].nextBlock, "-1");
    }
    for(i = 0; i < 1024; i++)
    {
        write_block(tempFatBlock, atoi(buffer->fat) + i);
    }
    EmptyFileBlock* tempFileBlock = malloc(sizeof(EmptyFileBlock));
    struct stat st;
    stat(vdiskname, &st);
    int sizeOfDisk = st.st_size;
    char tempNext[8];
    for(i = 0; i < (sizeOfDisk / BLOCKSIZE) - 1033; i++)
    {
        sprintf(tempNext, "%d", i + 1);
        //printf("Next Empty Block = %s\n", tempNext);
        strcpy(tempFileBlock->nextEmptyBlock, tempNext);
        write_block(tempFileBlock, atoi(buffer->root) + i);
    }
    //printf("Next Empty Block = %s\n", "-1");
    strcpy(tempFileBlock->nextEmptyBlock, "-1");
    write_block(tempFileBlock, atoi(buffer->root) + tempNext);
    free(super);
    free(buffer);
    free(tempDirBlock);
    free(tempFatBlock);
    free(tempFileBlock);
    fsync (vdisk_fd); 
    close (vdisk_fd);
    return (0); 
}

int sfs_mount (char *vdiskname)
{
    // simply open the Linux file vdiskname and in this
    // way make it ready to be used for other operations.
    // vdisk_fd is global; hence other function can use it. 
    vdisk_fd = open(vdiskname, O_RDWR);
    openFiles = malloc(sizeof(DirectoryEntry) * 10);
    fatTable = malloc(sizeof(FatEntry) * ((BLOCKSIZE / sizeof(FatEntry)) * 1024));
    SuperBlock* tempSuper = malloc(sizeof(SuperBlock));
    read_block(tempSuper, 0);
    int i;
    int k;
    DirectoryEntry* tempDirEntry = malloc(sizeof(DirectoryEntry));
    strcpy(tempDirEntry->attributes.isValid, "0");
    for(i = 0; i < 10; i++)
    {
        cpyDirEntry(&(openFiles->entries[i]), tempDirEntry);
        //printf("Open File Entry is Valid = %d\n", atoi(openFiles->entries[i].attributes.isValid));
    }
    FatBlock* tempFatBlock = malloc(sizeof(FatBlock));
    for(i = 0; i < 1024; i++)
    {
        read_block(tempFatBlock, atoi(tempSuper->fat) + i);
        for(k = 0; k < 128; k++)
        {
            strcpy(fatTable->entries[k + (i * 128)].nextBlock, tempFatBlock->entries[k].nextBlock);
            //printf("Fat Table Entry No %d Next Block = %s\n", k + (i * 128),fatTable->entries[k + (i * 128)].nextBlock);
        }
    }
    free(tempDirEntry);
    free(tempFatBlock);
    return(0);
}

int sfs_umount ()
{
    if(vdisk_fd == 0)
    {
        printf("There is no disk mounted.\n");
        return(-1);
    }
    free(openFiles);
    free(fatTable);
    fsync (vdisk_fd); 
    close (vdisk_fd);
    vdisk_fd = 0;
    return (0); 
}


int sfs_create(char *filename)
{
    if(vdisk_fd == 0)
    {
        printf("There is no disk mounted.\n");
        return(-1);
    }
    //printf("Is OFT NULL = %d\n",vdisk_fd);
    //does file name exist
    if(fileNameController(filename) == 1){

    int len = strlen(filename);
    //printf("Size of file is: %d\n", len);
    SuperBlock* tempSuper = malloc(sizeof(SuperBlock));
    DirectoryEntryBlock* tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
    DirectoryEntry* tempDirEntry = malloc(sizeof(DirectoryEntry));
    read_block(tempSuper, 0);
    //printf("Dir entry count is: %s\n", tempSuper->dirEntryCount);
    if(len <= 32){
        if(atoi(tempSuper->dirEntryCount) < 56)
        {
            //it is valid size
            int i;
            int k;
            for(i = 0; i < 7; i++)
            {
                read_block(tempDirBlock, atoi(tempSuper->file) + i);
                for(k = 0; k < 8; k++)
                {
                    //printf("Is valid = %d\n", atoi(tempDirBlock->entries[i].attributes.isValid) == 0);
                    if(atoi(tempDirBlock->entries[k].attributes.isValid) == 0)
                    {
                        strcpy(tempDirEntry->fileName, filename);
                        strcpy(tempDirEntry->attributes.size, "0");
                        strcpy(tempDirEntry->attributes.isValid, "1");
                        strcpy(tempDirEntry->attributes.mode, "0");
                        strcpy(tempDirEntry->attributes.openByteOffset, "0");
                        char tempNum[3];
                        sprintf(tempNum, "%d", i);
                        strcpy(tempDirEntry->attributes.dirEntryBlockNum, tempNum);
                        sprintf(tempNum, "%d", (i * 8) + k);
                        strcpy(tempDirEntry->attributes.dirEntryNum, tempNum);
                        strcpy(tempDirEntry->attributes.firstblock, tempSuper->nextFree);
                        cpyDirEntry(&(tempDirBlock->entries[k]), tempDirEntry);
                        //printf("Directory block %d and directory entry %d\n", i , k);
                        //printf("%s -- -- -- %s\n", tempDirBlock->entries[k].fileName, tempDirEntry->fileName);
                        write_block(tempDirBlock, atoi(tempSuper->file) + i);
                        /*
                        free(tempDirBlock);
                        tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
                        read_block(tempDirBlock, i);
                        printf("File name is: %s\n", tempDirBlock->entries[k].fileName);
                        */
                        EmptyFileBlock* tempEmptyBlock = malloc(sizeof(EmptyFileBlock));
                        read_block(tempEmptyBlock, atoi(tempSuper->root) + atoi(tempSuper->nextFree));
                        FileBlock* tempFileBlock = malloc(sizeof(FileBlock));
                        write_block(tempFileBlock, atoi(tempSuper->root) + atoi(tempSuper->nextFree));
                        strcpy(tempSuper->nextFree, tempEmptyBlock->nextEmptyBlock);
                        char tempCount[3];
                        sprintf(tempCount, "%d", atoi(tempSuper->dirEntryCount) + 1);
                        strcpy(tempSuper->dirEntryCount, tempCount);
                        write_block(tempSuper, 0);
                        free(tempEmptyBlock);
                        free(tempFileBlock);
                        free(tempDirBlock);
                        free(tempDirEntry);
                        free(tempSuper);
                        return (0);
                    }
                }
            }

        }
        else
        {
            //There are too many files in disk
            printf("There are too many files in disk\n");
            free(tempDirBlock);
            free(tempDirEntry);
            free(tempSuper);
            return(-1);
        }
        
    }else{
        //filename size is not valid
        printf("File name is not valid\n");
        free(tempDirBlock);
        free(tempDirEntry);
        free(tempSuper);
        return(-1);
    }
    free(tempDirBlock);
    free(tempDirEntry);
    free(tempSuper);
    return (0);
    }
    else 
    {
    printf("File name exist in Directory Entries\n");
    return (-1);
    }
   
}


int sfs_open(char *file, int mode)
{
    if(vdisk_fd == 0)
    {
        printf("There is no disk mounted.\n");
        return(-1);
    }

    int isExist = 0;
    int i;
    for (i = 0; i < 10; i++)
    {
        //printf("Name of the open files: %s\n", openFiles->entries[i].fileName);
        if(strcmp(openFiles->entries[i].fileName, file) == 0 && strcmp(openFiles->entries[i].attributes.isValid, "1") == 0){
            //printf("------Filename already exist for %s-----\n", file);
            isExist = 1;
            break;
        }
    }
    
    if(isExist == 1){
        printf("File name already exist\n");
        return(-1);
    }else{
        SuperBlock* tempSuper = malloc(sizeof(SuperBlock));
        DirectoryEntryBlock* tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
        
        read_block(tempSuper, 0);

        //printf("Entry count is: %d\n", atoi(tempSuper->dirEntryCount));

        int i;
        int k;
        int l;
        for(i = 0; i < 7; i++)
        {
            read_block(tempDirBlock, atoi(tempSuper->file) + i);
            for(k = 0; k < 8; k++)
                {
                    if(strcmp(tempDirBlock->entries[k].fileName, file) == 0 && strcmp(tempDirBlock->entries[k].attributes.isValid, "1") == 0){
                        for(l = 0; l < 10; l++)
                        {
                            if(atoi(openFiles->entries[l].attributes.isValid) == 0)
                            {
                                cpyDirEntry(&(openFiles->entries[l]), &(tempDirBlock->entries[k]));
                                char tempMode[2];
                                sprintf(tempMode, "%d", mode);
                                strcpy(openFiles->entries[l].attributes.mode, tempMode);
                                strcpy(openFiles->entries[l].attributes.openByteOffset, "0");
                                free(tempSuper);
                                free(tempDirBlock);
                                //printf("Opened File = %s\n",openFiles->entries[l].fileName);
                                //printOpenFileTable();
                                return(l);
                            }
                        }
                        free(tempSuper);
                        free(tempDirBlock);
                        printf("You cannot open more than 10 files in a process.\n");
                        return(-1);
                    }
                }
        }
        free(tempSuper);
        free(tempDirBlock);
        printf("File name is not exist in the directory entry blocks\n");
        return(-1);
    }
    return (0);
}

int sfs_close(int fd){
    if(vdisk_fd == 0)
    {
        printf("There is no disk mounted.\n");
        return(-1);
    }
    if(fd < 0 || fd > 9)
    {
        printf("File descriptor is invalid\n");
        return(-1);
    }
    if(atoi(openFiles->entries[fd].attributes.isValid) == 0)
    {
        printf("This file is already closed\n");
        return(-1);
    }
    strcpy(openFiles->entries[fd].attributes.isValid, "0");
    return (0); 
}

int sfs_getsize (int  fd)
{
    if(vdisk_fd == 0)
    {
        printf("There is no disk mounted.\n");
        return(-1);
    }
    if(fd < 10 || fd >= 0)
    {
        if(atoi(openFiles->entries[fd].attributes.isValid) == 1)
        {
            //printf("File is found in open file table\n");
            return atoi(openFiles->entries[fd].attributes.size);
        }
        else
        {
            printf("File is invalid \n");
            return(-1);
        }
    }
    else
    {
        printf("This file is not in the open file table\n");
        return (-1);
    }
    return (0); 
}

int sfs_read(int fd, void *buf, int n){
    int byteCounter = 0;
    if(vdisk_fd == 0)
    {
        printf("There is no disk mounted.\n");
        return(-1);
    }
  
    if(fd >= 0 || fd < 10)
    {
        if((strcmp(openFiles->entries[fd].attributes.isValid, "1") == 0))
        {
            //printf("File is valid\n");
            if(atoi(openFiles->entries[fd].attributes.mode) == MODE_READ)
            {
                if((atoi(openFiles->entries[fd].attributes.size) - (atoi(openFiles->entries[fd].attributes.openByteOffset) + n)) >= 0){
                    //printf("We have place to read\n");
                    int q1 = (atoi(openFiles->entries[fd].attributes.openByteOffset) / BLOCKSIZE); //which block do we stay 
                    int r1 = (atoi(openFiles->entries[fd].attributes.openByteOffset) % BLOCKSIZE); //which byte of that block
                    int q2 = (atoi(openFiles->entries[fd].attributes.openByteOffset) + n) / BLOCKSIZE; //which block do we go
                    int r2 = (atoi(openFiles->entries[fd].attributes.openByteOffset) + n) % BLOCKSIZE; //which byte do we go in that block
                    //printf("OpenOffset %d - q1:%d - r1:%d - q2: %d - r2:%d\n",atoi(openFiles->entries[fd].attributes.openByteOffset), q1, r1, q2, r2);
                    SuperBlock* tempSuper = malloc(sizeof(SuperBlock));
                    read_block(tempSuper, 0);
                    FileBlock* tempFileBlock = malloc(sizeof(FileBlock));
                    read_block(tempFileBlock, atoi(tempSuper->root));
                    FatBlock* tempFatBlock = malloc(sizeof(FatBlock));
                    int bytes = 0;
                    
                    char values[n];
                    int currentFatBlock = atoi(openFiles->entries[fd].attributes.firstblock);
                    int i;
                    for (i = 0; i <= q2; i++)
                    {
                        read_block(tempFileBlock, atoi(tempSuper->root) + currentFatBlock);
                        if(i >= q1 || i <= q2)
                        { //these numbers are our block counts that we will include
                            //printf("i is: %d", i);
                            if(q1 == q2)
                            {
                                //printf("q1 and q2 equal\n");
                                int k;
                                for (k = r1; k < r2; k++)
                                {
                                    values[bytes] = tempFileBlock->data[k];
                                    byteCounter++;
                                    bytes++;
                                }
                                break;
                            }
                            else if(i == q1)
                            {
                                //printf("i == q1 case works");
                                int k;
                                for (k = r1; k <= BLOCKSIZE - 1; k++)
                                {
                                    values[bytes] = tempFileBlock->data[k];
                                    byteCounter++;
                                    bytes++;
                                }
                            }
                            else if(i > q1 && i != q2)
                            {
                                
                                int k;
                                for (k = 0; k <= BLOCKSIZE - 1; k++)
                                {
                                    values[bytes] = tempFileBlock->data[k];
                                    byteCounter++;
                                    bytes++;
                                }
                            }
                            else if(i == q2)
                            {
                                int k;
                                for (k = 0; k < r2; k++)
                                {
                                    values[bytes] = tempFileBlock->data[k];
                                    byteCounter++;
                                    bytes++;
                                }
                            }
                        }
                        currentFatBlock = atoi(fatTable->entries[currentFatBlock].nextBlock);
                    }

                    int l;
                    for (l = 0; l < n; l++)
                    {
                        ((char *) buf)[l] = values[l];
                    }


                    

                    //give new place of the openByteOffset
                    int newOpenByteOffset = atoi(openFiles->entries[fd].attributes.openByteOffset) + n;
                    char newPlace[8];
                    sprintf(newPlace, "%d", newOpenByteOffset);
                    strcpy(openFiles->entries[fd].attributes.openByteOffset, newPlace);
                    free(tempSuper);
                    free(tempFileBlock);
                    free(tempFatBlock);
                    return (byteCounter);
                }
                else
                {
                    printf("Reading request size is more than stayed read area\n");
                    return (-1);
                }
                
            }
            else
            {
                printf("File is not in read mode\n");
                return (-1);
            }
        }
        else
        {
            printf("File is invalid\n");
            return (-1);
        }
    }
    else 
    {
        printf("File descriptor is not valid\n");
        return(-1);
    }
    return (0); 
}

int sfs_append(int fd, void *buf, int n)
{
    int byteCounter = 0;
    if(vdisk_fd == 0)
    {
        printf("There is no disk mounted.\n");
        return(-1);
    }
    if(fd < 0 || fd > 9)
    {
        printf("File descriptor is invalid\n");
        return(-1);
    }
    if(atoi(openFiles->entries[fd].attributes.isValid) == 0)
    {
        printf("File is invalid\n");
        return(-1);
    }
    if(atoi(openFiles->entries[fd].attributes.mode) != MODE_APPEND)
    {
        printf("File is on wrong mode\n");
        return(-1);
    }
    SuperBlock* tempSuper = malloc(sizeof(SuperBlock));
    read_block(tempSuper, 0);
    int continueFlag;
    int tempBlockNum = atoi(openFiles->entries[fd].attributes.firstblock);
    int i;
    if((atoi(openFiles->entries[fd].attributes.size) + n) / BLOCKSIZE == atoi(openFiles->entries[fd].attributes.size) / BLOCKSIZE)
    {
        continueFlag = 1;
        while(continueFlag)
        {
            if(atoi(fatTable->entries[tempBlockNum].nextBlock) == -1)
            {
                continueFlag = 0;
            }
            else
            {
                tempBlockNum = atoi(fatTable->entries[tempBlockNum].nextBlock);
            }
        }
        FileBlock* tempFileBlock = malloc(sizeof(FileBlock));
        read_block(tempFileBlock, atoi(tempSuper->root) + tempBlockNum);
        //printf("File size = %d\n", atoi(openFiles->entries[fd].attributes.size));
        for(i = 0; i < n; i++)
        {
            tempFileBlock->data[(atoi(openFiles->entries[fd].attributes.size) % BLOCKSIZE) + i] = ((char*)buf)[i];
            byteCounter++;
            //printf("%c\n", tempFileBlock->data[(atoi(openFiles->entries[fd].attributes.size) % BLOCKSIZE) + i]);
        }
        write_block(tempFileBlock, atoi(tempSuper->root) + tempBlockNum);
        char tempSize[8];
        sprintf(tempSize, "%d", atoi(openFiles->entries[fd].attributes.size) + n);
        strcpy(openFiles->entries[fd].attributes.size, tempSize);
        DirectoryEntryBlock* tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
        read_block(tempDirBlock, atoi(tempSuper->file) + atoi(openFiles->entries[fd].attributes.dirEntryBlockNum));
        cpyDirEntry(&(tempDirBlock->entries[atoi(openFiles->entries[fd].attributes.dirEntryNum) % 8]), &(openFiles->entries[fd]));
        write_block(tempDirBlock, atoi(tempSuper->file) + atoi(openFiles->entries[fd].attributes.dirEntryBlockNum));
        free(tempSuper);
        free(tempFileBlock);
        free(tempDirBlock);
        return(byteCounter);
    }
    else
    {
        continueFlag = 1;
        while(continueFlag)
        {
            if(atoi(fatTable->entries[tempBlockNum].nextBlock) == -1)
            {
                continueFlag = 0;
            }
            else
            {
                tempBlockNum = atoi(fatTable->entries[tempBlockNum].nextBlock);
            }
        }
        int offsetCount = 0;
        FileBlock* tempFileBlock = malloc(sizeof(FileBlock));
        read_block(tempFileBlock, atoi(tempSuper->root) + tempBlockNum);
        //printf("File size = %d\n", atoi(openFiles->entries[fd].attributes.size));
        for(i = 0; i < (BLOCKSIZE - (atoi(openFiles->entries[fd].attributes.size) % BLOCKSIZE)); i++)
        {
            tempFileBlock->data[(atoi(openFiles->entries[fd].attributes.size) % BLOCKSIZE) + i] = ((char*)buf)[i];
            byteCounter++;
            offsetCount++;
        }
        //printf("OffsetCount = %d\n", offsetCount);
        write_block(tempFileBlock, atoi(tempSuper->root) + tempBlockNum);
        EmptyFileBlock* tempEmptyFileBlock = malloc(sizeof(EmptyFileBlock));
        FatBlock* tempFatBlock = malloc(sizeof(FatBlock));
        while(offsetCount - 1 < n)
        {
            strcpy(fatTable->entries[tempBlockNum].nextBlock, tempSuper->nextFree);
            read_block(tempFatBlock, atoi(tempSuper->fat) + (tempBlockNum / 128));
            strcpy(tempFatBlock->entries[tempBlockNum % 128].nextBlock, tempSuper->nextFree);
            write_block(tempFatBlock, atoi(tempSuper->fat) + (tempBlockNum / 128));
            tempBlockNum = atoi(tempSuper->nextFree);
            read_block(tempEmptyFileBlock, atoi(tempSuper->root) + atoi(tempSuper->nextFree));
            strcpy(tempSuper->nextFree, tempEmptyFileBlock->nextEmptyBlock);
            read_block(tempFileBlock, atoi(tempSuper->root) + tempBlockNum);
            if((n - offsetCount) < BLOCKSIZE)
            {
                for(i = 0; i < n - offsetCount; i++)
                {
                    tempFileBlock->data[i] = ((char*)buf)[i + offsetCount];
                    byteCounter++;
                    offsetCount++;
                }
                write_block(tempFileBlock, atoi(tempSuper->root) + tempBlockNum);
                char tempSize[8];
                sprintf(tempSize, "%d", atoi(openFiles->entries[fd].attributes.size) + n);
                strcpy(openFiles->entries[fd].attributes.size, tempSize);
                DirectoryEntryBlock* tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
                read_block(tempDirBlock, atoi(tempSuper->file) + atoi(openFiles->entries[fd].attributes.dirEntryBlockNum));
                cpyDirEntry(&(tempDirBlock->entries[atoi(openFiles->entries[fd].attributes.dirEntryNum) % 8]), &(openFiles->entries[fd]));
                write_block(tempDirBlock, atoi(tempSuper->file) + atoi(openFiles->entries[fd].attributes.dirEntryBlockNum));
                write_block(tempSuper, 0);
                free(tempSuper);
                free(tempFileBlock);
                free(tempEmptyFileBlock);
                free(tempFatBlock);
                return(byteCounter);
            }
            else
            {
                for(i = 0; i < BLOCKSIZE; i++)
                {
                    tempFileBlock->data[i] = ((char*)buf)[i + offsetCount];
                    byteCounter++;
                    offsetCount++;
                }
                write_block(tempFileBlock, atoi(tempSuper->root) + tempBlockNum);
            }
        }
        free(tempSuper);
        free(tempFileBlock);
        free(tempEmptyFileBlock);
        free(tempFatBlock);
        return(byteCounter);
    }
    free(tempSuper);
    return (0); 
}

int sfs_delete(char *filename)
{
    if(vdisk_fd == 0)
    {
        printf("There is no disk mounted.\n");
        return(-1);
    }

    int i;
    for (i = 0; i < 10; i++)
    {
        if(strcmp(openFiles->entries[i].fileName, filename) == 0 && strcmp(openFiles->entries[i].attributes.isValid, "1") == 1){
            printf("File is not closed please close the file before delete it\n");
            return (-1);
        }
    }

    SuperBlock* tempSuper = malloc(sizeof(SuperBlock));
    DirectoryEntryBlock* tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
    
    read_block(tempSuper, 0);

    //printf("Entry count is: %d\n", atoi(tempSuper->dirEntryCount));

    int isItFound = 0;
    int k;
    int firstBlock;
    for(i = 0; i < 7 && (isItFound == 0); i++)
    {
        read_block(tempDirBlock, atoi(tempSuper->file) + i);
        for(k = 0; k < 8 && (isItFound == 0); k++)
            {
                if(strcmp(tempDirBlock->entries[k].fileName, filename) == 0 && strcmp(tempDirBlock->entries[k].attributes.isValid, "1") == 0){
                    isItFound = 1;
                    firstBlock = atoi(tempDirBlock->entries[k].attributes.firstblock);
                    //make tempdirblock deleted
                    //printf("Deletion file is found\n");
                    strcpy(tempDirBlock->entries[k].attributes.isValid, "0");
                    write_block(tempDirBlock, atoi(tempSuper->file) + i);
                }
            }
    }

    if(isItFound == 0){
        printf("File is not exist in directory block entries\n");
        return (-1);
    }

    EmptyFileBlock* tempEmptyBlock = malloc(sizeof(EmptyFileBlock));
    FatBlock* tempFatBlock = malloc(sizeof(FatBlock));
    int continueFlag = 1;
    char tempNext[8];
    while(continueFlag)
    {
        strcpy(tempEmptyBlock->nextEmptyBlock, tempSuper->nextFree);
        write_block(tempEmptyBlock, atoi(tempSuper->root) + firstBlock);
        sprintf(tempNext, "%d", firstBlock);
        strcpy(tempSuper->nextFree, tempNext);
        if(atoi(fatTable->entries[firstBlock].nextBlock) == - 1)
        {
            continueFlag = 0;
            char tempCount[3];
            sprintf(tempCount, "%d", atoi(tempSuper->dirEntryCount) - 1);
            strcpy(tempSuper->dirEntryCount, tempCount);
            write_block(tempSuper, 0);
            free(tempSuper);
            free(tempDirBlock);
            free(tempEmptyBlock);
            free(tempFatBlock);
            return(0);
        }
        else
        {
            firstBlock = atoi(fatTable->entries[firstBlock].nextBlock);
            strcpy(fatTable->entries[atoi(tempSuper->nextFree)].nextBlock, "-1");
            read_block(tempFatBlock, atoi(tempSuper->fat) + (atoi(tempSuper->nextFree) / 128));
            strcpy(tempFatBlock->entries[atoi(tempSuper->nextFree) % 128].nextBlock, "-1");
            write_block(tempFatBlock, atoi(tempSuper->fat) + (atoi(tempSuper->nextFree) / 128));
        }
    }

    free(tempSuper);
    free(tempDirBlock);
    free(tempEmptyBlock);
    free(tempFatBlock);
    return (0); 
}

void cpyDirEntryBlock(DirectoryEntryBlock* dest, DirectoryEntryBlock* source)
{
    int i = 0;
    for(i = 0; i < 8; i++)
    {
        strcpy(dest->entries[i].fileName, source->entries[i].fileName);
        strcpy(dest->entries[i].attributes.size, source->entries[i].attributes.size);
        strcpy(dest->entries[i].attributes.isValid, source->entries[i].attributes.isValid);
        strcpy(dest->entries[i].attributes.mode, source->entries[i].attributes.mode);
        strcpy(dest->entries[i].attributes.firstblock, source->entries[i].attributes.firstblock);
        strcpy(dest->entries[i].attributes.openByteOffset, source->entries[i].attributes.openByteOffset);
        strcpy(dest->entries[i].attributes.dirEntryBlockNum, source->entries[i].attributes.dirEntryBlockNum);
        strcpy(dest->entries[i].attributes.dirEntryNum, source->entries[i].attributes.dirEntryNum);
    }
    return (0);
}

void cpyDirEntry(DirectoryEntry* dest, DirectoryEntry* source)
{
    strcpy(dest->fileName, source->fileName);
    strcpy(dest->attributes.size, source->attributes.size);
    strcpy(dest->attributes.isValid, source->attributes.isValid);
    strcpy(dest->attributes.mode, source->attributes.mode);
    strcpy(dest->attributes.firstblock, source->attributes.firstblock);
    strcpy(dest->attributes.openByteOffset, source->attributes.openByteOffset);
    strcpy(dest->attributes.dirEntryBlockNum, source->attributes.dirEntryBlockNum);
    strcpy(dest->attributes.dirEntryNum, source->attributes.dirEntryNum);
    return(0);
}

/* This method controls that is the file name exist on the disk block entries. 
 *  If not, it returns 1 else 0
 */
int fileNameController(char *filename){

    //printf("The file name is: %s\n", filename);
    
    SuperBlock* tempSuper = malloc(sizeof(SuperBlock));
    DirectoryEntryBlock* tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
    
    read_block(tempSuper, 0);

    //printf("Entry count is: %d\n", atoi(tempSuper->dirEntryCount));

    int isItFound = 0;
    int i;
    int k;
    for(i = 0; i < 7 && (isItFound == 0); i++)
    {
        read_block(tempDirBlock, atoi(tempSuper->file) + i);
        for(k = 0; k < 8 && (isItFound == 0); k++)
            {
                if(strcmp(tempDirBlock->entries[k].fileName, filename) == 0 && strcmp(tempDirBlock->entries[k].attributes.isValid, "1") == 0){
                    isItFound = 1;
                    //printf("File Name already exist in Directory Entries Directory Block %d and Directory Entry %d\n", i , k);
                }
            }
    }

    free(tempSuper);
    free(tempDirBlock);

    if(isItFound == 0){
        return (1);
    }else{
        return (-1);
    }

}

/* 
 *  This method prints directory entry structure in directory blocks
 */
void printDirectoryEntryStructure(){


    SuperBlock* tempSuper = malloc(sizeof(SuperBlock));
    DirectoryEntryBlock* tempDirBlock = malloc(sizeof(DirectoryEntryBlock));
    
    read_block(tempSuper, 0);

    printf("---------Directory Structure---------\n");
    printf("Entry count is: %d\n", atoi(tempSuper->dirEntryCount));

    int isItFound = 0;
    int count = 0;
    int i;
    int k;
    for(i = 0; i < 7; i++)
    {
        read_block(tempDirBlock, atoi(tempSuper->file) + i);
        printf("Data Block %d\n", i);
        for(k = 0; k < 8; k++)
        {
                printf("E%d: %s, Fb:%s, Iv:%s, Md:%s, Sz:%s, Oo:%s, Bn:%s En:%s  ", k, tempDirBlock->entries[k].fileName, tempDirBlock->entries[k].attributes.firstblock, tempDirBlock->entries[k].attributes.isValid, tempDirBlock->entries[k].attributes.mode, tempDirBlock->entries[k].attributes.size, tempDirBlock->entries[k].attributes.openByteOffset, tempDirBlock->entries[k].attributes.dirEntryBlockNum, tempDirBlock->entries[k].attributes.dirEntryNum);
                if(k == 4){
                    printf("\n");
                }
        }
        printf("\n");
    }

    printf("-------------------------------------\n");

    free(tempSuper);
    free(tempDirBlock);

    return 1;
}

void printOpenFileTable(){
    int i;
    for (i = 0; i < 10; i++)
    {
        printf("OpenFileTable index %d: %s, FirstBlock:%s, isValid:%s, Mode:%s, Size:%s, OpenByteOffset:%s, DirEntryBlockNum:%s, DirEntryNum:%s\n", i, openFiles->entries[i].fileName, openFiles->entries[i].attributes.firstblock, openFiles->entries[i].attributes.isValid, openFiles->entries[i].attributes.mode, openFiles->entries[i].attributes.size, openFiles->entries[i].attributes.openByteOffset, openFiles->entries[i].attributes.dirEntryBlockNum, openFiles->entries[i].attributes.dirEntryNum);
    }
}