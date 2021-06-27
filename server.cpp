#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "helpers.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <limits.h>
#include <vector>
using namespace std;

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}


/**
 * Functie care converteste un string in sir de caractere.
 * 
 * @param - primeste un string
 * @return - returneaza un array de caractere.
 */ 
char *convertFromString(string myString) {
	char *arrayFromString = (char *)malloc((myString.length() + 1) * sizeof(char));

	int i = 0;
	for (i = 0; i < myString.length(); i++) {
		arrayFromString[i] = myString[i];
	}

	arrayFromString[i] = '\0';
	return arrayFromString;
}


/**
 * Functie care converteste un sir de caractere intr-un string.
 * 
 * @param array - primeste un sir de caractere.
 * @return - returneaza un string.
 */
string convertToString(char* array) {
    int i, size;
    string string_convert = "";

	size = strlen(array);
    for (i = 0; i < size; i++) {
        string_convert = string_convert + array[i];
    }
    return string_convert;
}


/**
 * Functie care numara cifrele unui numar.
 * 
 * @param number - un numar
 * @return - numarul de cifre
 */ 
unsigned int countDigits(int number) {
	int cnt;

	cnt = 0;
	while (number) {
		cnt++;
		number /= 10;
	}

	return cnt;
}


/**
 * Functie care converteste un numar(int) in sir de caractere
 * 
 * @param value - numarul ce urmeaza a fi convertit
 * @param continut - vectorul in care se converteste numarul
 */ 
void convertIntToCharArray(int value, char continut[]) {
	int digits, size;

	digits = countDigits(value);
	
	if (value == 0) {
		continut[0] = '0';
		continut[1] = '\0';

	} else if (value > 0) {
		size = digits - 1;
		while (value) {
			continut[size] = (value % 10) + '0';
			value /= 10;
			size--;
		}
		continut[digits] = '\0';

	} else {
		size = digits;
		while (value) {
			continut[size] = (-1) * (value % 10) + '0';
			value /= 10;
			size--;
		}
		continut[size] = '-';
		continut[digits + 1] = '\0';
	}

}


/**
 * Functie care converteste un float in sir de caractere.
 * 
 * @param value - numarul
 * @param power - puterea lui 1/10 cu care trebuie inmultit numarul
 * @param continut - vectorul in care se converteste numarul
 */ 
void convertFloatToCharArray(int value, int power, char continut[]) {
	int digits, size;

	digits = countDigits(value);

	// daca numarul e 0
	if (value == 0) {
		continut[0] = '0';
		continut[1] = '\0';

	// daca valoarea e pozitiva
	} else if (value > 0) {

		// daca numarul de cifre > puterea
		if (digits > power) {
			size = digits;
			if (power == 0) {
			    size--;
			}
			while (value) {

				// locul unde adaugam punct "."
				if (size == digits - power && power != 0) {
					continut[size--] = '.';
				}
				continut[size--] = (value % 10) + '0';
				value = value / 10;		
			}
			if (power == 0) {
			    continut[digits] = '\0';
			} else {
			    continut[digits + 1] = '\0';
			}

		// daca numarul de cifre <= puterea
		// ex: nr = 123, pow = 4 => 0.0123 
		} else {
			size = 2 + (power - digits) + digits - 1;
			continut[0] = '0';
			continut[1] = '.';
			while (value) {
				continut[size--] = (value % 10) + '0';
				value = value / 10;		
			}
			for (int i = 1; i <= power - digits; i++) {
				continut[size--] = '0';
			}
			continut[2 + (power - digits) + digits] = '\0';
		}

	} else {

		// analog ca la valoare pozitiva
		// singura diferenta e ca adaugam un minus
		if (digits > power) {
			size = digits + 1;
			if (power == 0) {
			    size--;
			}
			while (value) {
				if (size == digits - power + 1) {
					continut[size--] = '.';
				}
				continut[size--] = (-1) * (value % 10) + '0';
				value = value / 10;		
			}
			continut[size] = '-';
			if (power == 0) {
			    continut[digits + 1] = '\0';
			} else {
			    continut[digits + 2] = '\0';
			}
			
		} else {
			size = 2 + (power - digits) + digits;
			continut[0] = '-';
			continut[1] = '0';
			continut[2] = '.';
			while (value) {
				continut[size--] = (-1) * (value % 10) + '0';
				value = value / 10;		
			}
			for (int i = 1; i <= power - digits; i++) {
				continut[size--] = '0';
			}
			continut[2 + (power - digits) + digits + 1] = '\0';
		}
	}
}


/**
 * Functie care parseaza un mesaj venit de la un client UDP si il adauga in msg.
 * 
 * @param udp_buffer - mesajul venit de la udp.
 * @param msg - mesajul construit ce urmeaza sa fie trimis clientilor TCP
 * @param udp_port - portul udp.
 * @param udp_ip - ip-ul
 */
