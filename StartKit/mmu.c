#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
FILE *outputFile;
FILE *backingStoreFile;
FILE *addressFile;
signed char memoryValue;
int logicalAddress;
char addressBuffer[10];
int pnum = -1, poffset = -1, pageFoundIndex=-1;
int shiftIndex=0,physicalAddress=0,numberOfTLBHits = 0,totalFrames=0,totalPages = 0, TLBindex = 0,numberOfOperations = 0, numberOfFrames = 0, numberOfPageFaults = 0,currentFrame = 0;

typedef struct {
    int pagenum;
    int framenum;
} PageTableEntry;

PageTableEntry TLB[16];
int handleFileError(FILE *file, const char *errorMessage) {
    perror(errorMessage);
    fclose(file);
    exit(1);
}

void initialize_pages(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Invalid arguments. Please enter input in the correct format.\n");
        exit(1);
    }
    totalFrames = atoi(argv[1]);
    totalPages = totalFrames;
    if (!strcmp(argv[1], "256")) {
        totalPages = 256;
    }
    char filename[20];
    sprintf(filename, "output%s.csv", argv[1]);
    outputFile = fopen(filename, "w+");
    if (outputFile == NULL) {
        handleFileError(outputFile, "Error opening output file");
    }
}

void initialize_TLB(PageTableEntry *TLB) {
    memset(TLB, -1, 16 * sizeof(PageTableEntry));
}

void initialize_page_table(PageTableEntry *pagetable, int totalPages) {
    memset(pagetable, -1, totalPages * sizeof(PageTableEntry));
}

void handlePageFaultsTable256(PageTableEntry *pagetable, int pnum, int *currentFrame) {
    *currentFrame = numberOfFrames % totalFrames;
    numberOfFrames++;
    pagetable[pnum].framenum = *currentFrame;
    
}

void handlePageFaultsTable128(PageTableEntry *pagetable, int pnum, int *currentFrame) {
    if (totalFrames != 128) {
        return;
    }
    int newframe = pagetable[127].framenum;
    int i = 127;
    while (i > 0) {
        pagetable[i] = pagetable[i - 1];
        i--;
    }
    pagetable[0].pagenum = pnum;
    if (numberOfFrames > 127) {
        pagetable[0].framenum = newframe;
        *currentFrame = pagetable[0].framenum;
    } else {
        *currentFrame = numberOfFrames % 128;
        pagetable[0].framenum = *currentFrame;
    }
    numberOfFrames++;
}

void calculateAndPrintStatistics() {
    double pft = 100.0 * numberOfPageFaults / numberOfOperations;
    double thr = 100.0 * numberOfTLBHits / numberOfOperations;
    fprintf(outputFile, "Page Faults Rate, %.2f%%,\n", pft);
    fprintf(outputFile, "TLB Hits Rate, %.2f%%,", thr);
}

void closeFiles() {
    fclose(outputFile);
    fclose(backingStoreFile);
    fclose(addressFile);
}

void readFrameFromBackingStore(signed char phymem[][256], int pnum, int currentFrame) {
    if (fseek(backingStoreFile, pnum * 256, SEEK_SET) != 0) {
        handleFileError(backingStoreFile, "Error in retrieving frame from backing_store.bin");
    }
    if (fread(phymem[currentFrame], sizeof(signed char), 256, backingStoreFile) == 0) {
        handleFileError(backingStoreFile, "Error: Can't read from backing store");
    }
}

void updateTLB(int pnum, int frame) {
    TLB[TLBindex].pagenum = pnum;
    TLB[TLBindex].framenum = frame;
    TLBindex = (TLBindex + 1) % 16;
}

int lookupTLB(int pnum) {
    for (int i = 0; i < 16; i++) {
        if (TLB[i].pagenum == pnum) {
            numberOfTLBHits++;
            return TLB[i].framenum; // TLB hit
        }
    }
    return -1; // TLB miss
}

