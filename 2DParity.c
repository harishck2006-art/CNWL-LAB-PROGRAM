[24bcs170@mepcolinux EXP3]$cat 2d_Send.c
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHAR 128
#define BITS 8

char bits[MAX_CHAR][BITS];
char text[MAX_CHAR];
int rowParity[MAX_CHAR];
int colParity[BITS];
int cornerParity;
int totalChars;

int getParity(int count) {
    if (count % 2 == 0) {
        return 0;
    } else {
        return 1;
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

        for (b = 7; b >= 0; b--) {
            if ((ch >> b) & 1) {
                bits[totalChars][7 - b] = '1';
            } else {
                bits[totalChars][7 - b] = '0';
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

    printf("Binary bits of the message:\n");
    for (i = 0; i < totalChars; i++) {
        printf("Row %d (%c): ", i + 1, text[i]);
        for (j = 0; j < BITS; j++) {
            printf("%c ", bits[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void makeParity() {
    int i, j, count;

    /* row parity */
    for (i = 0; i < totalChars; i++) {
        count = 0;
        for (j = 0; j < BITS; j++) {
            if (bits[i][j] == '1') {
                count++;
            }
        }
        rowParity[i] = getParity(count);
    }

    /* column parity */
    for (j = 0; j < BITS; j++) {
        count = 0;
        for (i = 0; i < totalChars; i++) {
            if (bits[i][j] == '1') {
                count++;
            }
        }
        colParity[j] = getParity(count);
    }

    /* corner parity */
    count = 0;
    for (i = 0; i < totalChars; i++) {
        if (rowParity[i] == 1) {
            count++;
        }
    }
    cornerParity = getParity(count);
}

void injectError() {
    char choice;
    int row, col;

    printf("Do you want to add an error before sending? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        printf("Enter row number (1 to %d): ", totalChars);
        scanf("%d", &row);
        printf("Enter column number (1 to %d): ", BITS);
        scanf("%d", &col);

        row = row - 1;
        col = col - 1;

        if (row >= 0 && row < totalChars && col >= 0 && col < BITS) {
            if (bits[row][col] == '1') {
                bits[row][col] = '0';
            } else {
                bits[row][col] = '1';
            }
            printf("Error added at row %d column %d\n", row + 1, col + 1);
        } else {
            printf("Wrong row or column number\n");
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
        for (j = 0; j < BITS; j++) {
            fprintf(fp, "%c ", bits[i][j]);
        }
        fprintf(fp, "%d\n", rowParity[i]);
    }

    for (j = 0; j < BITS; j++) {
        fprintf(fp, "%d ", colParity[j]);
    }
    fprintf(fp, "%d\n", cornerParity);

    fclose(fp);
    printf("Sender: message saved to transmitted.txt\n");
}

int main() {
    readMessage();

    printf("Message read: %s\n", text);
    printf("Total characters: %d\n\n", totalChars);

    showBits();
    makeParity();
    injectError();
    writeFile();

    return 0;
}
[24bcs170@mepcolinux EXP3]$cc 2d_Send.c
[24bcs170@mepcolinux EXP3]$./a.out
Message read: Networks
Total characters: 8

Binary bits of the message:
Row 1 (N): 0 1 0 0 1 1 1 0
Row 2 (e): 0 1 1 0 0 1 0 1
Row 3 (t): 0 1 1 1 0 1 0 0
Row 4 (w): 0 1 1 1 0 1 1 1
Row 5 (o): 0 1 1 0 1 1 1 1
Row 6 (r): 0 1 1 1 0 0 1 0
Row 7 (k): 0 1 1 0 1 0 1 1
Row 8 (s): 0 1 1 1 0 0 1 1

Do you want to add an error before sending? (y/n): y
Enter row number (1 to 8): 3
Enter column number (1 to 8): 4
Error added at row 3 column 4
Sender: message saved to transmitted.txt
[24bcs170@mepcolinux EXP3]$cat 2d_Receiver.c
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHAR 128
#define BITS 8

char bits[MAX_CHAR][BITS];
int rowParity[MAX_CHAR];
int colParity[BITS];
int cornerParity;
int totalChars;

int getParity(int count) {
    if (count % 2 == 0) {
        return 0;
    } else {
        return 1;
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
        for (j = 0; j < BITS; j++) {
            fscanf(fp, " %c", &bits[i][j]);
        }
        fscanf(fp, "%d", &rowParity[i]);
    }

    for (j = 0; j < BITS; j++) {
        fscanf(fp, "%d", &colParity[j]);
    }
    fscanf(fp, "%d", &cornerParity);

    fclose(fp);
}

char getChar(int row) {
    char ch;
    int j;

    ch = 0;
    for (j = 0; j < BITS; j++) {
        ch = ch * 2;
        if (bits[row][j] == '1') {
            ch = ch + 1;
        }
    }
    return ch;
}

void checkParity() {
    int i, j, count;
    int errorRow, errorCol;
    int rowOk, colOk;

    errorRow = -1;
    errorCol = -1;

    printf("Receiver: checking the message...\n");

    /* check each row */
    for (i = 0; i < totalChars; i++) {
        count = 0;
        for (j = 0; j < BITS; j++) {
            if (bits[i][j] == '1') {
                count++;
            }
        }
        rowOk = getParity(count);
        if (rowOk != rowParity[i]) {
            errorRow = i;
            printf("Row %d does not match\n", i + 1);
        }
    }

    /* check each column */
    for (j = 0; j < BITS; j++) {
        count = 0;
        for (i = 0; i < totalChars; i++) {
            if (bits[i][j] == '1') {
                count++;
            }
        }
        colOk = getParity(count);
        if (colOk != colParity[j]) {
            errorCol = j;
            printf("Column %d does not match\n", j + 1);
        }
    }

    if (errorRow != -1 && errorCol != -1) {
        printf("\nError found at row %d column %d\n", errorRow + 1, errorCol + 1);

        if (bits[errorRow][errorCol] == '1') {
            bits[errorRow][errorCol] = '0';
        } else {
            bits[errorRow][errorCol] = '1';
        }

        printf("Bit fixed. Message is now correct.\n");
    } else if (errorRow == -1 && errorCol == -1) {
        printf("No error found. Message is correct.\n");
    } else {
        printf("Error found but it cannot be fixed.\n");
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
    checkParity();
    showMessage();

    return 0;
}
[24bcs170@mepcolinux EXP3]$cc 2d_Receiver.c
[24bcs170@mepcolinux EXP3]$./a.out
Receiver: checking the message...
Row 3 does not match
Column 4 does not match

Error found at row 3 column 4
Bit fixed. Message is now correct.
