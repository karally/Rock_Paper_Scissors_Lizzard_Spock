#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>     /* only needed on my mac */
#include "rpsls.h"

#define PORT_NUM1 60001

struct user {
    int fd;
    char username[MAX_USERNAME];
};

int games_played;
int p0_won;
int p1_won;

// Create a socket, initialize a sockaddr_in server with port port_number
// Return the socket.
int create_and_bind_socket(int port_number) {
    int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc < 0) {
        perror("server soc");
        exit(1);
    }
    int on = 1;
    if (setsockopt(soc, SOL_SOCKET, SO_REUSEADDR,
           (const char *) &on, sizeof(on)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port_number);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&(server.sin_zero), 0, 8);
    if (bind(soc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("server: bind");
        close(soc);
        exit(1);
    }
    return soc;
}

int listen_and_accept(int socket) {
    if (listen(socket, MAX_BACKLOG) == -1 ) { 
        perror("listen"); 
        exit(1); 
    }
    int client_socket = accept(socket, NULL, NULL);
    if (client_socket < 0) {
        perror("listen_and_accept");
        return -1;
    }
    return client_socket;
}

// Return the player number that wins this match
// and update the scores.
int rpsls(int p0, int p1) {
	games_played++;
    if (p0 == p1) {
        return 2;
    }
	switch(p0) {
		case ROCK:
			if (p1 == SCISSOR || p1 == LIZARD) {
                p0_won++;
				return 0;
			} else {
                p1_won++;
				return 1;
			}
		case PAPER:
			if (p1 == ROCK || p1 == SPOCK) {
                p0_won++;
				return 0;
			} else {
                p1_won++;
				return 1;
			}
		case SCISSOR:
			if (p1 == PAPER || p1 == LIZARD) {
                p0_won++;
				return 0;
			} else {
                p1_won++;
				return 1;
			}
		case LIZARD:
			if (p1 == PAPER || p1 == SPOCK) {
                p0_won++;
				return 0;
			} else {
                p1_won++;
				return 1;
			}
		case SPOCK:
			if (p1 == ROCK || p1 == SCISSOR) {
                p0_won++;
				return 0;
			} else {
                p1_won++;
				return 1;
			}
		default: return -1;
	}
}

int main(int argc, char const *argv[])
{
    int port0, port1, extra_port = 0, user_num = 0;
    int sockets[2];
    int read_num0, read_num1, dummy;
    int win = WIN, lose = LOSE, end = ENDGAME, draw = DRAW;
    char *extra = NULL;
    struct user users[2];


    // Getting port number
	if (argc == 2) {
        extra_port = strtol(argv[1], &extra, MAX_USERNAME);
        if (extra_port < 0 || strcmp(extra, "") != 0 || strcmp(extra, argv[1]) == 0) {
            fprintf(stderr, "Usage:\n\t Port specifier\n");
            exit(1);
        }
	} else if (argc > 2) {
        fprintf(stderr, "Usage:\n\t Command line argument number\n");
        exit(1);
    }
    port0 = PORT_NUM + extra_port;
    port1 = PORT_NUM1 + extra_port;

    // Array to store 2 users.
    for (int i = 0; i < 2; i++) {
        users[i].fd = -1;
    }
    // Create and bind 2 sockets.
	sockets[0] = create_and_bind_socket(port0);
    sockets[1] = create_and_bind_socket(port1);

    // Listen for 2 connections.
    while (user_num < 2) {
        int current_user_socket = listen_and_accept(sockets[user_num]);
        users[user_num].fd = current_user_socket;                    
        user_num++;
    }

    // Write initial game start indicator.
    int zero = 0;
    if (write(users[0].fd, &zero, sizeof(int)) < 0) {
        perror("write user0 indicator");
    }
    if (write(users[1].fd, &zero, sizeof(int)) < 0) {
        perror("write user1 indicator");
    }

    // Reading usernames
    if ((read_num0 = read(users[0].fd, &(users[0].username), MAX_USERNAME)) < 0) {
        perror("read");
        exit(1);
    }
    if ((read_num1 = read(users[1].fd, &(users[1].username), MAX_USERNAME)) < 0) {
        perror("read");
        exit(1);
    }
    
    if (read_num0 < 0 || read_num1 < 0) {
        exit(1);
    }

    // reading some garbage first: 
    read(users[0].fd, &dummy, sizeof(int));
    read(users[1].fd, &dummy, sizeof(int));

    // Reading and determining win/lose
    while (1) {
    	int user0_response, user1_response;
        if ((read_num0 = read(users[0].fd, &user0_response, sizeof(int))) < 0) {
            perror("read");
            exit(1);
        }
        if ((read_num1 = read(users[1].fd, &user1_response, sizeof(int))) < 0) {
            perror("read");
            exit(1);
        }
        if (read_num0 < 0 || read_num1 < 0) {
        	exit(1);
        } else if (read_num0 == 0 || read_num1 == 0) {
            fprintf(stderr, "Client disconnected\n");
            exit(1);
        }
        // The game is ended
        if (user0_response == 0 || user1_response == 0) {
        	int write_end_0 = write(users[0].fd, &end, sizeof(int));
        	int write_end_1 = write(users[1].fd, &end, sizeof(int));
        	if (write_end_0 < 0) {
        		perror("write end 0");
        		exit(1);
        	}
        	if (write_end_1 < 0) {
        		perror("write end 1");
        		exit(1);
        	}
        	if (write_end_0 < sizeof(int) || write_end_1 < sizeof(int)) {
        		fprintf(stderr, "Client disconnected\n");
            	exit(1);
        	}
        	char msg[256];
        	snprintf(msg, sizeof(msg),
                "Total number of games played: %d\n\tPlayer %s score: %d\n\tPlayer %s score: %d\n",
        		games_played, users[0].username, p0_won, users[1].username, p1_won);
        	write_end_0 = write(users[0].fd, &msg, 255);
        	write_end_1 = write(users[1].fd, &msg, 255);
        	if (write_end_0 < 0) {
        		perror("write msg 0");
        		exit(1);
        	}
        	if (write_end_1 < 0) {
        		perror("write msg 1");
        		exit(1);
        	}
        	if (write_end_0 < 255 || write_end_1 < 255) {
        		fprintf(stderr, "Client disconnected\n");
            	exit(1);
        	}
        	return 0;
        }
        int winner = rpsls(user0_response, user1_response);
        if (winner == DRAW) {
            if (write(users[0].fd, &draw, sizeof(int)) < 0) {
                perror("write draw 0");
            }
            if (write(users[1].fd, &draw, sizeof(int)) < 0) {
                perror("write draw 1");
            }
            continue;
        }
        if (write(users[winner].fd, &win, sizeof(int)) < 0) {
        	perror("write winner");
        }
        if (write(users[1 - winner].fd, &lose, sizeof(int)) < 0) {
        	perror("write lose");
        }
    }

	return 0;
}