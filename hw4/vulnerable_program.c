#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PASSWORD "secret"
#define LEN 6

int check_password(const char *input) {
    for (int i = 0; i < strlen(PASSWORD); i++) {
        if (input[i] != PASSWORD[i]) {
            return 0;
        }
        usleep(10000);
    }
    return 1;
}

int main() {
    char input[LEN+2];  // null terminator and \n

    printf("Enter the password (6 lowercase letters): ");
    fgets(input, LEN+2, stdin);
    input[strcspn(input, "\n")] = 0;

    if (check_password(input)) {
        printf("Access granted\n");
    } else {
        printf("Access denied\n");
    }

    return 0;
}
