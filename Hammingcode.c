[24bcs170@mepcolinux EXP3]$cat Ham_Send.c
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHAR 100
#define DATA_BITS 7
#define CODE_SIZE 12

int dataBits[MAX_CHAR][DATA_BITS];
int codeword[MAX_CHAR][CODE_SIZE];
char text[MAX_CHAR + 1];
int totalChars;

/* positions 1, 2, 4, 8 are parity bit positions in the codeword */
int isCheckPosition(int pos) {
    if (pos == 1 || pos == 2 || pos == 4 || pos == 8) {
        return 1;
    } else {
        return 0;
    }
}

int checkBit(int position, int mask) {
    if ((position & mask) != 0) {
        return 1;
    } else {
        return 0;
    }
}

void readMessage() {
    FILE *fp;
    int ch;
    int b;

    fp = fopen("input.txt", "r");
    if (fp == NULL) {
        printf("Cannot open input.txt\n");
        exit(1);
    }

    totalChars = 0;
    ch = fgetc(fp);
    while (ch != EOF && totalChars < MAX_CHAR) {
        if (ch == '\n' || ch == '\r') {
            ch = fgetc(fp);
            continue;
        }

        text[totalChars] = ch;

        for (b = 6; b >= 0; b--) {
            if ((ch >> b) & 1) {
                dataBits[totalChars][6 - b] = 1;
            } else {
                dataBits[totalChars][6 - b] = 0;
            }
        }

        totalChars++;
        ch = fgetc(fp);
    }

    text[totalChars] = '\0';
    fclose(fp);
}

