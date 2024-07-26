#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int add_numbers(int a, int b) {
    int result;
    result = a + b;
    printf("Addition result: %d\n", result);
    return 0;
}

int subtract_numbers(int a, int b) {
    int result;
    result = a - b;
    printf("Subtraction result: %d\n", result);
    return 0;
}

int multiply_numbers(int a, int b) {
    int result;
    result = a * b;
    printf("Multiplication result: %d\n", result);
    return 0;
}

int divide_numbers(int a, int b) {
    int result;
    if (b != 0) {
        result = a / b;
        printf("Division result: %d\n", result);
    } else {
        printf("Error: Division by zero\n");
    }
    return 0;
}

int vulnerable_function(char *input) {
    char buffer[8];
    strcpy(buffer, input);
    printf("You entered: %s\n", buffer);
    // long long *saved_base_pointer = (long long *)(buffer + sizeof(buffer));
    // long long *return_address = saved_base_pointer + 1;
    // printf("Saved base pointer: 0x%llx\n", *saved_base_pointer);
    // printf("Return address: 0x%llx\n", *return_address);
    return 0;
}

int secret_function() {
    printf("You've reached the secret function!\n");
    exit(0);
}

int process_numbers(int a, int b) {
    add_numbers(a, b);
    subtract_numbers(a, b);
    multiply_numbers(a, b);
    divide_numbers(a, b);
    return 0;
}

int menu() {
    printf("Choose an option:\n");
    printf("1. Add numbers\n");
    printf("2. Subtract numbers\n");
    printf("3. Multiply numbers\n");
    printf("4. Divide numbers\n");
    printf("5. Enter a string\n");
    printf("6. Exit\n");
    return 0;
}

int main() {
    int choice, a, b;
    char input[100];

    while (1) {
        menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter two integers: ");
                scanf("%d %d", &a, &b);
                add_numbers(a, b);
                break;
            case 2:
                printf("Enter two integers: ");
                scanf("%d %d", &a, &b);
                subtract_numbers(a, b);
                break;
            case 3:
                printf("Enter two integers: ");
                scanf("%d %d", &a, &b);
                multiply_numbers(a, b);
                break;
            case 4:
                printf("Enter two integers: ");
                scanf("%d %d", &a, &b);
                divide_numbers(a, b);
                break;
            case 5:
                printf("Enter a string: ");
                scanf("%s", input);
                vulnerable_function(input);
                break;
            case 6:
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }

    return 0;
}
