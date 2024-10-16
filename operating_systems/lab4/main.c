#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h> 
#include <stdbool.h>
#include <ctype.h>
#include <sys/select.h>  
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

bool is_digit_string(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        if (!isdigit(str[i]) && str[i]!=' ' && str[i]!='\n') {
            return false;
        }
        i++;
    }
    return true;
}

int main(int argc, char *argv[]) {
    bool debug = false;
    char *hostname, *port;
    int portnumber;
    if(argc == 4) { debug = true; strncpy(hostname, argv[1]+7, sizeof(argv[1])-7); strncpy(port, argv[2]+7, sizeof(argv[2])-7); portnumber = atoi(port); }
    else if(argc == 3 ) {
        if(strncmp(argv[2], "--debug", 7)== 0 ) {
            debug = true;
            if(strncmp(argv[1], "--host", 6) == 0 ) { strncpy(hostname, argv[1]+7, sizeof(argv[1])-7); portnumber = 18080; }
            else { strncpy(port, argv[1]+7, sizeof(argv[1])-7); portnumber = atoi(port);  hostname = "iot.dslab.pub.ds.open-cloud.xyz"; }
            }
        else { strncpy(hostname, argv[1]+7, sizeof(argv[1])-7); strncpy(port, argv[2]+7, sizeof(argv[2])-7); portnumber = atoi(port); }
        
    }
    else if(argc == 2) {
         if(strncmp(argv[1], "--debug", 7)== 0 ) { debug = true;  hostname = "iot.dslab.pub.ds.open-cloud.xyz";  portnumber = 18080; }
         else if(strncmp(argv[1], "--host", 6) == 0 ) { strncpy(hostname, argv[1]+7, sizeof(argv[1])-7); portnumber = 18080; }
         else { strncpy(port, argv[1]+7, sizeof(argv[1])-7); portnumber = atoi(port);  hostname = "iot.dslab.pub.ds.open-cloud.xyz";}
    }
    else { hostname = "iot.dslab.pub.ds.open-cloud.xyz";
           portnumber = 18080;
    }
 
    int domain = AF_INET;
    int type = SOCK_STREAM;

    int socket_fd=socket(domain,type,0);

    if (socket_fd<0) {
        perror("socket");
        return 1;
    }
    
    
    struct hostent *host;    
    host=gethostbyname(hostname);
    struct sockaddr_in sin; 
    if (host != NULL) {
    sin.sin_family = AF_INET;
    sin.sin_port = htons(portnumber);
    memcpy(&sin.sin_addr, host->h_addr_list[0], host->h_length);
    }
    else {
    perror("gethostbyname");
    return 1; }
     


    int is_connected=connect(socket_fd, &sin, sizeof(sin)); 

    if (is_connected<0) {

       perror("connect");
       return 1;
        
    }

    else {
       printf("Connected to HOST: %s\n", hostname);
    }
    
    while(1) {
        fd_set inset;
        int maxfd = 0;

        FD_ZERO(&inset);                // we must initialize before each call to select
       
   
        FD_SET(socket_fd, &inset);
        FD_SET(STDIN_FILENO, &inset);   // select will check for input from stdin
        maxfd = MAX(socket_fd, STDIN_FILENO) + 1; // update maxfd

       
        int ready_fds = select(maxfd, &inset, NULL, NULL, NULL);
        if (ready_fds <= 0) {
            perror("select");
            continue;                                       // just try again
        }

        // user has typed something, we can read from stdin without blocking
        if (FD_ISSET(STDIN_FILENO, &inset)) { 
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
            
            if(strncmp(buffer, "exit", 4)==0) {
                int check = shutdown(socket_fd, 2);
                if (check < 0) { perror("shutdown"); return 1; }
                close(socket_fd);
                exit(0);
            }
            
            else if(strncmp(buffer, "help", 4)==0) {
              printf("Available commands:\n");
              printf("* 'help'                    : Print this help message\n");
              printf("* 'exit'                    : Exit\n");
              printf("* 'get'                     : Retrieve sensor data\n");
              printf("* 'N name surname reason'   : Ask permission to go out\n");
            }
            
            else {
                if(write(socket_fd, buffer, n_read) < 0) { perror("write"); return 1; }
                if(debug) printf("[DEBUG] sent '%s'\n", buffer);
            }
        }
        // server has sent something 
        if (FD_ISSET(socket_fd, &inset)) {
            char buffer_server[101];
            int n_read_server = read(socket_fd, buffer_server, 100);
            if(n_read_server < 0) {
                perror("read");
                return 1;
                }
          
                buffer_server[n_read_server] = '\0';
                if (n_read_server > 0 && buffer_server[n_read_server-1] == '\n') {
                buffer_server[n_read_server-1] = '\0';
            }
                
                if(strncmp(buffer_server, "try again", 9)== 0) {
                printf("%s\n", buffer_server); 
               
                continue;
            }
            else if(strncmp(buffer_server, "ACK", 3)==0) {
                if(debug) printf("[DEBUG] read '%s'\n", buffer_server);
                printf("Response: '%s'\n", buffer_server);
               
            }
            else if(is_digit_string(buffer_server)) {
                
                if(debug) printf("[DEBUG] read '%s'\n", buffer_server);
                printf("---------------------------\n"); printf("Latest event:\n");
                 char interval [10];
                 char temperature [10];
                 char light [10];
                 char time [20];
                 char time_str[20];
            strncpy(interval,buffer_server,1);
            if (atoi(interval)==0){
              printf("boot (0)\n");

              }
            else if (atoi(interval)==1){
              printf("setup (1)\n");

              }
            else if (atoi(interval)==2){
              printf("interval (2)\n");

              }
            else if (atoi(interval)==3){
              printf("button (3)\n");

              }
            else if (atoi(interval)==4){
              printf("motion (4)\n");
              }

            strncpy(temperature,buffer_server +6,4);
            printf("Temperature is : %.2f\n",atof(temperature)/100.0);
            
            strncpy(light,buffer_server +2,3);
            printf("Light level is: %d\n",atoi(light) );
            
            strncpy(time,buffer_server+11,10);
            time_t time1 = atoi(time); 
            struct tm *info;
          
            info = localtime( &time1 );
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", info);
            printf("Timestamp is: %s\n", time_str);
           
            
            }
            else {
                
                if(debug) printf("[DEBUG] read '%s'\n", buffer_server);
                printf("Send verification code: '%s'\n", buffer_server);
               
            }
            
        }
    }

    
    close(socket_fd);
    return 0;
}




