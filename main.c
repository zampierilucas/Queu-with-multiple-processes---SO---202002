// Lucas Marcon Zampieri -

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct product {
  short int fruit;
  short int quality;
} product;

#define FEEDERS 3 // Number of feeder processes
#define SORTERS 2 // Number of sorter processes

#define N_FRUITS 6                     // Fruits supported
#define N_ITEMS 9                      // Must be a multiple o 3
#define N_PER_FEEDER N_ITEMS / FEEDERS // Items that a Feeder will add

int p_buff[2]; // Feeder to queue pipe
int p_list[2]; // Sortet to parent pipe

// Sortim algoritm
void bsort(struct product *list_to_sort, int arr_size) {
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

// Feeder processes main function
void add_to_queue() {
  srand(getpid() * time(NULL));
  struct product *prod = malloc(sizeof(product));

  for (int i = 0; i < N_PER_FEEDER; i++) {

    // Random delay to input and iten
    sleep(rand() % (5 + 1 - 1) + 1);

    // Randomly generate product data
    prod->fruit = (rand() % N_FRUITS);
    prod->quality = rand() % 100;

    write(p_buff[1], prod, sizeof(product));
    printf("%d ADD:  Fruta: %d Qualidade: %d\n", getpid(), prod->fruit, prod->quality);
  }
  exit(0);
}

// Sorter processes main function
void sort_queue() {
  int timeout = 0;
  ssize_t read_size;

  struct product *aux = malloc(sizeof(product));
  struct product ord_list[N_ITEMS];

  while (timeout < N_ITEMS) {
    // Get data from pipe(queue)
    read_size = read(p_buff[0], aux, sizeof(product));

    if (read_size > 0) {
      printf("%d SORT: Fruta: %d Qualidade: %d\n", getpid(), aux->fruit, aux->quality);

      ord_list[N_ITEMS - 1] = *aux;
      bsort(ord_list, N_ITEMS);
    } else {
      // If pipe is empty wait
      sleep(2);
      timeout++;
    }
  }

  // printf("\n");
  // for (int i = 0; i < N_ITEMS; i++)
  //   printf("%d LIST: Fruta: %d Qualidade: %d\n", getpid(), ord_list[i].fruit, ord_list[i].quality);

  printf("\n%d - Pipe is empty exiting ", getpid());

  // Send ordenated list of current classificator to feeder.
  write(p_list[1], &ord_list, sizeof(ord_list));
  exit(0);
}

int main() {
  pid_t c1_pid, c2_pid, c3_pid, c4_pid, c5_pid;

  // Spawn pipes
  if (pipe(p_buff) == -1 || pipe(p_list) == -1) {
    exit(EXIT_FAILURE);
  }

  // Make pipe reads non-blocking
  if (fcntl(p_buff[0], F_SETFL, O_NONBLOCK) < 0 || fcntl(p_list[0], F_SETFL, O_NONBLOCK) < 0)
    exit(2);

  // Creates 5 children
  (c1_pid = fork()) && (c2_pid = fork()) && (c3_pid = fork()) && (c4_pid = fork()) && (c5_pid = fork());

  // CHILDREN - FEEDER 1
  if (c1_pid == 0)
    add_to_queue();

  // CHILDREN - FEEDER 2
  else if (c2_pid == 0)
    add_to_queue();

  // CHILDREN - FEEDER 3
  else if (c3_pid == 0)
    add_to_queue();

  // CHILDREN - SORTER 1
  else if (c4_pid == 0)
    sort_queue();

  // CHILDREN - SORTER 2
  else if (c5_pid == 0)
    sort_queue();

  // PARENT
  else {

    //Wait for all children to exit
    while (wait(&c1_pid) > 0 || wait(&c2_pid) > 0 || wait(&c3_pid) > 0 || wait(&c4_pid) > 0 || wait(&c5_pid) > 0)
      continue;

    // Create a list to receive the sorted lists of the classificators
    struct product ordered_list[N_ITEMS * 2];

    // Get lists from Sorters
    read(p_list[0], &ordered_list, (sizeof(product) * N_ITEMS) * 2);

    // Merge classificators lists
    bsort(ordered_list, N_ITEMS * 2);

    // Print final classificated list
    printf("\n");
    for (int i = 0; i < N_ITEMS; i++) {
      printf("LIST: Fruta: %d Qualidade: %d\n", ordered_list[i].fruit, ordered_list[i].quality);
    }
    exit(0);
  }
}