void create_tcp_mesage(char udp_buffer[], tcp_msg *msg, uint16_t udp_port, char udp_ip[]) {
	char topic[51];
	char data_type;

	// parsare topic
	memset(topic, 0, 51);
	int index = 0;
	while (udp_buffer[index]) {
		topic[index] = udp_buffer[index];
		index++;
	}
	topic[index] = '\0';
	strcpy(msg->topic_name, topic);

	// parsare tip date
	data_type = udp_buffer[50] + '0';

	// parsare continut
	char content[1501];
	memset(content, 0, 1501);

	if (data_type == '0') {
		int value = int((unsigned char)(udp_buffer[52]) << 24 |
						(unsigned char)(udp_buffer[53]) << 16 |
						(unsigned char)(udp_buffer[54]) << 8 |
						(unsigned char)(udp_buffer[55]));
		if (udp_buffer[51] == 1) {
			value = value * (-1);
		}
		strcpy(msg->type, "INT");
		convertIntToCharArray(value, content);

	} else if (data_type == '1') {
		uint16_t value = (uint16_t)((unsigned char)(udp_buffer[51]) << 8 |
									(unsigned char)(udp_buffer[52]));
		strcpy(msg->type, "SHORT_REAL");
		convertFloatToCharArray(value, 2, content);

	} else if (data_type == '2') {
		int value = int((unsigned char)(udp_buffer[52]) << 24 |
						(unsigned char)(udp_buffer[53]) << 16 |
						(unsigned char)(udp_buffer[54]) << 8 |
						(unsigned char)(udp_buffer[55]));

		if (udp_buffer[51] == 1) {
			value = value * (-1);
		}
		strcpy(msg->type, "FLOAT");
		convertFloatToCharArray(value, udp_buffer[56], content);

	} else {
		int index = 51;
		int contentSize = 0;
		while (udp_buffer[index]) {
			content[contentSize] = udp_buffer[index];
			contentSize++;
			index++;
		}
		strcpy(msg->type, "STRING");
		content[contentSize] = '\0';
	}
	strcpy(msg->data, content);
	strcpy(msg->ip, udp_ip);
	convertIntToCharArray(udp_port, msg->udp_port);
}


