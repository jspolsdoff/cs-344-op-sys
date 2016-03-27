#include <arpa/inet.h>
#include <fcntl.h>     
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>     
#include <stdlib.h>   
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE    128000

int main(int argc, char** argv){
    
    char buff1[BUFFER_SIZE], buff2[BUFFER_SIZE], buff3[1];
    int fd;
    int i;
    int key_length;
    int num_received, num_sent;
    int ciper_text_length;
    int port;
    int sockfd;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    // make sure there are enough args
    if (argc < 4){
        printf("Usage: otp_dec ciphertext key port\n");
        exit(1);
    }

    // check port num
    sscanf(argv[3], "%d", &port);
    if (port < 0 || port > 65535){
        printf("otp_dec: invalid port\n");
        exit(2);
    }

    // open ciphertext for reading
    fd = open(argv[1], O_RDONLY);

    // if can't read print error
    if (fd < 0){
        printf("Error: cannot open ciphertext file %s\n", argv[1]);
        exit(1);
    }

    // make sure keep track of num of bytes read
    ciper_text_length = read(fd, buff1, BUFFER_SIZE);

    // validate contents of ciphertext
    for (i = 0; i < ciper_text_length - 1; i++){
        if ((int) buff1[i] > 90 || ((int) buff1[i] < 65 && (int) buff1[i] != 32)){
            printf("otp_dec error: ciphertext contains bad characters\n");
            exit(1);
        }
    }

    close(fd);

    // open key for reading
    fd = open(argv[2], O_RDONLY);

    // if can't read display error
    if (fd < 0){
        printf("Error: cannot open key file %s\n", argv[2]);
        exit(1);
    }

    // make sure keep track num of bytes read
    key_length = read(fd, buff2, BUFFER_SIZE);

    // validate contents of key
    for (i = 0; i < key_length - 1; i++){
        if ((int) buff2[i] > 90 || ((int) buff2[i] < 65 && (int) buff2[i] != 32)){
            printf("otp_dec error: key contains bad characters\n");
            exit(1);
        }
    }

    close(fd);

    // compare length of ciphertext to that of key
    if (key_length < ciper_text_length){
        printf("Error: key '%s' is too short\n", argv[2]);
    }

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("Error: could not contact otp_dec_d on port %d\n", port);
        exit(2);
    }

    memset(&serv_addr, '\0', sizeof(serv_addr));

    server = gethostbyname("localhost");
    if (server == NULL){
        printf("Error: could not connect to otp_dec_d\n");
        exit(2);
    }    
  
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);         

    serv_addr.sin_port = htons(port);

    // connect to otp_dec_d
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("Error: could not connect to otp_dec_d on port %d\n", port);
        exit(2);
    }

    // send ciphertext to otp_enc_d
    num_sent = write(sockfd, buff1, ciper_text_length - 1);
    if (num_sent < ciper_text_length - 1){
        printf("Error: could not send ciphertext to otp_dec_d on port %d\n", port);
        exit(2);
    }

    memset(buff3, 0, 1);

    // get acknowledgement from server
    num_received = read(sockfd, buff3, 1);
    if (num_received < 0){
       printf("Error receiving acknowledgement from otp_dec_d\n");
       exit(2);
    }

    // send key to otp_dec_d
    num_sent = write(sockfd, buff2, key_length - 1);
    if (num_sent < key_length - 1){
        printf("Error: could not send key to otp_dec_d on port %d\n", port);
        exit(2);
    }

    memset(buff1, 0, BUFFER_SIZE);

    do{
        // receive ciphertext from otp_dec_d
        num_received = read(sockfd, buff1, ciper_text_length - 1);
    }
    while (num_received > 0);

    if (num_received < 0){
       printf("Error receiving ciphertext from otp_dec_d\n");
       exit(2);
    }

    // output ciphertext to stdout
    for (i = 0; i < ciper_text_length - 1; i++){
        printf("%c", buff1[i]);
    }

    printf("\n");

    // close socket
    close(sockfd);

    return 0;
}