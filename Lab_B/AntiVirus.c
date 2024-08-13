#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus {
    unsigned short SigSize; // Signature size
    char virusName[16]; // Virus name
    unsigned char* sig; // Virus signature
} virus;

typedef struct link {
    struct link *nextVirus; // Pointer to next virus in list
    virus *vir; // Pointer to virus data
} link;

char defSigFileName[256] = "signatures-L"; // Default signature file name
char suspectedFileName[256];
link* global_virus_list = NULL; // Initialize the virus list as empty
int isLittleEndian = 1; // Assume little endian by default

// Function to print the buffer in hexadecimal format
void printHex(unsigned char* buffer, size_t length) {
    for(size_t i = 0; i < length; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}

unsigned short calculateSigSize(unsigned char firstByte, unsigned char secondByte, int isLittle) {
    if (isLittle) {
        return (secondByte << 8) | firstByte;
    } else {
        return (firstByte << 8) | secondByte;
    }
}

virus *readVirus(FILE *file) {
    unsigned char buffer[1024];
    int readBytes = fread(buffer, 1, 18, file);
    if (readBytes <= 0) {
        return NULL; // End of file
    }
    virus *v = (virus *)malloc(sizeof(virus));
    if (!v) {
        printf("Failed to allocate memory for virus");
        return NULL;
    }
    v->SigSize = calculateSigSize(buffer[0], buffer[1], isLittleEndian);
    memcpy(v->virusName, buffer + 2, 16); // Copy the virus name
    v->virusName[15] = '\0'; // Ensure null-termination

    v->sig = (unsigned char *)malloc(v->SigSize);
    if (!v->sig) {
        printf("Failed to allocate memory for virus signature");
        free(v);
        return NULL;
    }
    readBytes = fread(v->sig, 1, v->SigSize, file); // Read the virus signature
    if (readBytes != v->SigSize) {
        printf("Failed to read virus signature");
        free(v->sig);
        free(v);
        return NULL;
    }
    return v;
}

// Print the virus details
void printVirus(virus* v) {
    printf("Virus name: %s\n", v->virusName);
    printf("Virus signature length: %d\n", v->SigSize);
    printf("Virus signature: ");
    printHex(v->sig, v->SigSize);
    printf("\n");
}

// Print the list of viruses
void list_print(link *virus_list, FILE* output) {
    while (virus_list != NULL) {
        printVirus (virus_list->vir);
        virus_list = virus_list->nextVirus;
    }
}

// Append a virus to the list
link* list_append(link* virus_list, virus* data) {
    link* new_link = (link*)malloc(sizeof(link));
    if (!new_link){
        printf("Failed to allocate memory for new link");
        return virus_list;
    }
    new_link->vir = data;
    new_link->nextVirus = virus_list;
    return new_link;
}

// Free the list of viruses
void list_free(link *virus_list) {
    while (virus_list != NULL) {
        link* next = virus_list->nextVirus;
        free(virus_list->vir->sig);
        free(virus_list->vir);
        free(virus_list);
        virus_list = next;
    }
}

// Menu function declarations
void SetSigFileName();
void load_signatures();
void print_signatures();
void detect_viruses();
void fix_file();
void quit();

struct fun_desc {
    char *name;
    void (*fun)();
};

struct fun_desc menu[] = {
    {"Set signatures file name", SetSigFileName},
    {"Load signatures", load_signatures},
    {"Print signatures", print_signatures},
    {"Detect viruses", detect_viruses},
    {"Fix file", fix_file},
    {"Quit", quit},
    {NULL, NULL}
};

int main(int argc, char **argv) {
    int menu_size = sizeof(menu) / sizeof(menu[0]) - 1;
    if (argc > 1)
        strncpy(suspectedFileName, argv[1], sizeof(suspectedFileName) - 1); // -1 to remove the newline character
    else {
        printf("No suspected file name provided\n");
        return 1;
    }

    while(1) {
        // Initialize carray to be an empty string.
        printf("Select operation from the following menu:\n");
        for(int i = 0; menu[i].name != NULL; i++)
            printf("Option %d: %s\n",i, menu[i].name);
        char input[10];
        printf("Option: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        int option = atoi(input);
        if (option >= 0 && option < menu_size) {
            printf("\nWithin bounds\n");
            menu[option].fun();
        } else {
            printf("\nNot within bounds\n");
            break;
        }
        printf("DONE.\n\n");
    }
    list_free(global_virus_list);
    global_virus_list = NULL;
    return 0;
}

// Set the signature file name to user input
void SetSigFileName() {
    printf("Enter new signature file name: ");
    if(fgets(defSigFileName, sizeof(defSigFileName), stdin) == NULL) {
        printf("Failed to read signature file name");
        return;
    }
    // Remove the newline character from the end of the string
    int strlength = strlen(defSigFileName);
    if (strlength > 0 && defSigFileName[strlength - 1] == '\n') {
        defSigFileName[strlength - 1] = '\0';
    }
}

// Load virus signatures from default file
void load_signatures() {
    // Open the signature file for reading in binary mode (rb)
    FILE* file = fopen(defSigFileName, "rb");
    if (!file) {
        printf("Error opening signature file");
        return;
    }

    char magic[4];
    // Check the magic number, which should be "VIRL" or "VIRB" for little or big endian:
    if (fread(magic, 1, 4, file) != 4) {
        printf("Error reading magic number");
        fclose(file);
        return;
    }
    // Check if the magic number is valid and set the endian accordingly
    if (memcmp(magic, "VIRL", 4) == 0) {
        isLittleEndian = 1;
    } else if (memcmp(magic, "VIRB", 4) == 0) {
        isLittleEndian = 0;
    } else {
        printf("Invalid magic number");
        fclose(file);
        return;
    }

    virus* v;
    // Read the virus signatures from the file and append them to the global virus list
    while ((v = readVirus(file)) != NULL) 
        global_virus_list = list_append(global_virus_list, v);
    fclose(file);
}

// Print the loaded virus signatures
void print_signatures() {
    list_print(global_virus_list, stdout);
}

// Detect viruses in a buffer
void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    link *current_link = virus_list;
    while (current_link != NULL) {
        virus *v = current_link->vir;
        for (unsigned int i = 0; i <= (size - v->SigSize); i++) {
            if (memcmp(buffer + i, v->sig, v->SigSize) == 0) {
                printf("Virus detected!\n");
                printf("Starting byte location: %u\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Virus signature size: %d\n", v->SigSize);
            }
        }
        current_link = current_link->nextVirus;
    }

}

// Detect viruses in a file
void detect_viruses() {
    FILE *file = fopen(suspectedFileName, "rb");
    if (!file) {
        printf("Error opening suspected file");
        return;
    }

    unsigned int bufferSize = 10 * 1024;
    char *buffer = (char *)malloc(bufferSize);
    if(!buffer){
        printf("Faileed to allocate buffer");
        fclose(file);
        return;
    }

    unsigned int bytesRead = fread(buffer, 1, bufferSize, file);
    if (ferror(file)){
        printf("Error reading suspected file");
        free(buffer);
        fclose(file);
        return;
    }

    detect_virus(buffer, bytesRead, global_virus_list);

    free(buffer);
    fclose(file);
}

// Neutralize a virus by writing a RET instruction
void neutralize_virus(char *fileName, int signatureOffset) {
    FILE *file = fopen(fileName, "r+b");
    if (!file || fseek(file, signatureOffset, SEEK_SET) != 0) {
        printf("Error opening file or seeking to offset");
        fclose(file);
        return;
    }
    unsigned char retInstruction = 0xC3;
    if (fwrite(&retInstruction, 1, 1, file) != 1)
        printf("Error writing RET instruction to file");

    fclose(file);
}

// Fix a file by neutralizing detected viruses
void fix_file() {
    FILE *file = fopen(suspectedFileName, "rb");
    if (!file) {
        printf("Error opening suspected file");
        return;
    }
    unsigned int bufferSize = 10 * 1024;
    char *buffer = (char *)malloc(bufferSize);
    if (!buffer) {
        printf("Failed to allocate buffer");
        fclose(file);
        return;
    }
    unsigned int bytesRead = fread(buffer, 1, bufferSize, file);
    if (ferror(file)) {
        printf("Error reading suspected file");
        free(buffer);
        fclose(file);
        return;
    }
    link *current_link = global_virus_list;
    while (current_link != NULL) {
        virus *v = current_link->vir;
        for(unsigned int i = 0; i <= bytesRead - v->SigSize; i++) {
            if (memcmp(buffer + i, v->sig, v->SigSize) == 0) {
                printf("Neutralizing virus at byte location: %u\n", i);
                neutralize_virus(suspectedFileName, i);
            }
        }
        current_link = current_link->nextVirus;
    }

    free(buffer);
    fclose(file);
}

void quit() {
    list_free(global_virus_list);
    exit(0);
}
