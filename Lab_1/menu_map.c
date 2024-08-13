#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char my_get(char c);
char cprt(char c);
char encrypt(char c);
char decrypt(char c);
char xoprt(char c);

struct fun_desc {
    char *name;
    char(*fun)(char);
};

char* map(char *array, int array_length, char (*f) (char)){
    char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
    // Task 2-B:
    for(int i = 0; i < array_length; i++)
        mapped_array[i] = f(array[i]);

    return mapped_array;
}
 

int main(int argc, char **argv) {
    struct fun_desc menu[] = {
        {"Get String", my_get},
        {"Print String", cprt},
        {"Encrypt", encrypt},
        {"Decrypt", decrypt},
        {"Print Hex", xoprt},
        {NULL, NULL}
    };
    int base_len = 5;
    char *carray = (char*)malloc(base_len * sizeof(char));
    for (int i = 0; i < base_len; i++)
        carray[i] = '\0';
    int menu_size = sizeof(menu) / sizeof(menu[0]) - 1;

    while(1) {
        // Initialize carray to be an empty string.

        printf("Select operation from the following menu:\n");
        for(int i = 0; menu[i].name != NULL; i++)
            printf("Option %d: %s\n",i, menu[i].name);
        char input[10];
        printf("Option: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // EOF
        }

        int option = atoi(input);
        // printf("Option: %d\n\n",option);
        if (option >= 0 && option < menu_size) {
            printf("\nWithin bounds\n");
            carray = map(carray, base_len, menu[option].fun);
        } else {
            printf("\nNot within bounds\n");
            break;
        }
        printf("DONE.\n\n");
    }
    free(carray);
}

char my_get(char c) {
    return fgetc(stdin);
}

char cprt(char c) {
    if ((char)0x20 <= c && c <= (char)0x7E)
        printf("%c\n", c);
    else
        printf(".\n");
    return c;
}

char encrypt(char c) {
    if((char)0x20 <= c && c <= (char)0x4E)
        c = c + 0x20;
    return c;
}

char decrypt(char c) {
    if(0x40 <= c && c <= 0x7E)
        c = c - 0x20;
    return c;
}

char xoprt(char c) {
    printf("%x %o\n", c, c); // Desclaimer: The convertion was provided by StackOverflow
    return c;
}



