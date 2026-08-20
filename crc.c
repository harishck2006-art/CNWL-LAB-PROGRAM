

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FRAME_SIZE 10
#define PACKET_SIZE 24

// Control byte values in hex
#define SYN 0x16
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define DLE 0x10

typedef struct
{
    char url[50];
    char ip[20];
    char mac[25];
    int destinationPort;
} URLTable;

URLTable table[3] =
{
    {"www.mail.com", "142.250.183.14", "AA:BB:CC:DD:EE:01", 25},
    {"www.whatsapp.com", "142.250.190.46", "AA:BB:CC:DD:EE:02", 443},
    {"www.facebook.com", "157.240.22.35", "AA:BB:CC:DD:EE:03", 443}
};

// --- BINARY CONVERSION HELPERS ---

void charTo8BitBinary(unsigned char c, char *binaryOut)
{
    for (int i = 7; i >= 0; i--)
    {
        binaryOut[7 - i] = ((c >> i) & 1) ? '1' : '0';
    }
    binaryOut[8] = '\0';
}

void byteBufferToBinaryString(const char *bytes, int len, char *binStrOut)
{
    binStrOut[0] = '\0';
    char temp[9];
    for (int i = 0; i < len; i++)
    {
        charTo8BitBinary((unsigned char)bytes[i], temp);
        strcat(binStrOut, temp);
    }
}

// --- DYNAMIC POLYNOMIAL CONVERSION & CRC FUNCTIONS ---

// Converts binary divisor string to algebraic polynomial expression
void buildPolynomialString(const char *divisor, int degree, char *polyStr)
{
    polyStr[0] = '\0';
    int first = 1;

    for (int i = 0; i <= degree; i++)
    {
        int power = degree - i;
        if (divisor[i] == '1')
        {
            if (!first) strcat(polyStr, " + ");
            first = 0;

            if (power == 0)
            {
                strcat(polyStr, "1");
            }
            else if (power == 1)
            {
                strcat(polyStr, "x");
            }
            else
            {
                char term[20];
                sprintf(term, "x^%d", power);
                strcat(polyStr, term);
            }
        }
    }
    if (first) strcpy(polyStr, "0");
}

// Performs Modulo-2 Polynomial Division using the User-Defined Divisor
void calculateCRCBinary(const char *binaryData, const char *divisor, char *crcOut)
{
    int dataLen = strlen(binaryData);
    int divLen = strlen(divisor);
    int crcLen = divLen - 1;

    char temp[10000];
    strcpy(temp, binaryData);

    // Augment with crcLen zero bits
    for (int i = 0; i < crcLen; i++)
    {
        strcat(temp, "0");
    }

    printf("\n--- Step-by-Step CRC Long Division ---\n");
    printf("Augmented Binary Stream : %s\n", temp);
    printf("Divisor Binary Bitstring: %s\n", divisor);

    // Modulo-2 Division via XOR
    for (int i = 0; i < dataLen; i++)
    {
        if (temp[i] == '1')
        {
            for (int j = 0; j < divLen; j++)
            {
                temp[i + j] = (temp[i + j] == divisor[j]) ? '0' : '1';
            }
        }
    }

    // Extract remainder bits
    strncpy(crcOut, &temp[dataLen], crcLen);
    crcOut[crcLen] = '\0';

    printf("Calculated CRC Remainder : %s (%d bits)\n", crcOut, crcLen);
}

// Even Parity Calculation on Binary Bitstring
char calculateEvenParityBinary(const char *binaryData)
{
    int onesCount = 0;
    for (int i = 0; binaryData[i] != '\0'; i++)
    {
        if (binaryData[i] == '1') onesCount++;
    }
    return (onesCount % 2 == 0) ? '0' : '1';
}

// --- STANDARD LAYER CONVERSIONS ---

void decimalToBinary(int n, char binary[])
{
    int i, j = 0;
    for(i = 7; i >= 0; i--)
    {
        binary[j++] = ((n & (1 << i)) != 0) ? '1' : '0';
    }
    binary[j] = '\0';
}

void messageToBinary(char message[], char binary[])
{
    int j = 0;
    char temp[9];
    for(int i = 0; message[i] != '\0'; i++)
    {
        charTo8BitBinary(message[i], temp);
        for(int k = 0; k < 8; k++) binary[j++] = temp[k];
    }
    binary[j] = '\0';
}

