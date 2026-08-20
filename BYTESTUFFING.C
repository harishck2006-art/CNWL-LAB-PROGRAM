Bytestuffing.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE 10
#define F_SZ 8
#define PPP_FLAG 0x7E
#define PPP_ESC  0x7D
#define PPP_ADDR 0xFF
#define PPP_CTRL 0x03
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
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed!\n");
        return;
    }
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
void delete(char url[]) {
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
void printByteBinaryStdout(unsigned char n) {
    for (int i = 7; i >= 0; i--) printf("%d", (n >> i) & 1);
}
void writeByteBinary(FILE *fp, unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        int bit = (byte >> i) & 1;
        fprintf(fp, "%d", bit);
        printf("%d", bit);
    }
}
void printPortBinaryStdout(int port) {
    for (int i = 15; i >= 0; i--) printf("%d", (port >> i) & 1);
}
void printIPBinaryStdout(char ip[]) {
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        printByteBinaryStdout(a); printByteBinaryStdout(b); printByteBinaryStdout(c); printByteBinaryStdout(d);
    }
}
void printMACBinaryStdout(char mac[]) {
    unsigned int x[6];
    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (int i = 0; i < 6; i++) printByteBinaryStdout(x[i]);
    }
}
void showLayers(char msg[], int len, struct Node *dest) {
    printf("\n--- Network Layer Data ---");
    printf("\n========= TRANSPORT LAYER =========\n");
    printf("Source Port      : "); printPortBinaryStdout(srcPort); printf("\n");
    printf("Destination Port : "); printPortBinaryStdout(dest->port); printf("\n");
    printf("========= NETWORK LAYER =========\n\n");
    printf("Source IP      : "); printIPBinaryStdout(srcIP); printf("\n");
    printf("Destination IP : "); printIPBinaryStdout(dest->ip); printf("\n");
    printf("========= DATA LINK LAYER =========\n");
    printf("Source MAC     : "); printMACBinaryStdout(srcMAC); printf("\n");
    printf("Destination MAC: "); printMACBinaryStdout(dest->mac); printf("\n");
    printf("-----------------------------------\n");
}
void showFrames(char msg[], int len, int totalFrames, struct Node *dest) {
    printf("==== Frame Contents ====\n");
    for(int i = 0; i < totalFrames; i++) {
        printf("\n-----------------------------------------\n");
        int packetNo = (i / 2) + 1;
        printf("Packet No : %d\n", packetNo);
        printf("Frame No  : %d\n", i + 1);
        printf("Source Port      : "); printPortBinaryStdout(srcPort); printf("\n");
        printf("Destination Port : "); printPortBinaryStdout(dest->port); printf("\n\n");
        printf("Source IP      : "); printIPBinaryStdout(srcIP); printf("\n");
        printf("Destination IP : "); printIPBinaryStdout(dest->ip); printf("\n");
        printf("Source MAC     : "); printMACBinaryStdout(srcMAC); printf("\n");
        printf("Destination MAC: "); printMACBinaryStdout(dest->mac); printf("\n");
        printf("Frame Data     : ");
        for(int j = 0; j < F_SZ; j++) {
            int cur = (i * F_SZ) + j;
            if(cur < len) { printByteBinaryStdout(msg[cur]); printf(" "); }
            else { printByteBinaryStdout(0); printf(" "); }
        }
        printf("\nTail           : 00000000\n");
        printf("-----------------------------------------\n");
    }
}
void sender_PPP(unsigned char *data, int len) {
    FILE *fp = fopen("transmitted_ppp.txt", "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not open 'transmitted_ppp.txt' for writing.\n");
        return;
    }
    printf("\n========= SENDER PPP OUTPUT () =========\n");
    writeByteBinary(fp, PPP_FLAG); printf("\n");
    writeByteBinary(fp, PPP_ADDR);
    writeByteBinary(fp, PPP_CTRL); printf("\n");
    writeByteBinary(fp, 0x00);
    writeByteBinary(fp, 0x21);
    printf("\n");
    for (int i = 0; i < len; i++) {
        unsigned char current_byte = data[i];
        if (current_byte == PPP_FLAG || current_byte == PPP_ESC) {
            writeByteBinary(fp, PPP_ESC);
        }
        writeByteBinary(fp, current_byte);
    }
    printf("\n");
    writeByteBinary(fp, 0x00);
    printf("\n");
    writeByteBinary(fp, PPP_FLAG);
    printf("\n");
    fclose(fp);
    printf("=========================================================\n");
    printf("[Sender] Full structured binary PPP Frame saved to 'transmitted_ppp.txt'\n");
}
unsigned char binStringToByte(const char* bin_str, int start_idx, int len) {
    unsigned char byte = 0;
    for (int i = 0; i < len; i++) {
        if (bin_str[start_idx + i] == '1') {
            byte |= (1 << (len - 1 - i));
        }
    }
    return byte;
}
void receiver_PPP() {
    FILE *fp = fopen("transmitted_ppp.txt", "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open 'transmitted_ppp.txt' for reading.\n");
        return;
    }
    printf("\n========= RECEIVER PPP OUTPUT () =========\n");
    char *stream = (char *)malloc(64000);
    if (!stream) {
        fprintf(stderr, "Memory allocation failed for receiver stream buffer.\n");
        fclose(fp);
        return;
    }
    int stream_idx = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF && stream_idx < 63999) {
        if (ch == '0' || ch == '1') {
            stream[stream_idx++] = (char)ch;
        }
    }
    stream[stream_idx] = '\0';
    fclose(fp);
    int len = strlen(stream);
    if (len == 0) {
        printf("Error: 'transmitted_ppp.txt' is empty or unreadable.\n");
        free(stream);
        return;
    }
    char start_flag_bin[9] = "01111110";
    char end_flag_bin[9] = "01111110";
    char temp_start[9], temp_end[9];
    if (len < 56) {
        printf("Error: Invalid Frame - too short!\n");
        free(stream);
        return;
    }
    strncpy(temp_start, stream, 8); temp_start[8] = '\0';
    strncpy(temp_end, stream + len - 8, 8); temp_end[8] = '\0';
    if (strcmp(temp_start, start_flag_bin) != 0 || strcmp(temp_end, end_flag_bin) != 0) {
        printf("Error: Invalid Frame - missing or incorrect start/end flags!\n");
        free(stream);
        return;
    }
    int payload_start_bit_idx = 40;
    int payload_end_bit_idx = len - 8;
    if (payload_start_bit_idx >= payload_end_bit_idx) {
        printf("No payload data found between frame fields.\n");
        free(stream);
        return;
    }
    int destuffed_bits_count = 0;
    int skip_next_byte_bits = 0;
    for (int bit_stream_idx = payload_start_bit_idx; bit_stream_idx < payload_end_bit_idx; ) {
        unsigned char reconstructed_byte = 0;
        int bits_read_for_byte = 0;
        for (int bit_pos = 0; bit_pos < 8 && bit_stream_idx + bit_pos < payload_end_bit_idx; bit_pos++) {
            if (stream[bit_stream_idx + bit_pos] == '1') {
                reconstructed_byte |= (1 << (7 - bit_pos));
            }
            bits_read_for_byte++;
        }
        bit_stream_idx += 8;
        if (bits_read_for_byte < 8) break;
        if (skip_next_byte_bits) {
            printByteBinaryStdout(reconstructed_byte);
            destuffed_bits_count += 8;
            if (destuffed_bits_count % 8 == 0) printf(" ");
            skip_next_byte_bits = 0;
        } else {
            if (reconstructed_byte == PPP_ESC) {
                skip_next_byte_bits = 1;
            } else {
                printByteBinaryStdout(reconstructed_byte);
                destuffed_bits_count += 8;
                if (destuffed_bits_count % 8 == 0) printf(" ");
            }
        }
    }
    printf("\n");
    printf("=======================================================\n");
    printf("[Receiver] Destuffed binary stream processed from 'transmitted_ppp.txt'\n");
    free(stream);
}
int main() {
    char fn[50], url[100], msg[1000] = "";
    FILE *fp;
    int ch, idx = 0, m, cho;
    preload();
    while(1) {
        URLTable();
        printf("\n========= MAIN MENU (PPP PROTOCOL RUNNER) =========\n");
        printf("1. Hash Table Management\n");
        printf("2. Proceed to Data Framing & Run PPP\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &m) != 1) {
             while (getchar() != '\n');
             printf("Invalid input. Please enter a number (1, 2, or 3).\n");
             continue;
        }
        if (m == 1) {
            printf("\n--- Hash Table Functions ---\n");
            printf("1. Add URL Entry\n");
            printf("2. Delete URL Entry\n");
            printf("3. Back to Main Menu\n");
            printf("Enter your choice: ");
            if(scanf("%d", &cho) != 1) {
                 while (getchar() != '\n');
                 printf("Invalid input. Please enter a number.\n");
                 continue;
            }
            if (cho == 1) {
                char newUrl[50], newIp[20], newMac[20]; int newPort;
                printf("Enter URL: "); scanf("%49s", newUrl);
                printf("Enter IP: "); scanf("%19s", newIp);
                printf("Enter MAC: "); scanf("%19s", newMac);
                printf("Enter Port: ");
                if(scanf("%d", &newPort) != 1) {
                    while (getchar() != '\n');
                    printf("Invalid input for port. Please enter a number.\n");
                    continue;
                }
                insert(newUrl, newIp, newMac, newPort);
            } else if (cho == 2) {
                char delUrl[50];
                printf("Enter URL to delete: "); scanf("%49s", delUrl);
                delete(delUrl);
            } else if (cho == 3) {
            } else {
                printf("Invalid choice for hash table functions.\n");
            }
        } else if (m == 2) {
            break;
        } else if (m == 3) {
            printf("Exiting program.\n");
            return 0;
        } else {
            printf("Invalid choice. Please enter 1, 2, or 3.\n");
        }
    }
    printf("\n--- PPP Framing Simulation ---");
    printf("\nEnter Source URL from table: ");
    scanf("%99s", url);
    struct Node *srcNode = search(url);
    if(srcNode) {
        strcpy(srcURL, srcNode->url);
        strcpy(srcIP, srcNode->ip);
        strcpy(srcMAC, srcNode->mac);
        srcPort = srcNode->port;
    } else {
        printf("Source URL not found in table. Using default source values.\n");
        strcpy(srcURL, "Default Source");
        strcpy(srcIP, "192.168.1.10");
        strcpy(srcMAC, "11:22:33:44:55:66");
        srcPort = 51309;
    }
    printf("Enter Destination URL: ");
    scanf("%99s", url);
    struct Node *dest = search(url);
    if(!dest) {
        printf("Destination URL not found in table. Exiting.\n");
        return 1;
    }
    printf("Enter File Name to transmit: ");
    scanf("%49s", fn);
    fp = fopen(fn, "r");
    if(!fp) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", fn);
        return 1;
    }
    while((ch = fgetc(fp)) != EOF && idx < 999) {
        msg[idx++] = (char)ch;
    }
    msg[idx] = '\0';
    fclose(fp);
    int len = strlen(msg);
    int totalFrames = len / F_SZ + (len % F_SZ != 0);
    if (len == 0) totalFrames = 1;
    showLayers(msg, len, dest);
    showFrames(msg, len, totalFrames, dest);
    sender_PPP((unsigned char*)msg, len);
    receiver_PPP();
    return 0;
}


