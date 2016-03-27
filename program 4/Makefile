CC = gcc
CFLAGS = -g -Wall -Werror
BIN = keygen otp_enc otp_enc_d otp_dec otp_dec_d

all: keygen otp_enc otp_enc_d otp_dec otp_dec_d

default: keygen otp_enc otp_enc_d otp_dec otp_dec_d

keygen: 
	$(CC) $(CFLAGS) -o keygen keygen.c

otp_enc: otp_enc.o
	$(CC) $(CFLAGS) -o otp_enc otp_enc.o 

otp_enc_d: otp_enc_d.o 
	$(CC) $(CFLAGS) -o otp_enc_d otp_enc_d.o 

otp_dec: otp_dec.o
	$(CC) $(CFLAGS) -o otp_dec otp_dec.o 

otp_dec_d: otp_dec_d.o
	$(CC) $(CFLAGS) -o otp_dec_d otp_dec_d.o 

otp_enc.o:
	$(CC) $(CFLAGS) -c otp_enc.c

otp_enc_d.o:
	$(CC) $(CFLAGS) -c otp_enc_d.c

otp_dec.o:
	$(CC) $(CFLAGS) -c otp_dec.c

otp_dec_d.o:
	$(CC) $(CFLAGS) -c otp_dec_d.c

clean:
	rm -f *.o $(BIN)

