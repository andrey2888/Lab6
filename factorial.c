#include <utils.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/wait.h>
#include <sys/time.h>

struct FactArgs {
  int begin;
  int end;
  int k;
};

long fact_global = 1;
void multiply_fact(int a, int k);
void *fact_from_to(void* args);
int superFact(int from, int to, int mod, int thr_num);
int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (1) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        // TODO: your code here
        break;
      case 1:
        tnum = atoi(optarg);
        // TODO: your code here
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return 1;
  }

  err = listen(server_fd, 128);
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at %d\n", port);

  while (1) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }

    while (1) {
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      int read = recv(client_fd, from_client, buffer_size, 0);

      if (!read)
        break;
      if (read < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      pthread_t threads[tnum];

      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "Receive: %llu %llu %llu\n", begin, end, mod);

      struct FactArgs args[tnum];     
      uint64_t total = superFact(begin, end, mod, tnum);

      printf("Total: %llu\n", total);

      char buffer[sizeof(total)];
      memcpy(buffer, &total, sizeof(total));
      err = send(client_fd, buffer, sizeof(total), 0);
      if (err < 0) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;
}


int superFact(int from, int to, int mod, int thr_num){
    pthread_t threads[thr_num];
    struct FactArgs  args[thr_num];
    int step = (to-from)/thr_num;
      for (int i = 0; i < thr_num; i++) {
        args[i].k = mod;
        args[i].begin = from + step * i;
        args[i].end   = (i == thr_num - 1) ? to + 1: from + step * (i+1);
        if (pthread_create(&threads[i], NULL, fact_from_to, (void *)(args+i))) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }
       for (int i = 0; i < thr_num; i++) {
       pthread_join(threads[i], NULL);
       }
       printf("%ld\n",fact_global);       
       int ret = fact_global;
       fact_global = 1;
       return ret;
}


void multiply_fact(int a, int k){
    fact_global *= a;
    fact_global %= k;
}

void *fact_from_to(void* args){
    
    struct FactArgs* FA = args;
    long partial_factorial = 1;
    for(int i = FA->begin; i < FA->end; i++){
        partial_factorial *= i;
        partial_factorial %= FA->k; 
    }
    printf("%i    %i    %ld    %ld\n", FA->begin, FA->end, partial_factorial, fact_global);
    fflush(NULL);
    multiply_fact(partial_factorial,FA->k);
    return 0;
}