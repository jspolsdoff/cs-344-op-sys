#include <stdio.h>    
#include <stdlib.h>    
#include <string.h>
#include <fcntl.h>     
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE    128000

int main(int argc, char** argv){

    char buff1[BUFFER_SIZE], buff2[BUFFER_SIZE], buff3[BUFFER_SIZE];
    int sockfd;
    int newsockfd;
    int i;
    int key_length;
    int num_sent;
    int pid;
    int cip_text_length;
    int port;
    socklen_t clilen;

    struct sockaddr_in server;
    struct sockaddr_in client;

    // make sure enough arg
    if (argc < 2){
        printf("Usage: otp_dec_d port\n");
        exit(1);
    }

    // validate the port
    sscanf(argv[1], "%d", &port);
    if (port < 0 || port > 65535){
        printf("otp_dec_d: invalid port\n");
        exit(2);
    }

    // make socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Error: otp_dec_d could not create socket\n");
        exit(1);
    }

    // take care of IP address mem space
    memset(&server, '\0', sizeof(server));

    // set up an address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // bind socket to a port
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        printf("Error: otp_dec_d unable to bind socket to port %d\n", port);
        exit(2);
    }

    // start to listen for connection
    if (listen(sockfd, 5) == -1){
        printf("Error: otp_dec_d unable to listen on port %d\n", port);
        exit(2);
    }

    clilen = sizeof(client);

    // accept connections
    while (1){
        newsockfd = accept(sockfd, (struct sockaddr *) &client, &clilen);
        if (newsockfd < 0){
            printf("Error: otp_dec_d unable to accept connection\n");
            continue;
        }

        // when client connects spawn off that process
        pid = fork();

        if (pid < 0){
            perror("otp_dec_c: error on fork\n");
        }

        // child process
        if (pid == 0){

            // zero out buffer
            memset(buff1, 0, BUFFER_SIZE);

            // receive ciphertext from otp_dec
            cip_text_length = read(newsockfd, buff1, BUFFER_SIZE);
            if (cip_text_length < 0){
                printf("Error: otp_end_d could not read ciphertext on port %d\n", port);
                exit(2);
            }

            // send message to client
            num_sent = write(newsockfd, "!", 1);
            
            if (num_sent < 0){
                printf("otp_dec_d error sending acknowledgement to client\n");
                exit(2);
            }

            // zero out buffer
            memset(buff2, 0, BUFFER_SIZE);

            // receive key from otp_dec
            key_length = read(newsockfd, buff2, BUFFER_SIZE);
            
            if (key_length < 0){
                printf("Error: otp_dec_d could not read key on port %d\n", port);
                exit(2);
            }

            // validate ciphertext
            for (i = 0; i < cip_text_length; i++){
                if ((int) buff1[i] > 90 || ((int) buff1[i] < 65 && (int) buff1[i] != 32)){
                    printf("otp_dec_d error: ciphertext contains bad characters\n");
                    exit(1);
                }
            }

            // validate key
            for (i = 0; i < key_length; i++){
                if ((int) buff2[i] > 90 || ((int) buff2[i] < 65 && (int) buff2[i] != 32)){
                    printf("otp_dec_d error: key contains bad characters\n");
                    exit(1);
                }
            }

            // compare cipher and key
            if (key_length < cip_text_length){ 
                printf("otp_dec_d error: key is too short\n");
                exit(1);
            }

            // decrypt ciphertext
            for (i = 0; i < cip_text_length; i++)
            {
                // change spaces to asterisks
                if (buff1[i] == ' '){
                    buff1[i] = '@';
                }
                if (buff2[i] == ' '){
                    buff2[i] = '@';
                }

                // conversion to int
                int msg = (int) buff1[i];
                int key = (int) buff2[i];

                msg = msg - 64;
                key = key - 64;

                // use mod sub to combine
                int decrypted = msg - key;
                
                if (decrypted < 0) {
                    decrypted = decrypted + 27;
                }

                // add 64 back to that range is 64 - 90
                decrypted = decrypted + 64;

                //back to char
                buff3[i] = (char) decrypted + 0;

                if (buff3[i] == '@'){
                    buff3[i] = ' ';
                }
            }

            // send to otp_enc
            num_sent = write(newsockfd, buff3, cip_text_length);
            
            if (num_sent < cip_text_length){
                printf("otp_dec_d error writing to socket\n");
                exit(2);
            }

            close(newsockfd);
            close(sockfd);

            exit(0);
        }

        else close(newsockfd);
    } 

    return 0;
}