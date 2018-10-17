# Rock-Paper-Scissors-Lizzard-Spock (RPSLS)

 > _Note:_ This is the Assignment 4 of CSC209 Software Tools and System Programming, taught in Summer 2018. No starter code was given. All code was written by me.

## Summary
This is an online multiplayer game using socket programming in C. Two players, represented by two processes of the compiled executable of [rpsls_client.c](rpsls_client.c), can connect to a server process, the executable of [rpsls_server.c](rpsls_server.c), by specifying their IP address and port numbers. Players can choose their move by typing specific characers in the command line, and the client process will wait for the server signal before both players have moved. If either of the players wishes to exit game, server will send game statistics to both players' windows.
