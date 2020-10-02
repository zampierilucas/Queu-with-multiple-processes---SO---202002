// Lucas Marcon Zampieri -

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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

#define ITEMS_TO_ADD 9 // Must be a multiple o 3
#define ORDERED 20
int p_buff[2];
int p_list[2];
int timeout = 0;

struct product ord_list[ORDERED];

void adiciona_na_esteira() {
  srand(getpid() * time(NULL));
  struct product *prod = malloc(sizeof(product));

  for (int i = 0; i < ITEMS_TO_ADD / 3; i++) {
    // Random delay to input and iten
    sleep(rand() % (5 + 1 - 1) + 1);
    enum frutas size_of_enum = sizeOf;

    // Randomly generate product data
    prod->fruit = (rand() % (int)size_of_enum);
    prod->quality = rand() % 100;

    close(p_buff[0]);
    write(p_buff[1], prod, sizeof(product));

    printf("%d ADD:  Fruta: %d Qualidade: %d\n", getpid(), prod->fruit, prod->quality);
  }
  exit(0);
}

void bsortDesc(struct product *list_to_sort, int arr_size) {
  struct product temp;

  // Sort value in new list
  for (int i = 0; i < arr_size; i++) {
    for (int j = 0; j < arr_size - 1; j++) {
      if (list_to_sort[j].quality < list_to_sort[j + 1].quality) {
        temp = list_to_sort[j];
        list_to_sort[j] = list_to_sort[j + 1];
        list_to_sort[j + 1] = temp;
      }
    }
  }
}

void classifica_esteira() {
  while (true) {
    struct product *prod = malloc(sizeof(product));

    close(p_buff[1]);
    ssize_t read_size = read(p_buff[0], prod, sizeof(product));

    if (read_size > 0) {
      printf("%d SORT: Fruta: %d Qualidade: %d\n", getpid(), prod->fruit, prod->quality);
      ord_list[ITEMS_TO_ADD - 1] = *prod;
      bsortDesc(ord_list, ITEMS_TO_ADD);
    } else {
      // If pipe is empty wait
      sleep(2);
      timeout++;

      if (timeout > 6) {
        printf("\n");
        printf("%d - Pipe is empty exiting", getpid());

        // Send ordenated list of current classificator to feeder.
        write(p_list[1], &ord_list, sizeof(ord_list));

        // for (int i = 0; i < ORDERED - 10; i++) {
        //   printf("%d LIST: Fruta: %d Qualidade: %d\n", getpid(), ord_list[i].fruit, ord_list[i].quality);
        // }
        exit(0);
      }
    }
  }
}

int main() {
  pid_t c1_pid, c2_pid, c3_pid, c4_pid, c5_pid;

  // Spawn pipes
  if (pipe(p_buff) == -1 || pipe(p_list) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  // Make pipe reads non-blocking
  if (fcntl(p_buff[0], F_SETFL, O_NONBLOCK) < 0 || fcntl(p_list[0], F_SETFL, O_NONBLOCK) < 0)
    exit(2);

  // Creates 5 children
  (c1_pid = fork()) && (c2_pid = fork()) && (c3_pid = fork()) && (c4_pid = fork()) && (c5_pid = fork());

  // CHILDREN - FEEDER 1
  if (c1_pid == 0)
    adiciona_na_esteira();

  // CHILDREN - FEEDER 2
  else if (c2_pid == 0)
    adiciona_na_esteira();

  // CHILDREN - FEEDER 3
  else if (c3_pid == 0)
    adiciona_na_esteira();

  // CHILDREN - CLASSIFICATOR 1
  else if (c4_pid == 0)
    classifica_esteira();

  // CHILDREN - CLASSIFICATOR 2
  else if (c5_pid == 0)
    classifica_esteira();

  // PARENT
  else {

    //Wait for all children to exit
    while (wait(&c1_pid) > 0 || wait(&c2_pid) > 0 || wait(&c3_pid) > 0 || wait(&c4_pid) > 0 || wait(&c5_pid) > 0)
      continue;

    // Create a list to receiva the sorted lists of the classificators
    struct product ordered_list[ORDERED * 2];

    read(p_list[0], &ordered_list, sizeof(ord_list) * 2);

    // Merge classificators lists
    bsortDesc(ordered_list, ORDERED * 2);

    // Print final classificated list
    printf("\n");
    for (int i = 0; i < ITEMS_TO_ADD; i++) {
      printf("LIST: Fruta: %d Qualidade: %d\n", ordered_list[i].fruit, ordered_list[i].quality);
    }
    exit(0);
  }
}