void portToBinary(int port, char binary[])
{
    int j = 0;
    for(int i = 15; i >= 0; i--)
    {
        binary[j++] = ((port >> i) & 1) ? '1' : '0';
    }
    binary[j] = '\0';
}

int searchURL(char url[])
{
    for(int i = 0; i < 3; i++)
    {
        if(strcmp(table[i].url, url) == 0) return i;
    }
    return -1;
}

void printTable()
{
    printf("\n================ URL TABLE ================\n");
    printf("%-20s %-18s %-20s\n","URL","IP","MAC");
    for(int i = 0; i < 3; i++)
    {
        printf("%-20s %-18s %-20s\n", table[i].url, table[i].ip, table[i].mac);
    }
    printf("===========================================\n");
}

int performCharacterStuffing(const char input[], char output[])
{
    int inputIndex = 0, outputIndex = 0;
    printf("\n--- Starting Character Stuffing ---\n");

    while (input[inputIndex] != '\0')
    {
        if (strncmp(&input[inputIndex], "ETX", 3) == 0)
        {
            printf("Found text 'ETX' -> Stuffed 0x%02X 0x%02X\n", DLE, ETX);
            output[outputIndex++] = DLE;
            output[outputIndex++] = ETX;
            inputIndex += 3;
        }
        else if (strncmp(&input[inputIndex], "DLE", 3) == 0)
        {
            printf("Found text 'DLE' -> Stuffed 0x%02X 0x%02X\n", DLE, DLE);
            output[outputIndex++] = DLE;
            output[outputIndex++] = DLE;
            inputIndex += 3;
        }
        else if (strncmp(&input[inputIndex], "STX", 3) == 0)
        {
            printf("Found text 'STX' -> Stuffed 0x%02X 0x%02X\n", DLE, STX);
            output[outputIndex++] = DLE;
            output[outputIndex++] = STX;
            inputIndex += 3;
        }
        else
        {
            output[outputIndex++] = input[inputIndex++];
        }
    }
    printf("--- Stuffing complete! ---\n");
    return outputIndex;
}

void buildBisyncFrameBinary(const char stuffedBodyBin[], const char divisor[], char frameBinOut[])
{
    char synBin[9], sohBin[9], stxBin[9], etxBin[9];
    char hBin[9], dBin[9], rBin[9];

    charTo8BitBinary(SYN, synBin);
    charTo8BitBinary(SOH, sohBin);
    charTo8BitBinary(STX, stxBin);
    charTo8BitBinary(ETX, etxBin);
    charTo8BitBinary('H', hBin);
    charTo8BitBinary('D', dBin);
    charTo8BitBinary('R', rBin);

    frameBinOut[0] = '\0';

    // 1. Header Flags & Header Text
    strcat(frameBinOut, synBin); // SYN
    strcat(frameBinOut, synBin); // SYN
    strcat(frameBinOut, sohBin); // SOH
    strcat(frameBinOut, hBin);   // 'H'
    strcat(frameBinOut, dBin);   // 'D'
    strcat(frameBinOut, rBin);   // 'R'
    strcat(frameBinOut, stxBin); // STX

    // 2. Binary Payload Body
    strcat(frameBinOut, stuffedBodyBin);

    // 3. ETX Marker
    strcat(frameBinOut, etxBin);

    // 4. Calculate Binary CRC and Even Parity
    int degree = strlen(divisor) - 1;
    char crcBits[32];
    calculateCRCBinary(stuffedBodyBin, divisor, crcBits);
    char parityBit = calculateEvenParityBinary(stuffedBodyBin);

    // Construct 8-bit Checksum Trailer Byte: [ParityBit][Pad Zeros...][CRC Bits]
    char trailerBin[9];
    trailerBin[0] = parityBit;

    int padZeros = 8 - 1 - degree;
    for (int i = 1; i <= padZeros; i++)
    {
        trailerBin[i] = '0';
    }
    strncpy(&trailerBin[1 + padZeros], crcBits, degree);
    trailerBin[8] = '\0';

    strcat(frameBinOut, trailerBin);

    printf("\n>>> BINARY CHECKSUM GENERATION <<<\n");
    printf("  Binary Payload Length : %zu bits\n", strlen(stuffedBodyBin));
    printf("  Calculated Parity Bit  : %c (Even Parity)\n", parityBit);
    printf("  Calculated CRC Bits    : %s\n", crcBits);
    printf("  Trailer Checksum Byte  : %s\n", trailerBin);
}

