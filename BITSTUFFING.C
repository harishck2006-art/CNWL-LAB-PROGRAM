Bitstuffing.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE 10
#define F_SZ 8
struct Node {
    char url[50], ip[20], mac[20];
    int port;
    struct Node *next;
};
struct Node *table[SIZE] = {NULL};
char srcURL[50] = "Default Source";
char srcIP[20] = "192.168.1.10";
char srcMAC[20] = "11:22:33:44:55:66";
int srcPort = 51309;
int hash(char url[]) {
    int sum = 0;
    for(int i = 0; url[i]; i++) sum += url[i];
    return sum % SIZE;
}
void insert(char url[], char ip[], char mac[], int port) {
    int idx = hash(url);
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    strcpy(newNode->url, url);
    strcpy(newNode->ip, ip);
    strcpy(newNode->mac, mac);
    newNode->port = port;
    newNode->next = table[idx];
    table[idx] = newNode;
}
struct Node* search(char url[]) {
    struct Node *tmp = table[hash(url)];
    while(tmp) {
        if(strcmp(tmp->url, url) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}
void deleteURL(char url[]) {
    int idx = hash(url);
    struct Node *tmp = table[idx];
    struct Node *prev = NULL;
    while (tmp != NULL && strcmp(tmp->url, url) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL) return;
    if (prev == NULL) {
        table[idx] = tmp->next;
    } else {
        prev->next = tmp->next;
    }
    free(tmp);
}
void URLTable() {
    printf("\n================ URL TABLE ================\n");
    printf("%-20s %-18s %-19s %-5s\n", "URL", "IP", "MAC", "PORT");
    for(int i = 0; i < SIZE; i++) {
        struct Node *tmp = table[i];
        while(tmp) {
            printf("%-20s %-18s %-19s %-5d\n", tmp->url, tmp->ip, tmp->mac, tmp->port);
            tmp = tmp->next;
        }
    }
    printf("===========================================\n");
}
void preload() {
    insert("www.mail.com", "142.250.183.14", "AA:BB:CC:DD:EE:01", 25);
    insert("www.whatsapp.com", "142.250.190.46", "AA:BB:CC:DD:EE:02", 443);
    insert("www.facebook.com", "157.240.22.35", "AA:BB:CC:DD:EE:03", 80);
    insert("www.google.com", "142.250.190.47", "AA:BB:CC:DD:EE:04", 443);
}
void printByte(unsigned char n) {
    for (int i = 7; i >= 0; i--) printf("%d", (n >> i) & 1);
}
void printPort(int port) {
    for (int i = 15; i >= 0; i--) printf("%d", (port >> i) & 1);
}
void printIP(char ip[]) {
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        printByte(a); printByte(b); printByte(c); printByte(d);
    }
}
void printMAC(char mac[]) {
    unsigned int x[6];
    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (int i = 0; i < 6; i++) printByte(x[i]);
    }
}
void showLayers(char msg[], int len, struct Node *dest) {
    printf("\nMessage :\n");
    for(int i = 0; i < len; i++) printByte(msg[i]);
    printf("\n\n");
    printf("========= TRANSPORT LAYER =========\n");
    printf("Source Port      : "); printPort(srcPort); printf("\n");
    printf("Destination Port : "); printPort(dest->port); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) printByte(msg[i]);
    printPort(srcPort); printPort(dest->port);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32);
    printf("========= Network Layer =========\n\n");
    printf("Source IP      : "); printIP(srcIP); printf("\n");
    printf("Destination IP : "); printIP(dest->ip); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) printByte(msg[i]);
    printPort(srcPort); printPort(dest->port);
    printIP(srcIP); printIP(dest->ip);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64);
    printf("========= Data Link Layer =========\n");
    printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
    printf("Destination MAC: "); printMAC(dest->mac); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) printByte(msg[i]);
    printPort(srcPort); printPort(dest->port);
    printIP(srcIP); printIP(dest->ip);
    printMAC(srcMAC); printMAC(dest->mac);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64 + 96);
}
void showFrames(char msg[], int len, int totalFrames, struct Node *dest) {
    printf("====Frame Contents======\n");
    for(int i = 0; i < totalFrames; i++) {
        printf("\n-----------------------------------------\n");
        int packetNo = (i / 2) + 1;
        printf("Packet No : %d\n", packetNo);
        printf("Frame No  : %d\n", i + 1);
        printf("Source Port      : "); printPort(srcPort); printf("\n");
        printf("Destination Port : "); printPort(dest->port); printf("\n\n");
        printf("Source IP      : "); printIP(srcIP); printf("\n");
        printf("Destination IP : "); printIP(dest->ip); printf("\n");
        printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
        printf("Destination MAC: "); printMAC(dest->mac); printf("\n");
        printf("Frame Data     : ");
        for(int j = 0; j < F_SZ; j++) {
            int cur = (i * F_SZ) + j;
            if(cur < len) printByte(msg[cur]);
            else printByte(0);
        }
        printf("\nTail           : 00000000\n");
        printf("-----------------------------------------\n");
    }
}
void writeBit(FILE *fp, int bit) {
    fprintf(fp, "%d", bit);
    printf("%d", bit);
}
void writeByte(FILE *fp, unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        int bit = (byte >> i) & 1;
        fprintf(fp, "%d", bit);
        printf("%d", bit);
    }
}
void senderBit(unsigned char *data, int len) {
    FILE *fp = fopen("transmitted_bits.txt", "w");
    if (!fp) return;
    printf("\n========= SENDER INTERMEDIATE PROCESSING (BIT FRAMING) =========\n");
    printf("-> Start Flag     : ");
    writeByte(fp, 0x7E);
    printf("\n");
    printf("-> Address        : ");
    writeByte(fp, 0xFF);
    printf("\n");
    printf("-> Control        : ");
    writeByte(fp, 0x03);
    printf("\n");
    printf("-> Protocol       : ");
    writeByte(fp, 0x00);
    writeByte(fp, 0x21);
    printf("\n");
    printf("\nOriginal Payload (Binary):\n");
    for (int i = 0; i < len; i++) {
        for (int bit = 7; bit >= 0; bit--)
            printf("%d", (data[i] >> bit) & 1);
        printf(" ");
    }
    printf("\n\n-> Payload (Bit-Stuffed):\n   [ ");
    int co = 0;
    for (int i = 0; i < len; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            int cb = (data[i] >> bit) & 1;
            fprintf(fp, "%d", cb);
            printf("%d", cb);
            if (cb == 1) {
                co++;
                if (co == 5) {
                    fprintf(fp, "0");
                    printf("[0]");
                    co = 0;
                }
            } else {
                co = 0;
            }
        }
    }
    printf(" ]\n");
    printf("-> Checksum       : ");
    writeByte(fp, 0x00);
    printf("\n");
    printf("-> End Flag       : ");
    writeByte(fp, 0x7E);
    printf("\n");
    printf("================================================================\n");
    fclose(fp);
    printf("[Sender] Frame saved to 'transmitted_bits.txt'\n");
}
void receiverBit() {
    FILE *fp = fopen("transmitted_bits.txt", "r");
    if (!fp) return;
    printf("\n========= RECEIVER SIDE (BIT DESTUFFING) =========\n");
    static char stream[32000];
    if (fscanf(fp, "%31999s", stream) != 1) {
        fclose(fp);
        return;
    }
    fclose(fp);
    int len = strlen(stream);
    if (len < 56) {
        printf("Error: Invalid Frame!\n");
        return;
    }
    printf("-> Start Flag  : ");
    for(int i = 0; i < 8; i++) printf("%c", stream[i]);
    printf("\n");
    printf("-> Address     : ");
    for(int i = 8; i < 16; i++) printf("%c", stream[i]);
    printf("\n");
    printf("-> Control     : ");
    for(int i = 16; i < 24; i++) printf("%c", stream[i]);
    printf("\n");
    printf("-> Protocol    : ");
    for(int i = 24; i < 40; i++) printf("%c", stream[i]);
    printf("\n");
    printf("-> Payload (Destuffed):\n   ");
    int sIdx = 40;
    int eIdx = len - 16;
    int co = 0, bc = 0;
    for (int i = sIdx; i < eIdx; i++) {
        int bit = stream[i] - '0';
        if (bit == 1) {
            printf("1");
            co++;
            bc++;
        } else {
            if (co == 5) {
                co = 0;
                continue;
            }
            printf("0");
            co = 0;
            bc++;
        }
        if (bc == 8) {
            printf(" ");
            bc = 0;
        }
    }
    printf("\n-> Checksum    : ");
    for(int i = len - 16; i < len - 8; i++) printf("%c", stream[i]);
    printf("\n");
    printf("-> End Flag    : ");
    for(int i = len - 8; i < len; i++) printf("%c", stream[i]);
    printf("\n");
    printf("==================================================\n");
}
int main() {
    char fn[50], url[100], msg[1000] = "";
    FILE *fp;
    int ch, idx = 0, m, cho;
    preload();
    while(1) {
        URLTable();
        printf("\n========= MAIN MENU (BIT STUFFING RUNNER) =========\n");
        printf("1. Hash Table Management\n");
        printf("2. Proceed to Data Framing & Run Bit Stuffing\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &m) != 1) return 0;
        if (m == 1) {
            printf("\n--- Hash Table Functions ---\n");
            printf("1. Add URL Entry\n");
            printf("2. Delete URL Entry\n");
            printf("3. Back\n");
            if(scanf("%d", &cho) != 1) continue;
            if (cho == 1) {
                char newUrl[50], newIp[20], newMac[20]; int newPort;
                printf("Enter URL: "); scanf("%49s", newUrl);
                printf("Enter IP: "); scanf("%19s", newIp);
                printf("Enter MAC: "); scanf("%19s", newMac);
                printf("Enter Port: "); scanf("%d", &newPort);
                insert(newUrl, newIp, newMac, newPort);
            } else if (cho == 2) {
                char delUrl[50];
                printf("Enter URL to delete: "); scanf("%49s", delUrl);
                deleteURL(delUrl);
            }
        } else if (m == 2) {
            break;
        } else {
            return 0;
        }
    }
    printf("\nEnter Source URL from table: ");
    scanf("%99s", url);
    struct Node *srcNode = search(url);
    if(srcNode) {
        strcpy(srcURL, srcNode->url);
        strcpy(srcIP, srcNode->ip);
        strcpy(srcMAC, srcNode->mac);
        srcPort = srcNode->port;
    }
    printf("Enter Destination URL: ");
    scanf("%99s", url);
    struct Node *dest = search(url);
    if(!dest) return 0;
    printf("Enter File Name: ");
    scanf("%49s", fn);
    fp = fopen(fn, "r");
    if(!fp) return 0;
    while((ch = fgetc(fp)) != EOF && idx < 999) msg[idx++] = (char)ch;
    msg[idx] = '\0';
    fclose(fp);
    int len = strlen(msg);
    int totalFrames = len / F_SZ + (len % F_SZ != 0);
    showLayers(msg, len, dest);
    showFrames(msg, len, totalFrames, dest);
    senderBit((unsigned char*)msg, len);
    receiverBit();
    return 0;
}





PS C:\Users\hp\OneDrive\Desktop> ./P2.exe           

================ URL TABLE ================
URL                  IP                 MAC                 PORT 
www.whatsapp.com     142.250.190.46     AA:BB:CC:DD:EE:02   443  
www.facebook.com     157.240.22.35      AA:BB:CC:DD:EE:03   80   
www.google.com       142.250.190.47     AA:BB:CC:DD:EE:04   443  
www.mail.com         142.250.183.14     AA:BB:CC:DD:EE:01   25   
===========================================

========= MAIN MENU (BIT STUFFING RUNNER) =========
1. Hash Table Management
2. Proceed to Data Framing & Run Bit Stuffing
3. Exit
Enter your choice: 2

Enter Source URL from table: www.google.com
Enter Destination URL: www.mail.com
Enter File Name: input.txt

Message :
01101000011010010111111000001010

========= TRANSPORT LAYER =========
Source Port      : 0000000110111011
Destination Port : 0000000000011001
Stream           : 0110100001101001011111100000101000000001101110110000000000011001
Total Bits       : 64 bits

========= Network Layer =========

Source IP      : 10001110111110101011111000101111
Destination IP : 10001110111110101011011100001110
Stream           : 01101000011010010111111000001010000000011011101100000000000110011000111011111010101111100010111110001110111110101011011100001110
Total Bits       : 128 bits

========= Data Link Layer =========
Source MAC     : 101010101011101111001100110111011110111000000100
Destination MAC: 101010101011101111001100110111011110111000000001
Stream           : 01101000011010010111111000001010000000011011101100000000000110011000111011111010101111100010111110001110111110101011011100001110101010101011101111001100110111011110111000000100101010101011101111001100110111011110111000000001
Total Bits       : 224 bits

====Frame Contents======

-----------------------------------------
Packet No : 1
Frame No  : 1
Source Port      : 0000000110111011
Destination Port : 0000000000011001

Source IP      : 10001110111110101011111000101111
Destination IP : 10001110111110101011011100001110
Source MAC     : 101010101011101111001100110111011110111000000100
Destination MAC: 101010101011101111001100110111011110111000000001
Frame Data     : 0110100001101001011111100000101000000000000000000000000000000000
Tail           : 00000000
-----------------------------------------

========= SENDER INTERMEDIATE PROCESSING (BIT FRAMING) =========
-> Start Flag     : 01111110
-> Address        : 11111111
-> Control        : 00000011
-> Protocol       : 0000000000100001

Original Payload (Binary):
01101000 01101001 01111110 00001010 

-> Payload (Bit-Stuffed):
   [ 0110100001101001011111[0]1000001010 ]
-> Checksum       : 00000000
-> End Flag       : 01111110
================================================================
[Sender] Frame saved to 'transmitted_bits.txt'

========= RECEIVER SIDE (BIT DESTUFFING) =========
-> Start Flag  : 01111110
-> Address     : 11111111
-> Control     : 00000011
-> Protocol    : 0000000000100001
-> Payload (Destuffed):
   01101000 01101001 01111110 00001010 
-> Checksum    : 00000000
-> End Flag    : 01111110
===============================================
