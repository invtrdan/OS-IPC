#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define NUM_LOOPS 25

struct SharedData {
    int BankAccount;
    int Turn;
};

void DearOldDad(struct SharedData *sharedData);
void PoorStudent(struct SharedData *sharedData);

int main() {
    int ShmID;
    struct SharedData *ShmPTR;
    pid_t pid;
    int status;

    // Create the shared memory segment
    ShmID = shmget(IPC_PRIVATE, sizeof(struct SharedData), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }
    printf("Parent has received a shared memory segment...\n");

    // Attach the shared memory segment
    ShmPTR = (struct SharedData *)shmat(ShmID, NULL, 0);
    if ((long)ShmPTR == -1) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }
    printf("Parent has attached the shared memory...\n");

    // Initialize BankAccount and Turn
    ShmPTR->BankAccount = 0;
    ShmPTR->Turn = 0;

    // Fork a child process
    printf("Parent is about to fork a child process...\n");
    pid = fork();
    if (pid < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
    } else if (pid == 0) {
        // Child process
        PoorStudent(ShmPTR);
        exit(0);
    } else {
        // Parent process
        DearOldDad(ShmPTR);
    }

    // Wait for the child process to finish
    wait(&status);
    printf("Parent has detected the completion of its child...\n");

    // Detach and remove the shared memory segment
    shmdt((void *)ShmPTR);
    printf("Parent has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Parent has removed its shared memory...\n");
    printf("Parent exits...\n");
    exit(0);
}

void DearOldDad(struct SharedData *sharedData) {
    int balance;
    srand(time(NULL)); // Seed the random number generator

    for (int i = 0; i < NUM_LOOPS; i++) {
        sleep(rand() % 6); // Sleep for 0-5 seconds

        // Work with a local copy of BankAccount
        int account = sharedData->BankAccount;

        // Loop while Turn != 0 (busy-waiting)
        while (sharedData->Turn != 0);

        if (account <= 100) {
            balance = rand() % 101; // Randomly 0-100
            if (balance % 2 == 0) { // If even, deposit the balance
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        // Write the local copy back to the shared memory
        sharedData->BankAccount = account;
        sharedData->Turn = 1; // Set Turn to 1
    }
}

void PoorStudent(struct SharedData *sharedData) {
    int balance;
    srand(time(NULL) + getpid()); // Seed the random number generator with a different value than the parent

    for (int i = 0; i < NUM_LOOPS; i++) {
        sleep(rand() % 6); // Sleep for 0-5 seconds

        // Work with a local copy of BankAccount
        int account = sharedData->BankAccount;

        // Loop while Turn != 1 (busy-waiting)
        while (sharedData->Turn != 1);

        balance = rand() % 51; // Randomly 0-50
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) { // If the balance needed is less than or equal to the account
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // Write the local copy back to the shared memory
        sharedData->BankAccount = account;
        sharedData->Turn = 0; // Set Turn to 0
    }
}