PS C:\Users\hp> cd "C:\Users\hp\OneDrive\Desktop"
PS C:\Users\hp\OneDrive\Desktop> gcc P1.c -o P1.exe               
PS C:\Users\hp\OneDrive\Desktop> ./P1.exe

================ URL TABLE ================
URL                  IP                 MAC                 PORT 
www.whatsapp.com     142.250.190.46     AA:BB:CC:DD:EE:02   443  
www.facebook.com     157.240.22.35      AA:BB:CC:DD:EE:03   80   
www.google.com       142.250.190.47     AA:BB:CC:DD:EE:04   443  
www.mail.com         142.250.183.14     AA:BB:CC:DD:EE:01   25   
===========================================

========= MAIN MENU (PPP PROTOCOL RUNNER) =========
1. Hash Table Management
2. Proceed to Data Framing & Run PPP
3. Exit
Enter your choice: 2

--- PPP Framing Simulation ---
Enter Source URL from table: www.google.com
Enter Destination URL: www.mail.com
Enter File Name to transmit: message.txt

--- Network Layer Data ---
========= TRANSPORT LAYER =========
Source Port      : 0000000110111011
Destination Port : 0000000000011001
========= NETWORK LAYER =========

Source IP      : 10001110111110101011111000101111
Destination IP : 10001110111110101011011100001110
========= DATA LINK LAYER =========
Source MAC     : 101010101011101111001100110111011110111000000100
Destination MAC: 101010101011101111001100110111011110111000000001
-----------------------------------
==== Frame Contents ====

-----------------------------------------
Packet No : 1
Frame No  : 1
Source Port      : 0000000110111011
Destination Port : 0000000000011001

Source IP      : 10001110111110101011111000101111
Destination IP : 10001110111110101011011100001110
Source MAC     : 101010101011101111001100110111011110111000000100
Destination MAC: 101010101011101111001100110111011110111000000001
Frame Data     : 01101000 01101001 01111110 00000000 00000000 00000000 00000000 00000000 
Tail           : 00000000
-----------------------------------------

========= SENDER PPP OUTPUT () =========
01111110
1111111100000011
0000000000100001
01101000011010010111110101111110
00000000
01111110
=========================================================
[Sender] Full structured binary PPP Frame saved to 'transmitted_ppp.txt'

========= RECEIVER PPP OUTPUT () =========
01101000 01101001 01111110 00000000 
=======================================================
[Receiver] Destuffed binary stream processed from 'transmitted_ppp.txt'