void saveBinaryFrameToFile(const char filename[], const char divisor[], const char binaryFrame[])
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error: Could not save binary frame file!\n");
        exit(1);
    }
    // Save Divisor on Line 1, Full Frame on Line 2
    fprintf(fp, "%s\n%s\n", divisor, binaryFrame);
    fclose(fp);
}

int main()
{
    FILE *fp;
    char filename[100], message[500], url[50];

    printf("File Name: ");
    scanf("%s", filename);

    fp = fopen(filename, "r");
    if(fp == NULL)
    {
        printf("Error: File cannot be opened.\n");
        return 0;
    }
    if(fgets(message, sizeof(message), fp) == NULL)
    {
        printf("Error: File is empty or cannot be read.\n");
        fclose(fp);
        return 0;
    }
    fclose(fp);

    int a = 0;
    while(message[a] != '\0')
    {
        if(message[a] == '\n') { message[a] = '\0'; break; }
        a++;
    }

    printTable();
    printf("\nDestination URL : ");
    scanf("%s", url);

    int index = searchURL(url);
    if(index == -1)
    {
        printf("URL Not Found\n");
        return 0;
    }

    srand(time(NULL));
    int sourcePort = rand() % (65535 - 49152 + 1) + 49152;
    int destinationPort = table[index].destinationPort;

    char srcPortBinary[17], destPortBinary[17], binaryMessage[1000];
    portToBinary(sourcePort, srcPortBinary);
    portToBinary(destinationPort, destPortBinary);
    messageToBinary(message, binaryMessage);

    printf("\nMessage : %s\n", message);
    printf("Binary Message Representation:\n%s\n", binaryMessage);

    // Dynamic Polynomial Input from User
    int degree;
    printf("\n--------------------------------------------------------\n");
    printf("         DYNAMIC CRC POLYNOMIAL CONFIGURATION           \n");
    printf("--------------------------------------------------------\n");
    printf("Enter highest degree of polynomial : ");
    scanf("%d", &degree);

    if (degree < 1 || degree > 7)
    {
        printf("Error: Polynomial degree must be between 1 and 7.\n");
        return 1;
    }

    char divisor[32];
    printf("Enter coefficient for x^%d (must be 1): ", degree);
    int coeff;
    scanf("%d", &coeff);
    divisor[0] = '1';

    for (int i = degree - 1; i >= 0; i--)
    {
        printf("Enter coefficient for x^%d (0 or 1): ", i);
        scanf("%d", &coeff);
        divisor[degree - i] = (coeff == 1) ? '1' : '0';
    }
    divisor[degree + 1] = '\0';

    char polyFormula[100];
    buildPolynomialString(divisor, degree, polyFormula);

    printf("\nPolynomial Expression   : G(x) = %s\n", polyFormula);
    printf("Divisor Binary Bitstring : %s\n", divisor);

    // Character Stuffing
    char stuffedBody[1000];
    int stuffedLen = performCharacterStuffing(message, stuffedBody);

    // Convert Stuffed Payload to Pure Binary
    char stuffedBodyBin[8000];
    byteBufferToBinaryString(stuffedBody, stuffedLen, stuffedBodyBin);

    printf("\nStuffed Payload in Pure Binary:\n%s\n", stuffedBodyBin);

    // Assemble Binary BISYNC Frame
    char fullBinaryFrame[10000];
    buildBisyncFrameBinary(stuffedBodyBin, divisor, fullBinaryFrame);

    printf("\n========================================================\n");
    printf(" FINAL TRANSMITTED BINARY FRAME STREAM: \n");
    printf("%s\n", fullBinaryFrame);
    printf("========================================================\n");

    saveBinaryFrameToFile("bisync_output.txt", divisor, fullBinaryFrame);
    printf("[Sender]: Frame stream and polynomial divisor saved to 'bisync_output.txt'\n");

    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FRAME_SIZE 10
#define PACKET_SIZE 24

// Control byte values in hex
#define SYN 0x16
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define DLE 0x10

typedef struct
{
    char url[50];
    char ip[20];
    char mac[25];
    int destinationPort;
} URLTable;

URLTable table[3] =
{
    {"www.mail.com", "142.250.183.14", "AA:BB:CC:DD:EE:01", 25},
    {"www.whatsapp.com", "142.250.190.46", "AA:BB:CC:DD:EE:02", 443},
    {"www.facebook.com", "157.240.22.35", "AA:BB:CC:DD:EE:03", 443}
};

// --- BINARY CONVERSION HELPERS ---

void charTo8BitBinary(unsigned char c, char *binaryOut)
{
    for (int i = 7; i >= 0; i--)
    {
        binaryOut[7 - i] = ((c >> i) & 1) ? '1' : '0';
    }
    binaryOut[8] = '\0';
}

void byteBufferToBinaryString(const char *bytes, int len, char *binStrOut)
{
    binStrOut[0] = '\0';
    char temp[9];
    for (int i = 0; i < len; i++)
    {
        charTo8BitBinary((unsigned char)bytes[i], temp);
        strcat(binStrOut, temp);
    }
}

// --- DYNAMIC POLYNOMIAL CONVERSION & CRC FUNCTIONS ---

// Converts binary divisor string to algebraic polynomial expression
void buildPolynomialString(const char *divisor, int degree, char *polyStr)
{
    polyStr[0] = '\0';
    int first = 1;

    for (int i = 0; i <= degree; i++)
    {
        int power = degree - i;
        if (divisor[i] == '1')
        {
            if (!first) strcat(polyStr, " + ");
            first = 0;

            if (power == 0)
            {
                strcat(polyStr, "1");
            }
            else if (power == 1)
            {
                strcat(polyStr, "x");
            }
            else
            {
                char term[20];
                sprintf(term, "x^%d", power);
                strcat(polyStr, term);
            }
        }
    }
    if (first) strcpy(polyStr, "0");
}

// Performs Modulo-2 Polynomial Division using the User-Defined Divisor
void calculateCRCBinary(const char *binaryData, const char *divisor, char *crcOut)
{
    int dataLen = strlen(binaryData);
    int divLen = strlen(divisor);
    int crcLen = divLen - 1;

    char temp[10000];
    strcpy(temp, binaryData);

    // Augment with crcLen zero bits
    for (int i = 0; i < crcLen; i++)
    {
        strcat(temp, "0");
    }

    printf("\n--- Step-by-Step CRC Long Division ---\n");
    printf("Augmented Binary Stream : %s\n", temp);
    printf("Divisor Binary Bitstring: %s\n", divisor);

    // Modulo-2 Division via XOR
    for (int i = 0; i < dataLen; i++)
    {
        if (temp[i] == '1')
        {
            for (int j = 0; j < divLen; j++)
            {
                temp[i + j] = (temp[i + j] == divisor[j]) ? '0' : '1';
            }
        }
    }

    // Extract remainder bits
    strncpy(crcOut, &temp[dataLen], crcLen);
    crcOut[crcLen] = '\0';

    printf("Calculated CRC Remainder : %s (%d bits)\n", crcOut, crcLen);
}

// Even Parity Calculation on Binary Bitstring
char calculateEvenParityBinary(const char *binaryData)
{
    int onesCount = 0;
    for (int i = 0; binaryData[i] != '\0'; i++)
    {
        if (binaryData[i] == '1') onesCount++;
    }
    return (onesCount % 2 == 0) ? '0' : '1';
}

// --- STANDARD LAYER CONVERSIONS ---

void decimalToBinary(int n, char binary[])
{
    int i, j = 0;
    for(i = 7; i >= 0; i--)
    {
        binary[j++] = ((n & (1 << i)) != 0) ? '1' : '0';
    }
    binary[j] = '\0';
}

void messageToBinary(char message[], char binary[])
{
    int j = 0;
    char temp[9];
    for(int i = 0; message[i] != '\0'; i++)
    {
        charTo8BitBinary(message[i], temp);
        for(int k = 0; k < 8; k++) binary[j++] = temp[k];
    }
    binary[j] = '\0';
}

void portToBinary(int port, char binary[])
{
    int j = 0;
    for(int i = 15; i >= 0; i--)
    {
        binary[j++] = ((port >> i) & 1) ? '1' : '0';
    }
    binary[j] = '\0';
}

int searchURL(char url[])
{
    for(int i = 0; i < 3; i++)
    {
        if(strcmp(table[i].url, url) == 0) return i;
    }
    return -1;
}

void printTable()
{
    printf("\n================ URL TABLE ================\n");
    printf("%-20s %-18s %-20s\n","URL","IP","MAC");
    for(int i = 0; i < 3; i++)
    {
        printf("%-20s %-18s %-20s\n", table[i].url, table[i].ip, table[i].mac);
    }
    printf("===========================================\n");
}

int performCharacterStuffing(const char input[], char output[])
{
    int inputIndex = 0, outputIndex = 0;
    printf("\n--- Starting Character Stuffing ---\n");

    while (input[inputIndex] != '\0')
    {
        if (strncmp(&input[inputIndex], "ETX", 3) == 0)
        {
            printf("Found text 'ETX' -> Stuffed 0x%02X 0x%02X\n", DLE, ETX);
            output[outputIndex++] = DLE;
            output[outputIndex++] = ETX;
            inputIndex += 3;
        }
        else if (strncmp(&input[inputIndex], "DLE", 3) == 0)
        {
            printf("Found text 'DLE' -> Stuffed 0x%02X 0x%02X\n", DLE, DLE);
            output[outputIndex++] = DLE;
            output[outputIndex++] = DLE;
            inputIndex += 3;
        }
        else if (strncmp(&input[inputIndex], "STX", 3) == 0)
        {
            printf("Found text 'STX' -> Stuffed 0x%02X 0x%02X\n", DLE, STX);
            output[outputIndex++] = DLE;
            output[outputIndex++] = STX;
            inputIndex += 3;
        }
        else
        {
            output[outputIndex++] = input[inputIndex++];
        }
    }
    printf("--- Stuffing complete! ---\n");
    return outputIndex;
}

void buildBisyncFrameBinary(const char stuffedBodyBin[], const char divisor[], char frameBinOut[])
{
    char synBin[9], sohBin[9], stxBin[9], etxBin[9];
    char hBin[9], dBin[9], rBin[9];

    charTo8BitBinary(SYN, synBin);
    charTo8BitBinary(SOH, sohBin);
    charTo8BitBinary(STX, stxBin);
    charTo8BitBinary(ETX, etxBin);
    charTo8BitBinary('H', hBin);
    charTo8BitBinary('D', dBin);
    charTo8BitBinary('R', rBin);

    frameBinOut[0] = '\0';

    // 1. Header Flags & Header Text
    strcat(frameBinOut, synBin); // SYN
    strcat(frameBinOut, synBin); // SYN
    strcat(frameBinOut, sohBin); // SOH
    strcat(frameBinOut, hBin);   // 'H'
    strcat(frameBinOut, dBin);   // 'D'
    strcat(frameBinOut, rBin);   // 'R'
    strcat(frameBinOut, stxBin); // STX

    // 2. Binary Payload Body
    strcat(frameBinOut, stuffedBodyBin);

    // 3. ETX Marker
    strcat(frameBinOut, etxBin);

    // 4. Calculate Binary CRC and Even Parity
    int degree = strlen(divisor) - 1;
    char crcBits[32];
    calculateCRCBinary(stuffedBodyBin, divisor, crcBits);
    char parityBit = calculateEvenParityBinary(stuffedBodyBin);

    // Construct 8-bit Checksum Trailer Byte: [ParityBit][Pad Zeros...][CRC Bits]
    char trailerBin[9];
    trailerBin[0] = parityBit;

    int padZeros = 8 - 1 - degree;
    for (int i = 1; i <= padZeros; i++)
    {
        trailerBin[i] = '0';
    }
    strncpy(&trailerBin[1 + padZeros], crcBits, degree);
    trailerBin[8] = '\0';

    strcat(frameBinOut, trailerBin);

    printf("\n>>> BINARY CHECKSUM GENERATION <<<\n");
    printf("  Binary Payload Length : %zu bits\n", strlen(stuffedBodyBin));
    printf("  Calculated Parity Bit  : %c (Even Parity)\n", parityBit);
    printf("  Calculated CRC Bits    : %s\n", crcBits);
    printf("  Trailer Checksum Byte  : %s\n", trailerBin);
}

void saveBinaryFrameToFile(const char filename[], const char divisor[], const char binaryFrame[])
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error: Could not save binary frame file!\n");
        exit(1);
    }
    // Save Divisor on Line 1, Full Frame on Line 2
    fprintf(fp, "%s\n%s\n", divisor, binaryFrame);
    fclose(fp);
}

