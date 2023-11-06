#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd1[2];  // Used to store two ends of the pipe from P1 to P2
    int fd2[2];  // Used to store two ends of the pipe from P2 to P1

    char fixed_str1[] = "howard.edu";
    char fixed_str2[] = "gobison.org";
    char input_str[100];
    pid_t p;

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    p = fork();

    if (p < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    }

    // P1 (Parent) process
    if (p > 0) {
        close(fd1[0]); // Close reading end of the pipe from P1 to P2
        close(fd2[1]); // Close writing end of the pipe from P2 to P1

        char concat_str[200]; // Maintain concatenated result

        printf("Enter a string to send to P2: ");
        scanf("%s", input_str);

        // Write input string to P2 and close writing end of the pipe from P1 to P2
        write(fd1[1], input_str, strlen(input_str) + 1);

        // Read the concatenated string from P2
        read(fd2[0], concat_str, sizeof(concat_str));

        // Print the concatenated string
        printf("Concatenated string from P2: %s\n", concat_str);

        // Prompt for the second input
        printf("Enter a string to send to P2: ");
        scanf("%s", input_str);

        // Concatenate the second input with the result from P2
        strcat(concat_str, input_str);

        // Print the final concatenated string
        printf("Concatenated string from P2: %s\n", concat_str);

        close(fd1[1]); // Close writing end of the pipe from P1 to P2
        close(fd2[0]); // Close reading end of the pipe from P2 to P1
    }

    // P2 (Child) process
    else {
        close(fd1[1]); // Close writing end of the pipe from P1 to P2
        close(fd2[0]); // Close reading end of the pipe from P2 to P1

        // Read the string sent by P1
        char received_str[100];
        read(fd1[0], received_str, sizeof(received_str));

        // Concatenate "howard.edu" to the received string
        strcat(received_str, fixed_str1);

        // Write the first concatenated string back to P1
        write(fd2[1], received_str, strlen(received_str) + 1);

        // Read the second string sent by P1
        read(fd1[0], received_str, sizeof(received_str));

        // Concatenate "gobison.org" to the second received string
        strcat(received_str, fixed_str2);

        // Write the second concatenated string back to P1
        write(fd2[1], received_str, strlen(received_str) + 1);

        close(fd1[0]); // Close reading end of the pipe from P1 to P2
        close(fd2[1]); // Close writing end of the pipe from P2 to P1

        exit(0);
    }

    return 0;
}
