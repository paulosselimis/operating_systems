#include <stdio.h>
#include <stdlib.h>                
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

time_t start_time;
int gate;
char state;

void handle_sigterm(int sig) {
    exit(0);
}

void SIGALARM_handler(int sig) {
    time_t current_time = time(NULL);  // get the current time
    int elapsed_time = (int) difftime(current_time, start_time);
    printf("[GATE=%d/PID=%d/TIME=%ds] The gates are %s!\n", gate, getpid(), elapsed_time, state == 't' ? "open" : "closed");
    if(alarm(15)==-1) { perror("alarm"); exit(1);}
    
}

void handle_sigusr1(int sig) {
    if(state=='t') state ='f';
    else state = 't';
    time_t current_time = time(NULL);
    int elapsed_time = (int) difftime(current_time, start_time);
    printf("[GATE=%d/PID=%d/TIME=%ds] The gates are %s!\n", gate, getpid(), elapsed_time, state == 't' ? "open" : "closed");
   
}

void handle_sigusr2(int sig) {
    time_t current_time = time(NULL);  // get the current time
    int elapsed_time = (int) difftime(current_time, start_time);
    printf("[GATE=%d/PID=%d/TIME=%ds] The gates are %s!\n", gate, getpid(), elapsed_time, state == 't' ? "open" : "closed");

}


int main(int argc, char *args[]) {
            gate = atoi(args[1]);
            char *gates_str=args[2];
            state= gates_str[gate];
            sleep(1);
            start_time = time(NULL);
            printf("[GATE=%d/PID=%d/TIME=0s] The gates are %s!\n", gate, getpid(), state == 't' ? "open" : "closed");
            if(signal(SIGALRM, SIGALARM_handler) == SIG_ERR) { perror("signal"); return 1; }
            if(signal(SIGUSR1, handle_sigusr1) == SIG_ERR) { perror("signal"); return 1; }
            if(signal(SIGUSR2, handle_sigusr2) == SIG_ERR) { perror("signal"); return 1; }
            if(signal(SIGTERM, handle_sigterm)  == SIG_ERR) { perror("signal"); return 1; }
            if(alarm(15)==-1) {perror("alarm"); return 1; } 
            while(1) {
             pause();
            }  
            
    return 0;
}