int main()
{
    FILE *fp;
    char filename[100], message[500], url[50];

    printf("File Name: ");
    scanf("%s", filename);

    fp = fopen(filename, "r");
    if(fp == NULL)
    {
        printf("Error: File cannot be opened.\n");
        return 0;
    }
    if(fgets(message, sizeof(message), fp) == NULL)
    {
        printf("Error: File is empty or cannot be read.\n");
        fclose(fp);
        return 0;
    }
    fclose(fp);

    int a = 0;
    while(message[a] != '\0')
    {
        if(message[a] == '\n') { message[a] = '\0'; break; }
        a++;
    }

    printTable();
    printf("\nDestination URL : ");
    scanf("%s", url);

    int index = searchURL(url);
    if(index == -1)
    {
        printf("URL Not Found\n");
        return 0;
    }

    srand(time(NULL));
    int sourcePort = rand() % (65535 - 49152 + 1) + 49152;
    int destinationPort = table[index].destinationPort;

    char srcPortBinary[17], destPortBinary[17], binaryMessage[1000];
    portToBinary(sourcePort, srcPortBinary);
    portToBinary(destinationPort, destPortBinary);
    messageToBinary(message, binaryMessage);

    printf("\nMessage : %s\n", message);
    printf("Binary Message Representation:\n%s\n", binaryMessage);

    // Dynamic Polynomial Input from User
    int degree;
    printf("\n--------------------------------------------------------\n");
    printf("         DYNAMIC CRC POLYNOMIAL CONFIGURATION           \n");
    printf("--------------------------------------------------------\n");
    printf("Enter highest degree of polynomial : ");
    scanf("%d", &degree);

    if (degree < 1 || degree > 7)
    {
        printf("Error: Polynomial degree must be between 1 and 7.\n");
        return 1;
    }

    char divisor[32];
    printf("Enter coefficient for x^%d (must be 1): ", degree);
    int coeff;
    scanf("%d", &coeff);
    divisor[0] = '1';

    for (int i = degree - 1; i >= 0; i--)
    {
        printf("Enter coefficient for x^%d (0 or 1): ", i);
        scanf("%d", &coeff);
        divisor[degree - i] = (coeff == 1) ? '1' : '0';
    }
    divisor[degree + 1] = '\0';

    char polyFormula[100];
    buildPolynomialString(divisor, degree, polyFormula);

    printf("\nPolynomial Expression   : G(x) = %s\n", polyFormula);
    printf("Divisor Binary Bitstring : %s\n", divisor);

    // Character Stuffing
    char stuffedBody[1000];
    int stuffedLen = performCharacterStuffing(message, stuffedBody);

    // Convert Stuffed Payload to Pure Binary
    char stuffedBodyBin[8000];
    byteBufferToBinaryString(stuffedBody, stuffedLen, stuffedBodyBin);

    printf("\nStuffed Payload in Pure Binary:\n%s\n", stuffedBodyBin);

    // Assemble Binary BISYNC Frame
    char fullBinaryFrame[10000];
    buildBisyncFrameBinary(stuffedBodyBin, divisor, fullBinaryFrame);

    printf("\n========================================================\n");
    printf(" FINAL TRANSMITTED BINARY FRAME STREAM: \n");
    printf("%s\n", fullBinaryFrame);
    printf("========================================================\n");

    saveBinaryFrameToFile("bisync_output.txt", divisor, fullBinaryFrame);
    printf("[Sender]: Frame stream and polynomial divisor saved to 'bisync_output.txt'\n");

    return 0;
}



 HASH TABLE (URL -> IP -> MAC)
