#include <stdio.h>
#include <stdlib.h>                
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
void SIGALARM_handler(int sig);
void handle_sigusr1(int sig);
void handle_sigusr2(int sig);
void handle_sigterm(int sig);
time_t start_time;
int gate, num_gates, status, new_i;
int parent_signal = 0;
pid_t id[100], terminated_pid, wait_pid;
char state;
char *gates_str;

void handle_sigchld(int sig) {
    // Child process has terminated or stopped
     if (WIFSTOPPED(status)) {
         terminated_pid = waitpid(-1, &status, WNOHANG); if(terminated_pid==-1) {perror("waitpid"); exit(1);}
         if(kill(terminated_pid, SIGCONT)==-1) {perror("kill"); exit(1); }
        }
     else if(parent_signal==0) {
         terminated_pid = waitpid(-1, &status, WNOHANG); if(terminated_pid==-1) {perror("waitpid"); exit(1);}
            for(int i=0; i<num_gates; i++) {
                if(id[i]==terminated_pid) {new_i=i; break;} }
                printf("[PARENT/PID=%d] Child %d with PID=%d exited\n", getpid(), new_i, terminated_pid );
            
    id[new_i]=fork(); 
    if(id[new_i]==-1) { perror("fork"); exit(1); }
    else if(id[new_i]>0) printf("[PARENT/PID=%d] Created new child for gate %d (PID %d) and initial state '%c'\n", getpid(), new_i, id[new_i],  gates_str[new_i] );
    else  {
            //child 
            gate=new_i;
            state= gates_str[new_i];
            sleep(1);
            start_time = time(NULL);
            printf("[GATE=%d/PID=%d/TIME=0s] The gates are %s!\n", gate, getpid(), state == 't' ? "open" : "closed");
            if(signal(SIGALRM, SIGALARM_handler) == SIG_ERR) { perror("signal"); exit(1); }
            if(signal(SIGUSR1, handle_sigusr1) == SIG_ERR) { perror("signal"); exit(1); }
            if(signal(SIGUSR2, handle_sigusr2) == SIG_ERR) { perror("signal"); exit(1); }
            if(signal(SIGTERM, handle_sigterm)  == SIG_ERR) { perror("signal"); exit(1); }
            if(alarm(15)==-1) {perror("alarm"); exit(1); } 
           while(1) {
            
            pause();
            }  
            

        }
    }
   
    
}

void handle_sigterm(int sig) {
    exit(0);
}

void handle_sigterm_parent(int sig) {
    parent_signal=1;
    int children = num_gates;
    for(int i=0; i<num_gates; i++){
        printf("[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(), children );
        if(kill(id[i],SIGTERM)==-1) {perror("kill"); exit(1);} children--; 
        terminated_pid=wait(&status); if(terminated_pid==-1) { perror("wait"); exit(1);} 
        printf("[PARENT/PID=%d] Child with PID=%d terminated successfully with exit status code 0!\n", getpid(), terminated_pid );
    }
    printf("[PARENT/PID=%d] All children exited, terminating as well\n", getpid());
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

void handle_sigusr1parent(int sig) {
    for(int i=0; i<num_gates; i++){
        if(kill(id[i],SIGUSR1)==-1) {perror("kill"); exit(1);}
    }
  }



void handle_sigusr2(int sig) {
    time_t current_time = time(NULL);  // get the current time
    int elapsed_time = (int) difftime(current_time, start_time);
    printf("[GATE=%d/PID=%d/TIME=%ds] The gates are %s!\n", gate, getpid(), elapsed_time, state == 't' ? "open" : "closed");

}

void handle_sigusr2parent(int sig) {
    for(int i=0; i<num_gates; i++){
        if(kill(id[i],SIGUSR2)==-1) { perror("kill"); exit(1);}
    }
}




int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <gates>\n", argv[0]);
        return 1;
    }
   
   
    gates_str = argv[1];
    num_gates = strlen(gates_str);
    for(int i=0; i<num_gates; i++) if(gates_str[i] != 't' && gates_str[i] != 'f') {printf("Usage: %s <'t' or 'f'>\n", argv[0]); return 1; }
    for(int i=0; i<num_gates; i++) {
         id[i] = fork();
        if(id[i]==-1) {
            perror("fork"); return 1;
        }
        else if(id[i]==0) {
            //child 
            gate=i;
            state= gates_str[i];
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
            

        }
        else {
            //parent
            if(signal(SIGCHLD, handle_sigchld) == SIG_ERR) { perror("signal"); return 1; }
            if(signal(SIGUSR1, handle_sigusr1parent) == SIG_ERR) { perror("signal"); return 1; }
            if(signal(SIGUSR2, handle_sigusr2parent) == SIG_ERR) { perror("signal"); return 1; }
            if(signal(SIGTERM, handle_sigterm_parent) == SIG_ERR) { perror("signal"); return 1; }
            printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getpid(), i, id[i], gates_str[i]);
            
            
        }
    }
     while(1) {
         pause();
     }
     return 0;  
}
    
  






