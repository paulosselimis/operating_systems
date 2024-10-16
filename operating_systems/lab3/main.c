#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <string.h>
#include <ctype.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int is_digit_string(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        if (!isdigit(str[i])) {
            return 1;
        }
        i++;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char *mode = "--round-robin";
    
    if(argc > 3 || argc == 1) {
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 1;
    }
    int n = atoi(argv[1]);
    if(is_digit_string(argv[1]) != 0) { 
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 1;
    }
    if(argc == 3 && (strcmp(argv[2], "--round-robin") != 0 && strcmp(argv[2], "--random") != 0) ) {
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 1;
    }
    if(argc == 3 && strcmp(argv[2], "--random") == 0) mode = "--random";
    
    int dn = 2 * n;
    int (*pd)[2] = malloc(sizeof(*pd) * dn); // array of 2n pipes, each pipe has two file descriptors
    pid_t *id = malloc(n * sizeof(pid_t)); // array of n child process IDs
    if(pd == NULL || id == NULL) {
        perror("malloc");
        return 1;
    }
    // create the pipes
  for (int i = 0; i < dn; i++) {
    if (pipe(pd[i]) < 0) {
      perror("pipe");
      return 1;
    }
  }
  
   // create the child processes
  for (int i = 0; i < n; i++) { 
      id[i] = fork();
      if(id[i] < 0) {
          perror("fork");
          return 1;
      }
      else if(id[i] == 0) { //child process
      int val;
      int child = i;
      while(1) { 
          if(read(pd[child][0], &val, sizeof(int)) < 0) {
              perror("read");
              return 1;
          }
          printf("[Child %d] [%d] Child received %d!\n", child, getpid(), val); 
          val++; 
          sleep(5); 
          if(write(pd[(dn - 1) - child][1], &val, sizeof(int)) < 0) {
              perror("write");
              return 1;
          }
          printf("[Child %d] [%d] Child Finished hard work, writing back %d\n", child, getpid(), val); 
      }

       return 0;   
      }
      else { //parent process
      
      }
  }
  int child = 0;
  while(1) {
        fd_set inset;
        int maxfd = 0;

        FD_ZERO(&inset);                // we must initialize before each call to select
       // add all read ends of the pipes to the set
    for (int i = n; i < dn; i++) {
        FD_SET(pd[i][0], &inset);
        maxfd = MAX(maxfd, pd[i][0] + 1); // update maxfd
    }
    FD_SET(STDIN_FILENO, &inset);   // select will check for input from stdin
    maxfd = MAX(maxfd, STDIN_FILENO + 1); // update maxfd

       
        int ready_fds = select(maxfd, &inset, NULL, NULL, NULL);
        if (ready_fds <= 0) {
            perror("select");
            continue;                                       // just try again
        }

        // user has typed something, we can read from stdin without blocking
        if (FD_ISSET(STDIN_FILENO, &inset)) { 
            int val;
            char buffer[101];
            int n_read = read(STDIN_FILENO, buffer, 100);
            if(n_read < 0) {
                perror("read");
                return 1;
                }
            buffer[n_read] = '\0';                          

            // New-line is also read from the stream, discard it.
            if (n_read > 0 && buffer[n_read-1] == '\n') {
                buffer[n_read-1] = '\0';
            }
            if(n_read == 5 && strcmp(buffer, "exit") == 0) { //exit
                for(int i = 0; i < n; i++) {
                    if(close(pd[i][0]) == -1) {perror("close"); return 1;} 
                    if(close(pd[i][1]) == -1) {perror("close"); return 1;}
                    if(kill(id[i], SIGTERM) == -1) {perror("kill"); return 1;}
                    printf("Waiting for child %d\n", i);
                    if(wait(NULL) == -1) {perror("wait"); return 1;}
                }
                for (int i = n; i < dn; i++) {
                    if(close(pd[i][0]) == -1) {perror("close"); return 1;} 
                    if(close(pd[i][1]) == -1) {perror("close"); return 1;}
                }
                printf("All children terminated\n");
                free(id);
                free(pd);
                exit(0);
            }
            if(is_digit_string(buffer) != 0) { //help or random string 
                printf("Type a number to send job to a child!\n");
            }
            else {
            val = atoi(buffer);
            if(strcmp(mode, "--round-robin") == 0) {
            printf("[Parent] Assigned %d to child %d\n", val, child); 
            if(write(pd[child][1], &val, sizeof(int)) == -1) {perror("write"); return 1;}
            child++;
            if(child==n) child = 0; 
            }
            else { //--random
            int random_child = rand() % n;
            printf("[Parent] Assigned %d to child %d\n", val, random_child); 
            if(write(pd[random_child][1], &val, sizeof(int)) == -1) {perror("write"); return 1;}
            }
            }
        }
        
        // someone has written bytes to the pipe, we can read without blocking
        for (int i = n; i < dn; i++) {
               if (FD_ISSET(pd[i][0], &inset)) {
                    int val;
                    if(read(pd[i][0], &val, sizeof(int)) == -1) {perror("read"); return 1;}              

                    printf("[Parent] Received result from child %d --> %d\n", (dn - 1) - i, val);
               }
        }
        
      }
    
 return 0;   
}
