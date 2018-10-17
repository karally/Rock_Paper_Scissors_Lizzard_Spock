#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>     /* only needed on my mac */
#include <netdb.h>         /* gethostname */
#include "rpsls.h"


int create_and_connect(const char *hostname, int port) {
	int soc;
	if ((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("soc");
        exit(1);
	}
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memset(&(addr.sin_zero), 0, 8);
    struct hostent *hp = gethostbyname(hostname);
    if (hp == NULL) {
        fprintf(stderr, "unknown host %s\n", hostname);
        exit(1);
    }
    addr.sin_addr = *((struct in_addr *) hp->h_addr);
    if (connect(soc, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return -1;
    }
    return soc;
}

int rpsls_converter(char response) {
	switch(response) {
		case 'r': return ROCK;
		case 'p': return PAPER;
		case 's': return SCISSOR;
		case 'l': return LIZARD;
		case 'S': return SPOCK;
		case 'e': return END;
		default: return -1;
	}
}

int main(int argc, char const *argv[]) {
	int socket, port;
	int extra_port = 0;
	int read_name_length, server_response;
	int user_response_len, int_response, game_result;
	char name[11], user_response;
	const char *hostname;
	
	// Check argument number and validity
	if (argc == 2 || argc == 3) {
		hostname = argv[1];
		if (argc == 3) {
			char *extra = NULL;
			extra_port = strtol(argv[2], &extra, MAX_USERNAME);
			if (extra_port <= 0 || strcmp(extra, "") != 0 || strcmp(extra, argv[1]) == 0) {
				fprintf(stderr, "Usage:\n\t Port specifier\n");
				exit(1);
			}
		}
	} else {
		fprintf(stderr, "Usage:\n\tServer IP src\n");
		exit(1);
	}
	// Create and connect to a socket @ port
	// if fail, try the next socket
	port = PORT_NUM + extra_port;
	FILE *is_port = fopen("is_port", "rb");
    if (is_port != NULL) {
        port++;
        if (remove("is_port") < 0) {
    		perror("remove");
    	}
    } else {
    	is_port = fopen("is_port", "wb");
    }

	if ((socket = create_and_connect(hostname, port)) < 0) {
		exit(1);
	}
	// Wait for server to respond int 0 for game start
	// This read will be blocked until server sends something,
	// after 2 connections between server and 2 clients have been established
	printf("Waiting for server to start game... \n");
	if (read(socket, &server_response, sizeof(int)) < 0) {
		perror("Read server_response");
	}
	// Prompt for and write username
	printf("Please input your username (max 10 chars, extras will be truncated): ");
	read_name_length = scanf("%s", name);
	while (read_name_length == 0 || read_name_length > 10) {
		printf("Username invalid, please enter again: ");
		read_name_length = scanf(" %s", name);
	}
	name[10] = '\0';
	if (write(socket, &name, 11) == -1) {
        perror("write name");
        exit(1);
    }
    // Prompt for game action
    while (1) {
    	printf("Please enter your response: ");
    	user_response_len = scanf(" %c", &user_response);
    	if (user_response_len < 0 ||
    		(int_response = rpsls_converter(user_response)) < 0) {
    		printf("Bad data, please enter again! \n");
    		continue;
    	}
    	if (write(socket, &int_response, sizeof(int)) != sizeof(int)) {
    		perror("Write int_response");
    		exit(1);
    	}
    	if (read(socket, &game_result, sizeof(int)) != sizeof(int)) {
    		perror("Read game_result");
    		exit(1);
    	}
    	if (game_result == LOSE) {
    		fprintf(stderr, "lose\n");
    		printf("Oh no, you lost! :(\n");
    		continue;
    	} else if (game_result == WIN) {
    		fprintf(stderr, "win\n");
    		printf("Yay! You won!\n");
    		continue;
    	} else if (game_result == ENDGAME) {
    		printf("One or more players have ended the game. \n");
    		char msg[256];
    		if (read(socket, msg, 255) < 0) {
    			perror("Read msg");
    			exit(1);
    		}
    		msg[255] = '\0';
    		printf("%s\n", msg);
    		return 0;
    	} else if (game_result == DRAW) {
    		fprintf(stderr, "draw\n");
    		printf("It's a draw! \n");
    	} else {
    		fprintf(stderr, "Shouldn't get here\n");
    		exit(1);
    	}
    }
	return 1;
}