---------------------------------------------------
 Slot 1 : facebook.com    | 157.240.22.35   | F0:2F:74:6B:88:11
 Slot 2 : google.com      | 142.250.193.14  | 3C:5A:B4:1D:9F:02
 Slot 3 : amazon.com      | 205.251.242.103 | B8:27:EB:9A:3C:44
 Slot 5 : wikipedia.org   | 208.80.154.224  | 00:1A:2B:3C:4D:5E
 Slot 6 : youtube.com     | 142.250.72.14   | A4:5E:60:D3:2B:19
---------------------------------------------------

Enter SOURCE URL (example: google.com): www.google.com
Enter DESTINATION URL (example: youtube.com): www.mail.com

("www.google.com" was not in the table, so a new IP/MAC was created)

("www.mail.com" was not in the table, so a new IP/MAC was created)

----------------- ADDRESS RESOLUTION -----------------
SOURCE : www.google.com -> IP 14.133.79.174 -> MAC E9:2A:6E:03:85:9F
 IP binary (32 bits): 00001110100001010100111110101110
 MAC binary (48 bits): 111010010010101001101110000000111000010110011111
DESTINATION : www.mail.com -> IP 116.30.62.10 -> MAC C8:37:34:56:73:EC
 IP binary (32 bits): 01110100000111100011111000001010
 MAC binary (48 bits): 110010000011011100110100010101100111001111101100
