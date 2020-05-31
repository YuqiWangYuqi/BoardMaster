/* Yuqi Wang, Nathaniel Wu, Dominic Rosato
 * CPSC5042 2020
 * Milestone 1
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include "assert.h"
#include "BoardMasterGame.h"

using namespace std;

#define PORT 12121

char * checkLogin(char *, char *, char *);

class KeyValue {
private:
    char m_szKey[128];
    char m_szValue[2048];

public:
    KeyValue() {};
    void setKeyValue(char *pszBuff) {
        char *pch1;
        // find out where the "=" sign is, and take everything to the left of the equal for the key
        // go one beyond the = sign, and take everything else
        pch1 = strchr(pszBuff, '=');
        assert(pch1);
        int keyLen = (int)(pch1 - pszBuff);
        strncpy(m_szKey, pszBuff, keyLen);
        m_szKey[keyLen] = 0;
        strcpy(m_szValue, pszBuff + keyLen + 1);
    }

    char *getKey() {
        return m_szKey;
    }

    char *getValue() {
        return m_szValue;
    }
};


// This class will take a string that is passed to it in this format:

// input to constructor:
// <variable1>=<value1>;<variable2>=<value2>;
//You will then call the method  getNextKeyValue until getNextKeyValue returns NULL.
// getNextKeyValue will return a KeyValue object. Inside of that KeyValue object will contain the variable and the value
// You will then call getKey or getValue to get the contents of those fields.
// The example in main() will show how to call this function.
// By extracting the contents you then can determine the rpc you need to switch to, along with variables you will need
// You can also use this class in your client program, since you will need to determine the contents that you receive from server

class RawKeyValueString {
private:
    char m_szRawString[32768];
    KeyValue *m_pKeyValue;
    char *m_pch;
public:
    RawKeyValueString(char *szUnformattedString) {
        assert(strlen(szUnformattedString));
        strcpy(m_szRawString, szUnformattedString);
        m_pch = m_szRawString;
    }
    ~RawKeyValueString() {
        if (m_pKeyValue)
            delete (m_pKeyValue);
    }

    void getNextKeyValue(KeyValue & keyVal) {
        // It will attempt to parse out part of the string all the way up to the ";", it will then create a new KeyValue object  with that partial string
        // If it can;t it will return null;
        char *pch1;
        char szTemp[32768];

        pch1 = strchr(m_pch, ';');
        assert(pch1 != NULL);
        int subStringSize = (int)(pch1 - m_pch);
        strncpy(szTemp, m_pch, subStringSize);
        szTemp[subStringSize] = 0;
        m_pch = pch1 + 1;
        if (m_pKeyValue)
            delete (m_pKeyValue);
        keyVal.setKeyValue(szTemp);
    }
};

/**
 * Initially sets up the server to be used in the remainder of the main program.
 * @param address
 * @param server_fd
 */
void setup(struct sockaddr_in & address, int & server_fd){

    int opt = 1;


    printf("initializing server...\n");
    //creating socket file descriptor
    printf("Getting Socket\n\n");
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    //Forcefully attaching socket to the personal port
    printf("Socket received\n");
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //Forcefully attaching socket to the personal port
    printf("binding/listening\n\n");
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0){
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
}

/**
 * Check login logic, input connect command, username
 * and password to determine user's login.
 * @param response connect command
 * @param username user input username
 * @param password user input password
 * @return 0 if login success, -1 if login failed
 */
char * checkLogin(char * response, char * username, char * password) {
    if ((strcmp(username, "Yuqi") == 0 ||
         strcmp(username, "Dominic") == 0 ||
         strcmp(username, "Nathaniel") == 0) &&
        strcmp(password, "12345") == 0) {
        cout << "Login Success" << endl;
        strcpy(response, "0");
    } else {
        cout << "Login failed" << endl;
        strcpy(response, "-1");
    }
    return response;
}
/**
 * Method for handling a connect RPC. Checks login and username using the checkLogin method. Also sends the message
 * to the client on status
 * @param new_socket the socket being connected to
 * @param buff the RawKeyValueString containing the commands
 * @return true if success, false if failure
 */
bool connectRPC(int new_socket, RawKeyValueString * buff){
    KeyValue rpcKeyValue;   //initialize KeyValue for rpc
    KeyValue userKeyValue;  //initialize KeyValue for username
    KeyValue passKeyValue;  //initialize KeyValue for password
    char response [1024];
    buff->getNextKeyValue(rpcKeyValue);

    //check to make sure the call is for an rpc=connect
    const char * rpc = rpcKeyValue.getKey();
    const char * value = rpcKeyValue.getValue();
    if((strcmp(rpc, "rpc") != 0)|| strcmp(value, "connect") != 0){
        printf("RPC sent is not a connect\n");
        return false;
    }

    buff->getNextKeyValue(userKeyValue);
    char *pszUserValue = userKeyValue.getValue();
    std::cout << pszUserValue << std::endl;

    buff->getNextKeyValue(passKeyValue);
    char *pszPassValue = passKeyValue.getValue();
    std::cout << pszPassValue << std::endl;

    // Attempt checkLogin, retrieve validation result (0 / -1)
    strcpy(response, checkLogin(response, pszUserValue, pszPassValue));

    send(new_socket, response, strlen(response), 0);    //sends result to server

    printf("Response message sent\n");
    if(strcmp(response, "-1") == 0){
        return false;
    } else {
        return true;
    }
}
/**
 * Method to disconnect a connection. Sends a message to client that they have been disconnected afterwards
 * @param new_socket the socket to return information to
 * @return false on completion of disconnect
 */
