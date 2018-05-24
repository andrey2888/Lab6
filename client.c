#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server {
  char ip[255];
  int port;
};
uint64_t k = -1;
uint64_t mod = -1;
int sck[3]; 
int total_result = 1;
void wait_for_responce(void* args){
    int* sck_to_wait = args;
    char response[sizeof(uint64_t)];
    printf("alala %d\n",*sck_to_wait);
    fflush(NULL);
    if (recv(*sck_to_wait, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }
    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    total_result *= answer;
    total_result %= mod;
    printf("answer: %llu\n", answer);
    close(sck); 
}


int main(int argc, char **argv) {
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        // TODO: your code here
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // TODO: for one server here, rewrite with servers from file
  FILE* serv_list = fopen(servers,"r");
  char* line = 0;
  size_t len = 0;
  ssize_t read;
  int servers_num = 0;
  char addresses[100][100];
  int ports[100];
  while ((read = getline(&line, &len, serv_list)) != -1) {
        //printf("%s\n", line);
        char* pos = strchr(line,':');
        char port_str[100];
        char addr_str[100];
        strcpy(port_str, pos + 1);
        *pos = '\0';
        strcpy(addresses + servers_num, line);
        ports[servers_num] = atoi(port_str);
        //printf("%d  %s\n", ports[servers_num], addresses + servers_num);  
        //fflush(NULL);
        servers_num++;
  }
  struct Server *to = malloc(sizeof(struct Server) * servers_num);

  
  // TODO: work continiously, rewrite to make parallel

    struct sockaddr_in server;
    for(int i = 0; i < servers_num; i++){
    strcpy(to[i].ip, addresses[i]);
    to[i].port = 20001 + i;
    struct hostent *hostname = gethostbyname(to[i].ip);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);
    server.sin_family = AF_INET;
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }
        server.sin_port = htons(to[i].port);
        sck[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sck[i] < 0) {
          fprintf(stderr, "Socket creation failed!\n");
          exit(1);
        }
    
        if (connect(sck[i], (struct sockaddr *)&server, sizeof(server)) < 0) {
          fprintf(stderr, "Connection failed\n");
          exit(1);
        }
    }
    // TODO: for one server
    // parallel between servers
    int step = k/servers_num;
    pthread_t threads[servers_num];
    for(int i = 0; i < servers_num; i++){
        uint64_t begin = step * i + 1;
        uint64_t end = (i == servers_num - 1) ? k : step * (i+1);
        char task[sizeof(uint64_t) * 3];
        memcpy(task, &begin, sizeof(uint64_t));
        memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
        memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));
    
        if (send(sck[i], task, sizeof(task), 0) < 0) {
          fprintf(stderr, "Send failed\n");
          exit(1);
        }
        printf("olol %d\n", sck[i]);
        fflush(NULL);
        pthread_create(&threads[i], NULL, wait_for_responce,(void*)&sck[i]);
        
    }
   for (int i = 0; i < servers_num; i++) {
       pthread_join(threads[i], NULL);
   }
    printf("total result: %llu\n", total_result);
  
  free(to);

  return 0;
}
