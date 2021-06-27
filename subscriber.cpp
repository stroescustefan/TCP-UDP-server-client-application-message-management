#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"
#include <netinet/tcp.h>

/**
 * Functie care creeaza un mesaj si returneaza daca comanda
 * valida este una corecta sau nu.
 * @param msg - mesajul care se construieste
 * @param array - sirul primit care se transforma in mesaj
 * @return  - 1(daca am primit o comanda valida), 0 (altfel)
 */

int create_message(srv_msg *msg, char array[]) {

	// determinare numar spatii din mesaj
	int count_spaces = 0;
	for (int i = 0; i < strlen(array); i++) {
		if (array[i] == ' ') {
			count_spaces++;
		}
	}

	// extragerea primului cuvant (sir delimitat prin spatiu)
	char *p = strtok(array, " ");
	if (p == nullptr) {
		return 0;
	} else {
		// verificare existenta subscribe / unsubscribe
		if (strcmp(p, "subscribe") == 0) {
			msg->type = 's';

		} else if (strcmp(p, "unsubscribe") == 0) {
			msg->type = 'u';

		} else {
			return 0;
		}
	}

	// verificare numar spatii in functie de tipul mesajului
	if (msg->type == 's' && count_spaces != 2) {
		return 0;
	}
	if (msg->type == 'u' && count_spaces != 1) {
		return 0;
	}

	// extragerea celui de-al doilea cuvant
	// pentru topic, verificare + copiere in mesaj
	p = strtok(nullptr, " ");
	if (p == nullptr) {
		return 0;
	} else {

		// verificare dimensiune topic
		if (strlen(p) > 50) {
			return 0;
		} else {
			strcpy(msg->topic_name, p);
		}
	}

	// extragerea celui de-al treilea cuvant
	p = strtok(nullptr, " ");
	if (msg->type == 'u' && p != nullptr) {
		return 0;
	}

	if (msg->type == 's' && p == nullptr) {
		return 0;
	}
	if (msg->type == 's' && p != nullptr) {

		// verificare corectitudine sf pentru subscribe
		if (p[0] != '0' && p[0] != '1' ) {
			return 0;
		} else {
			if (p[1] == '\0') {
				msg->sf = p[0];
			} else {
				return 0;
			}
		}
	}

	return 1;
}

void usage(char *file) {
	fprintf(stderr, "Usage: %s client_ID server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, nullptr, _IONBF, BUFSIZ);
	int sockfd, ret;
	struct sockaddr_in serv_addr;
	char buffer_read[BUFF_READ];
	char buffer_recv[BUFLEN];


	if (argc < 4) {
		usage(argv[0]);
	}


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	fd_set read_set, tmp_set;

    FD_ZERO(&read_set);
    FD_ZERO(&tmp_set);

    FD_SET(STDIN_FILENO, &read_set);
    FD_SET(sockfd, &read_set);

    int maxfd = sockfd;

	// trimitere ID client TCP la connectare.
	char acceptance[3];
	int n = send(sockfd, argv[1], strlen(argv[1]) + 1, 0);
	DIE(n < 0, "send TCP 1");

	// primire OK(nu exista) sau NO(exista), daca exista sau nu
	// un client connectat cu acelasi ID.
	recv(sockfd, acceptance, 3, 0); 
	DIE(n < 0, "recv TCP 1");
	if (strcmp(acceptance, "NO") == 0) {
		close(sockfd);
		exit(0);
	}

	int flag = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));

	while(1) {
	    tmp_set = read_set;

	    int r = select(maxfd + 1, &tmp_set, NULL, NULL, NULL);
	    DIE(r < 0, "Select");

	    for (int i = 0; i <= maxfd; i++) {
	        if (FD_ISSET(i, &tmp_set)) {
	            if (i == STDIN_FILENO) {
					srv_msg sent_msg;

					memset(buffer_read, 0, BUFF_READ);
                    fgets(buffer_read, BUFF_READ, stdin);
					buffer_read[strlen(buffer_read) - 1] = '\0';

					// daca am introdus comanda exit => inchidem clientul
					if (strcmp(buffer_read, "exit") == 0) {
						close(sockfd);
						exit(0);

					// daca am introdus o comanda de subscribe/ unsubscribe valida
					// se construieste mesajul si printam pe client Subscribed/ Unsubscribed
					// to topic
					} else if (create_message(&sent_msg, buffer_read)) {
						int n = send(sockfd, (char*) &sent_msg, sizeof(sent_msg), 0);
						DIE(n < 0, "send msg to server");

                		if (sent_msg.type == 's') {
                    		printf("Subscribed to topic.\n");
                		} else {
                    		printf("Unsubscribed from topic.\n");
                		}

					} else {
						printf("Incorrect command.\n");
					}

	            } else if (i == sockfd) {
					memset(buffer_recv, 0, BUFLEN);

					// s-a primit un mesaj de la server
                    int n = recv(i, buffer_recv, sizeof(tcp_msg), 0);
					DIE(n < 0, "primire mesaj pentru TCP");
					int total = sizeof(tcp_msg) - n;

					// serverul a inchis conexiunea cu clientul
					if (n == 0) {
						close(sockfd);
						exit(0);
					}

					// verificam primirea mesajelor de dimensiunea asteptata
					while (total > 0) {
						n = recv(i, buffer_recv + (sizeof(tcp_msg) - total), total, 0);
						DIE(n < 0, "primire mesaj pentru TCP");
						total -= n;

					}

					tcp_msg *recv_msg;
					recv_msg = (tcp_msg*)buffer_recv;

					// printam mesajul
					printf("%s:%s - %s - %s - %s\n", recv_msg->ip, recv_msg->udp_port,
						recv_msg->topic_name, recv_msg->type, recv_msg->data);
	            }
	        }
	    }
	}

	close(sockfd);
	return 0;
}