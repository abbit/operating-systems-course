#include <stdio.h>
#include <unistd.h>

void printID()
{
    printf("real user ID: %d\n", getuid());
    printf("effective user ID: %d\n", geteuid());
}
void fileCheck(FILE *file)
{
    if (file != NULL) {
        fclose(file);
    }
    else {
        perror("ERROR");
    }
}


int main(int argc, char *argv[])
{
    FILE *file = argc > 1 ? fopen(argv[1], "r") : fopen("file", "r");
    printID();
    fileCheck(file);
    setuid(geteuid());
    file = argc > 1 ? fopen(argv[1], "r") : fopen("file", "r");
    printID();
    fileCheck(file);

    return 0;
}