int main(int argc, char *argv[]) {
	setvbuf(stdout, nullptr, _IONBF, BUFSIZ);
	int sockfd, newsockfd, portno, sockfdUDP;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	// retine pentru un socket clientul care e conectat pe el(ID-ul lui).
	unordered_map<int, string> sock_ID_Map;

	// retine pentru un client(ID), socketul pe care acesta e conectat.
	unordered_map<string, int> id_sock_map;

	// map intre topic si clientii abonati la el (ID-urile lor).
	unordered_map<string, unordered_set<string>> topic_clients;

	// map intre client(ID) si mesajele care trebuie trimise(daca e cazul)
	// cat timp a fost offline.
	unordered_map<string, vector<tcp_msg>> offline_msg; 

	// map intre client(ID) si topicurile cu sf-urile 1.
	unordered_map<string, unordered_set<string>> clients_sf;

	if (argc < 2) {
		usage(argv[0]);
	}


	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	// socketul TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	//socketul UDP
	sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfdUDP < 0, "socket UDP");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	// se completeaza informatiile despre socketul UDP si TCP.
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// bind TCP si UDP
	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");
	ret = bind(sockfdUDP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind UDP");


	ret = listen(sockfd, INT_MAX);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	FD_SET(sockfdUDP, &read_fds);
	if (sockfd > sockfdUDP) {
		fdmax = sockfd;
	} else {
		fdmax = sockfdUDP;
	}


	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");
					int flag = 1;
                    setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));

					// primim ID-ul clientului TCP
					char ID[11];
					memset(ID, 0, 11);
					n = recv(newsockfd, ID, sizeof(ID), 0);
					DIE(n < 0, "recv");

					// verificam daca exista un client cu acelasi ID conectat.
					int check_id_exist = 0;
					if (id_sock_map.find(convertToString(ID)) != id_sock_map.end()) {
						check_id_exist = 1;
					} else {
						id_sock_map[convertToString(ID)] = newsockfd;
					}

					// nu exista un client cu ID-ul primit.
					if (!check_id_exist) {

						// trimitem accept (OK).
						n = send(newsockfd, "OK", 3, 0);
						DIE(n < 0, "send OK to TCP");
						sock_ID_Map[newsockfd] = convertToString(ID);

						// daca exista mesaje netrimise pentru client
						// in cazul in care clientul s-a abonat cu SF = 1
						if (offline_msg.find(convertToString(ID)) != offline_msg.end()) {
							for (tcp_msg off_msg : offline_msg[convertToString(ID)]) {
								n = send(newsockfd, (char*) &off_msg, sizeof(off_msg), 0);
								DIE(n < 0, "send to TCP");		
							}

							// stergem entry-ul din map, ca sa nu le trimitem de mai multe ori
							offline_msg.erase(convertToString(ID));
						}

						// se adauga noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
						printf("New client %s connected from %s:%d.\n",
					 		ID, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

					} else {
						printf("Client %s already connected.\n", ID);

						// trimitem refuse(NO)
						n = send(newsockfd, "NO", 3, 0);
						DIE(n < 0, "send NO to TCP");
					}

				// citire comanda pe server
				} else if (i == STDIN_FILENO) {
					char exitCommand[5];

					fgets(exitCommand, 5, stdin);
					exitCommand[4] = '\0';
					if (strcmp(exitCommand, "exit") == 0) {
						// inchidere socket-uri
						for (int j = 0; j <= fdmax; j++) {
							if (j != sockfd && j != sockfdUDP && j != STDIN_FILENO && FD_ISSET(j, &read_fds)) {
								close(j);
								FD_CLR(j, &read_fds);
							}
						}
						close(STDIN_FILENO);
						close(sockfd);
						close(sockfdUDP);
						exit(0);
					}

				// primire mesaj de la client UDP
				} else if (i == sockfdUDP) {
					char udp_buffer[1553];
					tcp_msg tcp_sent_msg;


					// primire payload
					memset(udp_buffer, 0, 1553);
					clilen = sizeof(cli_addr);
					n = recvfrom(i, udp_buffer, sizeof(udp_buffer), 0,
                         (struct sockaddr*)&cli_addr, &clilen);
					DIE(n < 0, "recv UDP");
					memset(&tcp_sent_msg, 0, BUFLEN);

					//parsare si construire mesaj
					create_tcp_mesage(udp_buffer, &tcp_sent_msg, ntohs(cli_addr.sin_port), inet_ntoa(cli_addr.sin_addr));

					// daca exista clienti abonati la topic
					if (topic_clients.find(convertToString(tcp_sent_msg.topic_name)) != topic_clients.end()) {
						for (string myID : topic_clients[convertToString(tcp_sent_msg.topic_name)]) {

							// daca este conectat clientul TCP.
							if (id_sock_map.find(myID) != id_sock_map.end()) {
								n = send(id_sock_map[myID], (char*) &tcp_sent_msg, sizeof(tcp_sent_msg), 0);
								DIE(n < 0, "send to TCP");
							} else {
								// daca clientul e deconectat si are topicuri la care e abonat cu SF = 1
								if (clients_sf[myID].find(convertToString(tcp_sent_msg.topic_name)) != clients_sf[myID].end()) {
									offline_msg[myID].push_back(tcp_sent_msg);
								}
							}
						}
					}

				// s-au primit date pe unul din socketii de client TCP,
				// asa ca serverul trebuie sa le receptioneze
				} else {
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(srv_msg), 0);
					DIE(n < 0, "recv subscribe/ unsubscribe");

					if (n == 0) {

						// conexiunea s-a inchis
						printf("Client %s disconnected.\n", convertFromString(sock_ID_Map[i]));
						id_sock_map.erase(sock_ID_Map[i]);
						sock_ID_Map.erase(i);
						close(i);
                        FD_CLR(i, &read_fds);

					} else {
						int total = sizeof(srv_msg) - n;
						while (total > 0) {
							n = recv(i, buffer + (sizeof(srv_msg) - total), total, 0);
							DIE(n < 0, "recv subscribe/ unsubscribe");
							total -= n;

						}
						srv_msg *message = (srv_msg*)buffer;

						// daca  mesajul e de tip subscribe
						if (message->type == 's') {
							if (message->sf == '1') {
								clients_sf[sock_ID_Map[i]].insert(convertToString(message->topic_name));

							// daca un client era abonat la topicul "x" cu SF = 1 si se aboneaza dupa cu SF = 0
							// atunci topicul "x" trebuie sters din topicurile cu SF = 1 ale clientului	
							} else {
								if (clients_sf[sock_ID_Map[i]].find(convertToString(message->topic_name)) != clients_sf[sock_ID_Map[i]].end()) {
									clients_sf[sock_ID_Map[i]].erase(convertToString(message->topic_name));
								}
							}
							// adaugam un nou client la topic
							topic_clients[convertToString(message->topic_name)].insert(sock_ID_Map[i]);

						} else {
							// daca primim unsubscribe pe un topic cu SF = 1 il stergem si
							// din map-ul cu topicuri si din map-ul cu topicuri de 1.

							if (topic_clients[convertToString(message->topic_name)].size() == 0) {
								topic_clients.erase(convertToString(message->topic_name));
							}

							
							if (clients_sf[sock_ID_Map[i]].find(convertToString(message->topic_name)) != clients_sf[sock_ID_Map[i]].end()) {
								if (clients_sf[sock_ID_Map[i]].size() == 0) {
									clients_sf.erase(sock_ID_Map[i]);
								} else {
									clients_sf[sock_ID_Map[i]].erase(convertToString(message->topic_name));
								}		
							}
							topic_clients[convertToString(message->topic_name)].erase(sock_ID_Map[i]);
						}
					}
				}
			}
		}
	}

	close(sockfd);
	close(sockfdUDP);

	return 0;
}