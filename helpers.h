#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>

/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

// structura pentru mesaj subscribe/ unsubscribe
struct __attribute__((packed)) srv_msg{
    char type;
    char topic_name[51];
    char sf;
};

// structura pentru mesaj trimis de server catre clientii TCP
struct __attribute__((packed)) tcp_msg{
    char ip[16];
    char udp_port[10];
    char topic_name[51];
    char type[11];
    char data[1501];
};




#define BUFLEN (sizeof(tcp_msg))	// dimensiune mesaj pentru TCP

#define BUFF_READ 256               // dimensiune pentru mesajele citite
                                    // de la tastatura de clientii TCP

#endif
