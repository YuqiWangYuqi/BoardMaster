// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <thread>

#define PORT 12121

/**
 * Method used for the initial server connection. Is called in the first connect RPC
 * @param sock the socket to be connected to
 * @return 0 on success, -1 on failure
 */
int connectToServer(int & sock)
{
    char const* host = "127.0.0.1";
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    // Interpret server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    return 0;
}

/**
 * Method used for connection to the server. Will always be called on run of the program, but not afterwards
 * @param sock the socket being connected to
 * @return 0 on completion of the program successfully logging in, -1 if connection failed
 */
int connectRPC(int & sock)
{
    // Input Arguments are:
    // username
    // password
    // input format="rpc=connect;username=<Your user>;password=<Your password>;"
    // Output arguments are:
    // status     (This will be set to 1 if success and -1 if error)
    // error      ( This will be to blank or an error message)
    // output format="status=<error status>;error=<error or blank>

    int connected = connectToServer(sock);
    if(connected == -1){
        return -1;  //early exit on connection failure
    }

    int valRead = 0;
    // [0] = username, [1] = password, [2] = concatenated input
    char login[3][100];

    // Assembles the RPC with login information
    std::cout << "Please enter your username:\n";
    std::cin >> login[0];
    std::cout << "Please enter your password:\n";
    std::cin >> login[1];
    strcpy(login[2], "rpc=connect;username=");
    strcat(login[2], login[0]);
    strcat(login[2], ";password=");
    strcat(login[2], login[1]);
    strcat(login[2], ";");

    // Sends the RPC to the server
    char buffer[1024] = {0};
    send(sock, login[2], strlen(login[2]), 0);
    std::cout << "Login information sent\n";

    // Interpret the server response. 0 = no error, -1 = invalid login
    // Anything else = invalid server response
    valRead = read(sock, buffer, 1024);
    std::string error;
    if (strcmp(buffer, "0") == 0) {
        error = "None";
    }
    else if (strcmp(buffer, "-1") == 0) {
        error = "Invalid login";
    }
    else {
        error = "Invalid server response";
    }

    // Print out the server response and relevant notification to the user
    std::cout << "status = " << buffer << "; error = " << error << "; valRead = "  << valRead << std::endl;
    if(strcmp(buffer, "0") != 0){
        std::cout << "Disconnecting from server. Please try again." << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Welcome to BoardMaster, " << login[0] << ". what can we do for you?\n";
    return 0;
}

/**
 * Method used to disconnect from the server. Doing so will close this program and indicate to the server that the
 * socket being used may also be closed.
 * @param sock the socket to close
 * @return 0 on success
 */
int disconnectRPC(int & sock)
{
    // input format="rpc=disconnect;"
    // output format="status=<error status>; error=<error or blank>;
    char disconnect[24] = "rpc=disconnect;";
    char buffer[1024] = {0};
    send(sock, disconnect, strlen(disconnect), 0);
    std::cout << "Disconnect sent\n";
    read(sock, buffer, 1024);
    printf(buffer);
    return 0;
}

/**
 * Method used to start a BoardMaster game. Does not require any user input once this has started
 * @param sock the socket to send the game start command to
 * @return 0 on success
 */
int startGameRPC(int& sock)
{
    //input format="rpc=start;'
    //output format="New game started" or error
    char start[24] = "rpc=start;";
    char buffer[1024] = {0};
    send(sock, start, strlen(start) , 0);
    std::cout << "Start game sent\n";
    read(sock, buffer, 1024);
    printf(buffer);
    return 0;
}

/**
 * Method used to make a guess to the server on a running BoardMaster game. Will ask the user for input on their guess.
 * The guess will be sent and results will be printed back.
 * @param sock the socket to send the information to
 * @return 0 on success
 */
int guessRPC(int& sock)
{
    //input format="rpc=guess;code =<input guess>"
    //output format="Perfect matches:<number> Matches out of position: <number> Guesses remaining: <number>
    std::string guess = "rpc=guess;code=";
    std::string code;
    std::cout << "Please enter your guess: ";
    std::cin >> code;

    // if the guess is quit (case insensitive), exit the game
    for(int i = 0; i < (int) code.length(); i++){
        code[i] = std::toupper(code[i]);
    }
    if (code == "QUIT") {
        std::cout << "You have exited the game.\n";
        return 1;
    }

    // assemble the guess RPC and send it to the server 
    guess += (code + ";");
    char buffer[1024] = {0};
    send(sock, guess.c_str(), strlen(guess.c_str()) , 0);
    std::cout << "Guess sent\n";
    read(sock, buffer, 1024);
    printf(buffer);

    // return 1 if solved, else return 0 (determines if the user gets more guesses)
    if (strcmp(buffer, "Perfect guess! You've won the game!\n") == 0)
    {
        return 1;
    }
    return 0;
}

/**
 * Method used to get a record of wins and losses from the server. Will send the command and print the results.
 * @param sock the socket to send the request to
 * @return 0 on success
 */
int recordRPC(int& sock)
{
    //input format="rpc=record"
    //output format="Total wins:<number> Total losses: <number>"
    char buffer[1024] = {0};
    const char * record = "rpc=record;";
    send(sock, record, strlen(record), 0);
    std::cout << "Record request sent\n";
    read(sock, buffer, 1024);
    printf(buffer);
    return 0;
}

/**
 * Method for handling which RPCs to send to the server. Will ask for user input and proceed accordingly. If a user
 * types 'help' instead, a printout of valid commands will be displayed
 * @param sock the socket that will be passed to the necessary RPC to work with
 * @return true if a connection should stay, false if a connection is terminated (happens only on disconnect)
 */
bool RPCSelector(int sock)
{
    std::cout << "Please enter the desired RPC. Type 'HELP' to show options." << std::endl;
    std::string input;
    std::cin >> input;

    //sets input to uppercase regardless of user input
    for(int i = 0; i < (int) input.length(); i++){
        input[i] = std::toupper(input[i]);
    }

    // Displays the available commands when 'HELP' is typed
    if (input == "HELP") {
        std::cout << "RPCs (case insensitive):" << std::endl;
        std::cout << "'START': Start a new game. You can type 'QUIT' to exit during a game." << std::endl;
        std::cout << "'RULES': Display the rules for the game." << std::endl;
        std::cout << "'RECORDS': Displays total wins/losses on the server." << std::endl;
        std::cout << "'DISCONNECT': Disconnects from the server." << std::endl;
        return true;
    }
    // Displays the rules for the game when 'RULES' is typed
    else if (input == "RULES") 
    {
        std::cout << "The server will randomly generate a 4 digit code using values between 1-5." << std::endl;
        std::cout << "You have 8 guesses to correctly guess the code. After each guess, the" << std::endl;
        std::cout << "server will tell you how many perfect matches (correct number in correct slot)" << std::endl;
        std::cout << "and matches out of position (correct number in incorrect slot) there are." << std::endl;
        std::cout << "Good luck!" << std::endl;
        return true;
    }
    // Disconnects the client from the server and closes the socket when 'DISCONNECT' is typed
    else if (input == "DISCONNECT")
    {
        disconnectRPC(sock);
        close(sock);
        return false;
    }
    // Displays the total wins/losses on the server when 'RECORDS' is typed
    else if (input == "RECORDS")
    {
        recordRPC(sock);
        return true;
    }
    // Starts a new game on the server when 'START' is typed
    else if (input == "START")
    {
        startGameRPC(sock);
        for(int i = 0; i < 8; i++)
        {
            if (guessRPC(sock) == 1) 
            {
                break;
            }
        }
        return true;
    }
    // The input was not recognized
    else
    {
        std::cout << "Invalid input. Please enter a RPC or 'HELP'." << std::endl;
        return true;
    }
}

/**
 * Main method that runs the client-side program. Will continuously make calls to the RPCSelector function
 * until a disconnect request is sent.
 * @return 0 on completion of the program
 */
int main()
{
    int sock = 0;
    // boolean for whether to continue asking for input
    bool cont = true;

    int connected = connectRPC(sock);
    if(connected == -1){
        cont = false;   //failed connection (likely server down), so terminate now
    }

    // Keep taking user input until disconnect is called
    while (cont)
    {
        cont = RPCSelector(sock);
    }
    return 0;
}