-------------------------------------------------------

Enter the file name that contains the message (example: message.txt): message.txt
=============== APPLICATION LAYER ===============
Message read from message.txt: "41 10 42"

 '4' -> 00110100
 '1' -> 00110001
 ' ' -> 00100000
 '1' -> 00110001
 '0' -> 00110000
 ' ' -> 00100000
 '4' -> 00110100
 '2' -> 00110010

Stream : 0011010000110001001000000011000100110000001000000011010000110010
Total Bits : 64 bits
===================================================

=============== TRANSPORT LAYER ===============
Source Port : 7669 (binary: 0001110111110101)
Destination Port : 17855 (binary: 0100010110111111)
Stream : 000111011111010101000101101111110011010000110001001000000011000100110000001000000011010000110010
Total Bits : 96 bits
=================================================

=============== NETWORK LAYER ===============
Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Stream : 00001110100001010100111110101110011101000001111000111110000010100011010000110001001000000011000100110000001000000011010000110010
Total Bits : 128 bits

Total packets: 4
===============================================

=============== DATA LINK LAYER ===============
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Stream : 1110100100101010011011100000001110000101100111111100100000110111001101000101011001110011111011000000111010000101010011111010111001110100000111100011111000001010001101000011000100100000001100010011000000100000001101000011001000000000
Total Bits : 232 bits

