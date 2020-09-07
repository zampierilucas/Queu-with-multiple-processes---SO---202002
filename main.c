#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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

struct produto {
  short int fruta;
  short int qualidade;
} produto;

FILE *esteira_lock(char *operacao, char *file_name) {

  FILE *file = fopen(file_name, operacao);

  while (file == NULL) {
    printf("File in use, waiting\n");
    sleep(1);
    file = fopen(file_name, operacao);
  }

  return file;
}
void remove_first_from_queue(FILE *file) {
  struct produto object2[QUEUE_MAX_SIZE];
  int i = 0;
  while (!feof(file)) {
    fread(&object2[i], sizeof(struct produto), 1, file);
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
    fwrite(&object2[j], sizeof(struct produto), 1, file);
  }
}
void esteira(char *operacao, struct produto *prod, char *fila) {
  FILE *file = esteira_lock(operacao, fila);

  if (strcmp(operacao, "a") == 0) {
    struct produto object;

    object.fruta = (*prod).fruta;
    object.qualidade = (*prod).qualidade;

    // printf("PF: %d Q: %d\n", (*prod).fruta, (*prod).qualidade);
    // printf("OF: %d Q: %d\n", object.fruta, object.qualidade);

    fwrite(&object, sizeof(struct produto), 1, file);

  } else if (strcmp(operacao, "r") == 0) {

    // Get first product on the queue
    fread(prod, sizeof(struct produto), 1, file);
    // printf("READ PROD: %d quali:%d\n", prod->fruta, prod->qualidade);

    // Remove product from queue
    remove_first_from_queue(file);
  }
  fclose(file);
}

void classifica_esteira() {
  // exit(0);
  char *mode = "r";
  struct produto prod;

  esteira(mode, &prod, "fila");

  printf("CLASSIFICA: F: %d Q: %d\n", prod.fruta, prod.qualidade);
}

void adiciona_na_esteira() {
  // exit(0);
  struct produto prod;
  enum frutas size_of_enum = sizeOf;
  srand(getpid());

  // Randomly generate product data
  prod.fruta = (rand() % (int)size_of_enum);
  prod.qualidade = rand() % 100;

  printf("ADD: F: %d Q: %d\n", prod.fruta, prod.qualidade);

  char *mode = "a";
  esteira(mode, &prod, "fila");
}

int main() {
  pid_t c1_pid, c2_pid, c3_pid, c4_pid;
  int DEBUG = 0;

  // Creates 5 children
  (c1_pid = fork()) && (c2_pid = fork()) && (c3_pid = fork()) && (c4_pid = fork());

  if (c1_pid == 0) {
    // printf("1 - Adicionando a esteira um produto\n");
    // adiciona_na_esteira();
    DEBUG ? printf("1 - Filho: Meu PID = %d, PPID = %d \n", getpid(), getppid()) : false;

    exit(0);
  } else if (c2_pid == 0) {
    // printf("2 - Adicionando a esteira um produto\n");
    adiciona_na_esteira();
    // sleep(1);
    exit(0);
    DEBUG ? printf("2 - Filho: Meu PID = %d, PPID = %d \n", getpid(), getppid()) : false;

  } else if (c3_pid == 0) {
    // printf("3 - Adicionando a esteira um produto\n");
    // adiciona_na_esteira();
    exit(0);

    DEBUG ? printf("3 - Filho: Meu PID = %d, PPID = %d \n", getpid(), getppid()) : false;

  } else if (c4_pid == 0) {
    classifica_esteira();
    exit(0);

    DEBUG ? printf("4 - Filho: Meu PID = %d, PPID = %d \n", getpid(), getppid()) : false;

  } else {
    sleep(2);

    DEBUG ? printf("Pai: PID = %d, PPID = %d \n", getpid(), getppid()) : false;
  }
}