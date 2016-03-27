#include <fcntl.h>     
#include <netinet/in.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <string.h>
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
    int plain_text_length;
    int port;
    socklen_t clilen;

    struct sockaddr_in server;
    struct sockaddr_in client;

    // verify args
    if (argc < 2){
        printf("Usage: otp_enc_d port\n");
        exit(1);
    }

    // check port num
    sscanf(argv[1], "%d", &port);
    if (port < 0 || port > 65535){
        printf("otp_enc_d: invalid port\n");
        exit(2);
    }

    // make socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Error: opt_enc_d could not create socket\n");
        exit(1);
    }

    memset(&server, '\0', sizeof(server));

    // set address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // bind socket to port
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        printf("Error: otp_enc_d unable to bind socket to port %d\n", port);
        exit(2);
    }

    // listen for connections
    if (listen(sockfd, 5) == -1){
        printf("Error: otp_enc_d unable to listen on port %d\n", port);
        exit(2);
    }

    clilen = sizeof(client);

    // accept connections
    while (1){
        newsockfd = accept(sockfd, (struct sockaddr *) &client, &clilen);
        if (newsockfd < 0){
            printf("Error: opt_enc_d unable to accept connection\n");
            continue;
        }

        // when client connects spawn off that process
        pid = fork();

        if (pid < 0){
            perror("opt_enc_c: error on fork\n");
        }

        // child process
        if (pid == 0){
 
            // zero buffer
            memset(buff1, 0, BUFFER_SIZE);

            plain_text_length = read(newsockfd, buff1, BUFFER_SIZE);
            if (plain_text_length < 0){
                printf("Error: otp_end_d could not read plaintext on port %d\n", port);
                exit(2);
            }

            // send message to client
            num_sent = write(newsockfd, "!", 1);
            
            if (num_sent < 0){
                printf("otp_enc_d error sending acknowledgement to client\n");
                exit(2);
            }

            // zero buffer
            memset(buff2, 0, BUFFER_SIZE);

            // get key from otp_enc
            key_length = read(newsockfd, buff2, BUFFER_SIZE);
            
            if (key_length < 0){
                printf("Error: otp_end_d could not read key on port %d\n", port);
                exit(2);
            }

            // validate plaintext
            for (i = 0; i < plain_text_length; i++){
                if ((int) buff1[i] > 90 || ((int) buff1[i] < 65 && (int) buff1[i] != 32)){
                    printf("otp_enc_d error: plaintext contains bad characters\n");
                    exit(1);
                }
            }

            // validate key
            for (i = 0; i < key_length; i++){
                if ((int) buff2[i] > 90 || ((int) buff2[i] < 65 && (int) buff2[i] != 32)){
                    printf("otp_enc_d error: key contains bad characters\n");
                    exit(1);
                }
            }

            // compare key and plaintext
            if (key_length < plain_text_length){ 
                printf("otp_enc_d error: key is too short\n");
                exit(1);
            }

            // create ciphertext
            for (i = 0; i < plain_text_length; i++){
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

                // combine key and message using modular addition
                int cipher = (msg + key) % 27;
                cipher = cipher + 64;

                // type conversion back to char
                buff3[i] = (char) cipher + 0;

                // change asterisks to spaces
                if (buff3[i] == '@'){
                    buff3[i] = ' ';
                }
            }

            // send ciphertext to otp_enc
            num_sent = write(newsockfd, buff3, plain_text_length);
            if (num_sent < plain_text_length){
                printf("otp_enc_d error writing to socket\n");
                exit(2);
            }

            // close sockets
            close(newsockfd);
            close(sockfd);

            exit(0);
        }
        else close(newsockfd);
    } 

    return 0;
}