void handleTLBMissTable128(PageTableEntry *pagetable, int *currentFrame, int pnum) {
    for (int i = 0; i < totalPages; i++) {
        if (pagetable[i].pagenum == pnum && pagetable[i].framenum >= 0) {
            pageFoundIndex = i;
            break; 
        }
    }
    if (pageFoundIndex != -1) {
        PageTableEntry entry = pagetable[pageFoundIndex];
        for (int k = pageFoundIndex; k > 0; k--) {
            pagetable[k] = pagetable[k - 1];
        }
        pagetable[0] = entry;
        *currentFrame = entry.framenum;
    }
}

void handleTLBMissTable256(PageTableEntry *pagetable, int *currentFrame, int pnum) {
    if (pagetable[pnum].framenum >= 0) {
        pageFoundIndex = pnum;
        *currentFrame = pagetable[pnum].framenum;
    }
}

int main(int argc, char *argv[]) {
    initialize_pages(argc, argv);
    PageTableEntry pagetable[totalPages];
    signed char phymem[totalFrames][256];
     initialize_TLB(TLB);
    initialize_page_table(pagetable, totalPages);
    backingStoreFile = fopen(argv[2], "r");
    addressFile = fopen(argv[3], "r");
    memset(phymem, 0, totalFrames * 256 * sizeof(signed char));

    while (fgets(addressBuffer, 10, addressFile)) {
        logicalAddress = atoi(addressBuffer);
        pnum = (logicalAddress >> 8) & 0x00FF; 
        poffset = logicalAddress & 0x00FF;   
        pageFoundIndex = -1;
        int pageFoundflag = 0; // Flag to indicate if the page is found in TLB        
        // Check if the page is in the TLB
        for (int i = 0; i < 16; i++) {
            int tlbFrameNumber = lookupTLB(pnum);
            if (tlbFrameNumber != -1) {
                pageFoundflag = 1; 
                currentFrame = tlbFrameNumber;      
                // Special handling for systems with only 128 pages
                if (totalPages == 128) {
                    // Locate the page in the page table
                    for (int j = 0; j < totalPages; j++) {
                        if (pagetable[j].pagenum == pnum) {
                            shiftIndex = j;
                            break;
                        }
                    }
                    currentFrame = (numberOfFrames + 1) % 128;
                    PageTableEntry pageEntry = pagetable[shiftIndex];
                    for (int k = shiftIndex; k > 0; k--) {
                        pagetable[k] = pagetable[k - 1];
                    }
                    pagetable[0] = pageEntry;
                    currentFrame = pagetable[0].framenum;
                }
                break; // Exit the loop as the page is found in TLB
            }
        }

        // TLB miss handling
        if (!pageFoundflag) {
            // Check if the page is in the page table (256 pages case)
            if (totalPages == 256 && pagetable[pnum].framenum >= 0) {
            handleTLBMissTable256(pagetable, &currentFrame, pnum);
            }
            // Check if the page is in the page table (128 pages case)
            if (totalPages == 128) {
           handleTLBMissTable128(pagetable, &currentFrame, pnum);
            }

            // Page fault handling
            if (pageFoundIndex == -1) {
                numberOfPageFaults++;
                // Handling page fault for 256 frames
                if (totalFrames == 256) {
                    handlePageFaultsTable256(pagetable, pnum, &currentFrame);
                }
                // Handling page fault for 128 frames
                if (totalFrames == 128) {
                    handlePageFaultsTable128(pagetable, pnum, &currentFrame);
                }
                readFrameFromBackingStore(phymem, pnum, currentFrame);
            }
            updateTLB(pnum, currentFrame);
        }
        physicalAddress = (currentFrame << 8) | poffset;
        memoryValue = phymem[currentFrame][poffset];
        fprintf(outputFile, "%d,%d,%d\n", logicalAddress, physicalAddress, memoryValue);
        numberOfOperations++;
    }
    calculateAndPrintStatistics();
    closeFiles();
    return 0;
}