bool disconnectRPC(int new_socket){
    printf("Disconnect RPC received\n");
    return false;
}

/**
 * Method for starting or resetting a game. Sends a message to client that a new game has started. Will stop an existing
 * game in progress and reset it without logging a win or loss.
 * @param new_socket the socket to return information to
 * @param game the BoardMasterGame that handles the game logic
 * @return true on successful restart
 */
bool startRPC(int new_socket, BoardMasterGame & game){
    game.startResetGame();
    const char * start= "New game started!\n";
    send(new_socket, start, strlen(start), 0);
    return true;
}

/**
 * Method to make a guess on an existing BoardMasterGame object. Sends a response back to the client on state
 * of the guess being made
 * @param new_socket the socket being communicated with
 * @param buff the RawKeyValueString containing the guess
 * @param game the BoardMasterGame object containing the state of the
 * @return true on successful guess, false if error occurs
 */
bool guessRPC(int new_socket, RawKeyValueString * buff, BoardMasterGame & game) {
    const char *state;

    if (game.getMovesLeft() == 0) {
        state = "No moves remaining. Please start a new game.\n";
        send(new_socket, state, strlen(state), 0);
    }

    KeyValue code;
    buff->getNextKeyValue(code);
    char *guess = code.getValue();

    if (!game.isValidGuess(guess, strlen(guess))) {
        state = "Invalid guess. All guesses must be 4 digits from 1 to 5.\n";
        send(new_socket, state, strlen(state), 0);
    } else {
        //make a guess and determine results to send back
        game.makeGuess(guess, strlen(guess));
        if (game.isGameWon()) {
            state = "Perfect guess! You've won the game!\n";
            send(new_socket, state, strlen(state), 0);
        } else {
            string guessStatus = "\nPerfect matches: ";
            guessStatus += std::to_string(game.getPerfMatches());
            guessStatus += "\nMatches out of position: ";
            guessStatus += std::to_string(game.getCharMatches());
            guessStatus += "\nGuesses remaining: ";
            guessStatus += std::to_string(game.getMovesLeft());
            guessStatus += "\n";
            //check if this was the last move
            if (game.isGameLost()) {
                guessStatus += "Sorry, you have lost the game.\n";
            }
            state = guessStatus.c_str();
            send(new_socket, state, strlen(state), 0);
        }
    }
    return true;
}

/**
 * Method that returns the current log of wins and losses on the current session.
 * @param new_socket the socket to send the information to
 * @param game the BoardMasterGame object keeping track of wins and losses
 * @return true on successful send
 */
bool recordRPC(int new_socket, BoardMasterGame & game){
    const char * scores;
    string totalScores = "Total wins: ";
    totalScores += std::to_string(game.getTotalGamesWon());
    totalScores += " Total losses: ";
    totalScores += std::to_string(game.getTotalGamesLost());
    totalScores += "\n";
    scores = totalScores.c_str();
    send(new_socket, scores, strlen(scores), 0);
    return true;
}

/**
 * Method for determining which RPC function to use. Passes relevant objects needed to the method for use and returns
 * their results
 * @param new_socket the socket to have information returned to
 * @param buff the RawKeyValueString containing the RPC information
 * @param game the BoardMasterGame containing the game-state and logic
 * @return true on successful handling of RPC that doesn't involve disconnect, false for an incorrect RPC or disconnect
 * RPC sent (should signal to main to close the socket)
 */
bool handleRPC(int new_socket, RawKeyValueString * buff, BoardMasterGame & game){
    KeyValue rpcKeyValue;
    buff->getNextKeyValue(rpcKeyValue);
    const char * key = rpcKeyValue.getKey();
    const char * value = rpcKeyValue.getValue();

    if(strcmp(key, "rpc") != 0){
        printf("RPC not issued\n");
        return false;
    }

    if(strcmp(value, "disconnect") == 0){
        return disconnectRPC(new_socket);
    } else if(strcmp(value, "start") == 0){
        return startRPC(new_socket, game);
    } else if(strcmp(value, "guess") == 0){
        return guessRPC(new_socket, buff ,game);
    } else if(strcmp(value, "record") == 0){
        return recordRPC(new_socket, game);
    } else {
        printf("Invalid RPC issued\n");
        return false;
    }
}

/**
 * Connect and turn on the server, listen from client for
 * RPCs. An endless loop to not kill the server side for more
 * commands from client.
 */
int main(int argc, char const *argv[]) {
    //initial setup of server
    struct sockaddr_in address;
    int server_fd, new_socket;
    setup(address, server_fd);
    int addrlen = sizeof(address);

    //establish buffer and status for handling when to disconnect
    char buffer[1024] = { 0 };
    int status;

    while(true){
        printf("Waiting for client\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
            perror("Accepting denied");
            exit(EXIT_FAILURE);
        }
        printf("Accepted\n");

        //generate BoardMasterGame object, read buffer and use for connect RPC (must come first!)
        BoardMasterGame game;
        read(new_socket, buffer, 1024);
        RawKeyValueString * buff = new RawKeyValueString(buffer);

        status = connectRPC(new_socket, buff);  //true if successful connection, false if invalid info

        delete buff;

        while(status == true){
            read(new_socket, buffer, 1024);
            RawKeyValueString * buff = new RawKeyValueString(buffer);
            status = handleRPC(new_socket, buff, game);
            delete buff;
        }
        const char * disconnectMessage = "You have been disconnected.";
        send(new_socket, disconnectMessage, strlen(disconnectMessage), 0);
        close(new_socket);
    }
}