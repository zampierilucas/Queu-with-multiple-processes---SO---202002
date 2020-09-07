#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define QUEUE_MAX_SIZE 256

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

  FILE *file = fopen(file_name, operacao);

  while (file == NULL) {
    printf("File in use, waiting\n");
    sleep(1);
    file = fopen(file_name, operacao);
  }

  return file;
}
void remove_first_from_queue(FILE *file) {
  int i = 0;
  struct product fromQueue[QUEUE_MAX_SIZE];

  while (!feof(file)) {
    fread(&fromQueue[i], sizeof(struct product), 1, file);
    // printf("obj2: %d quali:%d %d\n", object2[i].fruta, object2[i].qualidade, i);
    i++;

    int c = fgetc(file);
    if (c == EOF) {
      printf("Hit EOF\n");
      ungetc(c, file);
    } else
      ungetc(c, file);
  }

  freopen("fila", "w", file);
  for (int j = 0; j < i; j++) {
    fwrite(&fromQueue[j], sizeof(struct product), 1, file);
  }
}
void queue(char *oper, struct product *prod, char *fila) {
  FILE *file = queue_lock(oper, fila);

  if (strcmp(oper, "a") == 0) {
    struct product toQueue;

    toQueue.fruit = (*prod).fruit;
    toQueue.quality = (*prod).quality;

    // printf("PF: %d Q: %d\n", (*prod).fruta, (*prod).qualidade);
    // printf("OF: %d Q: %d\n", object.fruta, object.qualidade);

    fwrite(&toQueue, sizeof(struct product), 1, file);

  } else if (strcmp(oper, "r") == 0) {

    // Get first product on the queue
    fread(prod, sizeof(struct product), 1, file);
    // printf("READ PROD: %d quali:%d\n", prod->fruta, prod->qualidade);

    // Remove product from queue
    remove_first_from_queue(file);
  }
  fclose(file);
}

void classifica_esteira() {
  // exit(0);
  char *mode = "r";
  struct product prod;

  queue(mode, &prod, "fila");

  printf("SORT: F: %d Q: %d\n", prod.fruit, prod.quality);
}

void adiciona_na_esteira() {
  // exit(0);
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
    // adiciona_na_esteira();
    exit(0);

  } else if (c4_pid == 0) {
    DEBUG ? printf("4C-Consumer: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
    classifica_esteira();
    exit(0);

  } else if (c5_pid == 0) {
    DEBUG ? printf("5C-Consumer: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
    // classifica_esteira();
    exit(0);

  } else {
    DEBUG ? printf("Pai: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
    sleep(2);

    pid_t pid1 = wait(&c1_pid);
    pid_t pid2 = wait(&c2_pid);
    pid_t pid3 = wait(&c3_pid);
    pid_t pid4 = wait(&c4_pid);
    pid_t pid5 = wait(&c5_pid);

    printf("\n\n\nPai: Filho (PID = %d) terminou, Status = %d \n", pid1, WEXITSTATUS(c1_pid));
    printf("Pai: Filho (PID = %d) terminou, Status = %d \n", pid2, WEXITSTATUS(c2_pid));
    printf("Pai: Filho (PID = %d) terminou, Status = %d \n", pid3, WEXITSTATUS(c3_pid));
    printf("Pai: Filho (PID = %d) terminou, Status = %d \n", pid4, WEXITSTATUS(c4_pid));
    printf("Pai: Filho (PID = %d) terminou, Status = %d \n", pid5, WEXITSTATUS(c5_pid));
  }
}