void showBits() {
    int i, j;

    printf("Binary value of the message:\n");
    for (i = 0; i < totalChars; i++) {
        printf("%c : ", text[i]);
        for (j = 0; j < DATA_BITS; j++) {
            printf("%d", dataBits[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void makeHamming(int row) {
    int i, dataIndex, pos, parityPos, sum, j, totalOnes;

    for (i = 0; i < CODE_SIZE; i++) {
        codeword[row][i] = 0;
    }

    /* put the 7 data bits into the non parity positions */
    dataIndex = 0;
    for (pos = 1; pos <= 11; pos++) {
        if (isCheckPosition(pos) == 0) {
            codeword[row][pos] = dataBits[row][dataIndex];
            dataIndex++;
        }
    }

    /* work out parity bits 1, 2, 4, 8 */
    parityPos = 1;
    while (parityPos <= 8) {
        sum = 0;
        for (j = 1; j <= 11; j++) {
            if (checkBit(j, parityPos) == 1) {
                sum = sum + codeword[row][j];
            }
        }
        codeword[row][parityPos] = sum % 2;
        parityPos = parityPos * 2;
    }

    /* overall parity bit goes in position 0 */
    totalOnes = 0;
    for (j = 1; j <= 11; j++) {
        if (codeword[row][j] == 1) {
            totalOnes++;
        }
    }
    codeword[row][0] = totalOnes % 2;
}

void makeAllHamming() {
    int i;

    for (i = 0; i < totalChars; i++) {
        makeHamming(i);
    }
}

void injectError() {
    char choice;
    int charNum, bitPos;

    printf("Do you want to add an error before sending? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        printf("Enter character number (1 to %d): ", totalChars);
        scanf("%d", &charNum);
        printf("Enter bit index (0 to 11): ");
        scanf("%d", &bitPos);

        charNum = charNum - 1;

        if (charNum >= 0 && charNum < totalChars && bitPos >= 0 && bitPos < CODE_SIZE) {
            if (codeword[charNum][bitPos] == 1) {
                codeword[charNum][bitPos] = 0;
            } else {
                codeword[charNum][bitPos] = 1;
            }
            printf("Error added on character %d at bit %d\n", charNum + 1, bitPos);
        } else {
            printf("Wrong character number or bit index\n");
        }
    } else {
        printf("Sending message without any error\n");
    }
}

void writeFile() {
    FILE *fp;
    int i, j;

    fp = fopen("transmitted.txt", "w");
    if (fp == NULL) {
        printf("Cannot create transmitted.txt\n");
        exit(1);
    }

    fprintf(fp, "%d\n", totalChars);
    for (i = 0; i < totalChars; i++) {
        for (j = 0; j < CODE_SIZE; j++) {
            fprintf(fp, "%d ", codeword[i][j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("Sender: Hamming codewords saved to transmitted.txt\n");
}

int main() {
    readMessage();

    printf("Message read: %s\n", text);
    printf("Total characters: %d\n\n", totalChars);

    showBits();
    makeAllHamming();
    injectError();
    writeFile();

    return 0;
}
[24bcs170@mepcolinux EXP3]$cat Ham_Receiver.c
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHAR 100
#define DATA_BITS 7
#define CODE_SIZE 12

int codeword[MAX_CHAR][CODE_SIZE];
int totalChars;

int checkBit(int position, int mask) {
    if ((position & mask) != 0) {
        return 1;
    } else {
        return 0;
    }
}

void readFile() {
    FILE *fp;
    int i, j;

    fp = fopen("transmitted.txt", "r");
    if (fp == NULL) {
        printf("Cannot open transmitted.txt\n");
        exit(1);
    }

    fscanf(fp, "%d", &totalChars);

    for (i = 0; i < totalChars; i++) {
        for (j = 0; j < CODE_SIZE; j++) {
            fscanf(fp, "%d", &codeword[i][j]);
        }
    }

    fclose(fp);
}

char getChar(int row) {
    /* data bits sit at positions 3, 5, 6, 7, 9, 10, 11 */
    int bitOrder[DATA_BITS] = {3, 5, 6, 7, 9, 10, 11};
    char ch;
    int i, pos;

    ch = 0;
    for (i = 0; i < DATA_BITS; i++) {
        pos = bitOrder[i];
        ch = ch * 2;
        if (codeword[row][pos] == 1) {
            ch = ch + 1;
        }
    }
    return ch;
}

void checkAndFix() {
    int r, i, j, parityPos, sum, errorPos, totalOnes, overallError;

    printf("Receiver: checking each character...\n\n");

    for (r = 0; r < totalChars; r++) {
        errorPos = 0;
        parityPos = 1;

        while (parityPos <= 8) {
            sum = 0;
            for (j = 1; j <= 11; j++) {
                if (checkBit(j, parityPos) == 1) {
                    sum = sum + codeword[r][j];
                }
            }
            if (sum % 2 != 0) {
                errorPos = errorPos + parityPos;
            }
            parityPos = parityPos * 2;
        }

        totalOnes = 0;
        for (i = 0; i < CODE_SIZE; i++) {
            totalOnes = totalOnes + codeword[r][i];
        }

        if (totalOnes % 2 != 0) {
            overallError = 1;
        } else {
            overallError = 0;
        }

        if (errorPos != 0 && overallError == 1) {
            printf("Character %d: error found at bit %d\n", r + 1, errorPos);
            if (codeword[r][errorPos] == 1) {
                codeword[r][errorPos] = 0;
            } else {
                codeword[r][errorPos] = 1;
            }
            printf("Character %d: error fixed\n", r + 1);
        } else if (errorPos != 0 && overallError == 0) {
            printf("Character %d: two bit error found, cannot be fixed\n", r + 1);
        } else {
            printf("Character %d: no error\n", r + 1);
        }
    }
}

void showMessage() {
    int i;

    printf("\nFinal message: ");
    for (i = 0; i < totalChars; i++) {
        printf("%c", getChar(i));
    }
    printf("\n");
}

int main() {
    readFile();
    checkAndFix();
    showMessage();

    return 0;
}
[24bcs170@mepcolinux EXP3]$cc Ham_Send.c
[24bcs170@mepcolinux EXP3]$./a.out
Message read: Networks
Total characters: 8

Binary value of the message:
N : 1001110
e : 1100101
t : 1110100
w : 1110111
o : 1101111
r : 1110010
k : 1101011
s : 1110011

Do you want to add an error before sending? (y/n): y
Enter character number (1 to 8): 5
Enter bit index (0 to 11): 10
Error added on character 5 at bit 10
Sender: Hamming codewords saved to transmitted.txt
[24bcs164@mepcolinux EXP3]$cc Ham_Receiver.c
[24bcs164@mepcolinux EXP3]$./a.out
Receiver: checking each character...

Character 1: no error
Character 2: no error
Character 3: no error
Character 4: no error
Character 5: error found at bit 10
Character 5: error fixed
Character 6: no error
Character 7: no error
Character 8: no error

Final message: Networks