Data : 0011010000110001001000000011000100110000001000000011010000110010
Src Port : 0001110111110101
Dest Port : 0100010110111111
Src IP : 00001110100001010100111110101110
Dest IP : 01110100000111100011111000001010
Src MAC : 111010010010101001101110000000111000010110011111
Dest MAC : 110010000011011100110100010101100111001111101100
Trailer : 00000000
Full Stream : 001101000011000100100000001100010011000000100000001101000011001000011101111101010100010110111111000011101000010101001111101011100111010000011110001111100000101011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000000000

=========================================================
 EXP1 + EXP2: BISYNC FRAMING WITH CRC & PARITY CHECK
=========================================================
The message above is now framed using BISYNC (SYN SYN SOH
...STX...ETX), with CRC + Even Parity error detection
added on top (Exp2).

----- BISYNC + CRC FRAMING : SENDER -----
Step 1: Data handed off from Data Link Layer
 Message : "41 10 42"
 Length : 8 bytes

Step 2: Dynamic CRC Polynomial Configuration
--------------------------------------------------------
 DYNAMIC CRC POLYNOMIAL CONFIGURATION 
--------------------------------------------------------
Enter highest degree of polynomial (1 to 7): 4
Enter coefficient for x^4 (must be 1): 1
Enter coefficient for x^3 (0 or 1): 0
Enter coefficient for x^2 (0 or 1): 1
Enter coefficient for x^1 (0 or 1): 0
Enter coefficient for x^0 (0 or 1): 1

 Polynomial Expression : G(x) = x^4 + x^2 + 1
 Divisor Binary Bitstring : 10101

Step 3: Character Stuffing (escape literal ETX/DLE/STX text)

 --- Starting Character Stuffing ---
 --- Stuffing complete! ---

Step 4: Convert Stuffed Payload to Pure Binary
 Stuffed Payload Binary: 0011010000110001001000000011000100110000001000000011010000110010

