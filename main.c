#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define QUEUE_MAX_SIZE 256
#define LOCK_SH 1 /* shared lock */
#define LOCK_EX 2 /* exclusive lock */
#define LOCK_NB 4 /* don't block when locking */
#define LOCK_UN 8 /* unlock */

enum frutas {
  banana,
  uva,
  abacate,
  abacaxi,
  sizeOf
} fruta;

struct product {
  short int fruit;
  short int quality;
} product;

FILE *queue_lock(char *operacao, char *file_name) {
  int lock;
  FILE *file = fopen(file_name, operacao);
  lock = flock(fileno(file), LOCK_EX);

  while (file == NULL) {
    printf("File in use, waiting\n");

    sleep(1);
    file = fopen(file_name, operacao);
    lock = flock(fileno(file), LOCK_EX);
  }

  return file;
}

#define ADD 1

void remove_first_from_queue(FILE *file) {
  int i = 0, n = 0;
  struct product fromQueue[QUEUE_MAX_SIZE];

  // Get data from file
  while (!feof(file)) {
    n = fread(&fromQueue[i], sizeof(struct product), 1, file);

    // Nothing to read
    if (n == 0)
      break;

    i++;
  }

  // Clear file queue
  freopen("fila", "w", file);

  // Write data to file
  for (int j = 0; j < i; j++) {
    fwrite(&fromQueue[j], sizeof(struct product), 1, file);
  }
}

void queue(char *oper, struct product *prod, char *fila) {
  FILE *file = queue_lock(oper, fila);

  if (strcmp(oper, "a") == 0) {
    // Aux structure
    struct product toQueue;

    // Add ramdomly generated data to object
    toQueue.fruit = (*prod).fruit;
    toQueue.quality = (*prod).quality;

    // Write to end queue
    fwrite(&toQueue, sizeof(struct product), 1, file);

  } else if (strcmp(oper, "r") == 0) {
    int test = 0;

    // Read data to be processed
    test = fread(prod, sizeof(struct product), 1, file);

    // If there is no data to read
    if (test == 0) {
      printf("Nothing to sort on queue\n");
      exit(1);
    }

    // Remove product from queue
    remove_first_from_queue(file);
  }
  fclose(file);
}

void classifica_esteira() {
  // ADD ? false : exit(0);
  char *mode = "r";
  struct product prod;

  queue(mode, &prod, "fila");

  printf("SORT: F: %d Q: %d\n", prod.fruit, prod.quality);
}

void adiciona_na_esteira() {
  // ADD ? exit(0) : false;
  struct product prod;
  enum frutas size_of_enum = sizeOf;
  srand(getpid());

  // Randomly generate product data
  prod.fruit = (rand() % (int)size_of_enum);
  prod.quality = rand() % 100;

  printf("ADD: F: %d Q: %d\n", prod.fruit, prod.quality);

  char *mode = "a";
  queue(mode, &prod, "fila");
}

int main() {
  pid_t c1_pid, c2_pid, c3_pid, c4_pid, c5_pid;
  int DEBUG = 0;

  // Creates 5 children
  (c1_pid = fork()) && (c2_pid = fork()) && (c3_pid = fork()) && (c4_pid = fork()) && (c5_pid = fork());

  if (c1_pid == 0) {
    DEBUG ? printf("1C-Feeder: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
    // adiciona_na_esteira();
    exit(0);

  } else if (c2_pid == 0) {
    DEBUG ? printf("2C-Feeder: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
    adiciona_na_esteira();
    exit(0);

  } else if (c3_pid == 0) {
    DEBUG ? printf("3C-Feeder: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
    adiciona_na_esteira();
    exit(0);

  } else if (c4_pid == 0) {
    DEBUG ? printf("4C-Consumer: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
    classifica_esteira();
    exit(0);

  } else if (c5_pid == 0) {
    DEBUG ? printf("5C-Consumer: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
    classifica_esteira();
    exit(0);

  } else {
    DEBUG ? printf("Pai: PID = %d, PPID = %d \n", getpid(), getppid()) : false;

    pid_t pid1 = wait(&c1_pid);
    pid_t pid2 = wait(&c2_pid);
    pid_t pid3 = wait(&c3_pid);
    pid_t pid4 = wait(&c4_pid);
    pid_t pid5 = wait(&c5_pid);

    if (DEBUG) {
      printf("\n\nCPID = %d, Status = %d\n", pid1, WEXITSTATUS(c1_pid));
      printf("CPID = %d, Status = %d\n", pid2, WEXITSTATUS(c2_pid));
      printf("CPID = %d, Status = %d\n", pid3, WEXITSTATUS(c3_pid));
      printf("CPID = %d, Status = %d\n", pid4, WEXITSTATUS(c4_pid));
      printf("CPID = %d, Status = %d\n", pid5, WEXITSTATUS(c5_pid));
    }
  }
}