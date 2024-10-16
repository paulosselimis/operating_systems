#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>

int main (int argc, char* argv[]) {
    if(argc != 2) {
         printf("Usage: ./a.out filename\n");
         return 1;
} //ΚΑΝΕΝΑ Η ΔΥΟ Η ΠΕΡΙΣΣΟΤΕΡΑ ΟΡΙΣΜΑΤΑ
    
    struct stat buffer;
    if(stat(argv[1], &buffer) == 0) {
        printf("Error: %s already exists\n", argv[1]);
        return 1;
} //ΤΟ ΑΡΧΕΙΟ ΥΠΑΡΧΕΙ ΗΔΗ

    if(strcmp(argv[1], "--help")==0) {
        printf("Usage: ./a.out filename\n");
        return 0;
    } //ΚΛΗΣΗ ΜΕ flag --help

    int status;
    int id;
    id = fork();
    if(id<0) {
        perror("fork");
        return 1;
    } //ΕΛΕΓΧΟΣ ΛΑΘΟΥΣ ΣΤΟ fork
    
    //ΚΩΔΙΚΑΣ ΠΑΙΔΙΟΥ:
    if(id==0){
        pid_t Cpid=getpid();
        pid_t Ppid=getppid();
        char msg[50];
        sprintf(msg, "[CHILD] getpid()= %d, getppid()=%d\n", Cpid, Ppid);
        int fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY, 0644);
        if (fd == -1) { 
          perror("open"); 
          return 1; 
        } //ΕΛΕΓΧΟΣ ΛΑΘΟΥΣ ΣΤΟ open
        if (write(fd, msg, strlen(msg)) < strlen(msg)) { 
          perror("write"); 
          return 1; 
        } //ΚΑΝΕΙ write ΚΑΙ ΕΛΕΓΧΕΙ ΓΙΑ ΛΑΘΟΣ ΣΤΟ write
        
        close(fd);
        exit(0);
    }
    //ΚΩΔΙΚΑΣ ΠΑΤΕΡΑ:
    else {
       
        if(wait(&status)==-1) {
            perror("wait");
            return 1;
        } //ΚΑΝΕΙ wait ΚΑΙ ΕΛΕΓΧΕΙ ΓΙΑ ΛΑΘΟΣ ΣΤΟ wait
        pid_t Mpid=getpid();
        pid_t Ppid=getppid();
        char msg[50];
        sprintf(msg, "[PARENT] getpid()= %d, getppid()=%d\n", Mpid, Ppid);
        int fd = open(argv[1], O_APPEND | O_WRONLY, 0644);
        if (fd == -1) { 
          perror("open"); 
          return 1; 
        } //ΕΛΕΓΧΟΣ ΛΑΘΟΥΣ ΣΤΟ open
        if (write(fd, msg, strlen(msg)) < strlen(msg)) { 
          perror("write"); 
          return 1; 
        } //ΚΑΝΕΙ write ΚΑΙ ΕΛΕΓΧΕΙ ΓΙΑ ΛΑΘΟΣ ΣΤΟ write
        close(fd);
        exit(0);
    }
    
 
    return 0;
}