Step 5: Assemble Full BISYNC Frame (SYN SYN SOH H D R STX...ETX+Trailer)

 --- Step-by-Step CRC Long Division ---
 Augmented Binary Stream : 00110100001100010010000000110001001100000010000000110100001100100000
 Divisor Binary Bitstring: 10101
 Calculated CRC Remainder : 1101 (4 bits)

 >>> BINARY CHECKSUM GENERATION <<<
 Binary Payload Length : 64 bits
 Calculated Parity Bit : 1 (Even Parity)
 Calculated CRC Bits : 1101
 Trailer Byte : 10001101

Step 6: Clean Frame Before Transmission
 0001011000010110000000010100100001000100010100100000001000110100001100010010000000110001001100000010000000110100001100100000001110001101

Step 7: Simulate Transmission Channel (optional error injection)
 Do you want to simulate a transmission error? (y/n): y
 How many bit(s) do you want to corrupt? 2
 Enter bit position #1 to flip (0 to 135): 2
 >>> Bit #2 flipped: '0' -> '1'
 Enter bit position #2 to flip (0 to 135): 3
 >>> Bit #3 flipped: '1' -> '0'

Step 8: Final Transmitted Binary Frame Stream
 0010011000010110000000010100100001000100010100100000001000110100001100010010000000110001001100000010000000110100001100100000001110001101

Step 9: Write Divisor + Frame to Channel File
 [Sender]: Frame stream and polynomial divisor saved to 'bisync_output.txt'

----- BISYNC + CRC FRAMING : RECEIVER -----
Step 1: Read Divisor + Frame from Channel File
 Divisor Used : 10101
 Frame Stream : 0010011000010110000000010100100001000100010100100000001000110100001100010010000000110001001100000010000000110100001100100000001110001101

Step 2: Extract & Verify Frame (Header / STX / ETX / Trailer)
 [Error]: Invalid Binary Header Pattern (SYN SYN SOH error)!

=========================================================

Total frames: 8

====Each frame and its content======

---------------------------------------------
Packet No : 1
Frame No : 1
Source Port : 0001110111110101
Destination Port : 0100010110111111

Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Frame Data : 00110100
Trailer : 00000000
Frame Stream : 0000000011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000110100
---------------------------------------------
Packet No : 1
Frame No : 2
Source Port : 0001110111110101
Destination Port : 0100010110111111

Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Frame Data : 00110001
Trailer : 00000000
Frame Stream : 0000000011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000110001
---------------------------------------------
Packet No : 2
Frame No : 1
Source Port : 0001110111110101
Destination Port : 0100010110111111

Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Frame Data : 00100000
Trailer : 00000000
Frame Stream : 0000000011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000100000
---------------------------------------------
Packet No : 2
Frame No : 2
Source Port : 0001110111110101
Destination Port : 0100010110111111

Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Frame Data : 00110001
Trailer : 00000000
Frame Stream : 0000000011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000110001
---------------------------------------------
Packet No : 3
Frame No : 1
Source Port : 0001110111110101
Destination Port : 0100010110111111

Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Frame Data : 00110000
Trailer : 00000000
Frame Stream : 0000000011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000110000
---------------------------------------------
Packet No : 3
Frame No : 2
Source Port : 0001110111110101
Destination Port : 0100010110111111

Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Frame Data : 00100000
Trailer : 00000000
Frame Stream : 0000000011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000100000
---------------------------------------------
Packet No : 4
Frame No : 1
Source Port : 0001110111110101
Destination Port : 0100010110111111

Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Frame Data : 00110100
Trailer : 00000000
Frame Stream : 0000000011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000110100
---------------------------------------------
Packet No : 4
Frame No : 2
Source Port : 0001110111110101
Destination Port : 0100010110111111

Source IP : 00001110100001010100111110101110
Destination IP : 01110100000111100011111000001010
Source MAC : 111010010010101001101110000000111000010110011111
Destination MAC : 110010000011011100110100010101100111001111101100
Frame Data : 00110010
Trailer : 00000000
Frame Stream : 0000000011101001001010100110111000000011100001011001111111001000001101110011010001010110011100111110110000110010
---------------------------------------------

=============== SUMMARY ===============
Message : "41 10 42"
Application bits : 64
Transport layer bits : 96
Network layer bits : 128
Data Link layer bits : 232
Total packets : 4
Total frames : 8
[24bcs170@mepcolinux ex2]$exit
exit
