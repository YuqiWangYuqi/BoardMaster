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

    //establish buffer and KeyValue objects for reading RPCs
    char buffer[1024] = { 0 }, response[1024];
    RawKeyValueString *pRawKey;
    KeyValue rpcKeyValue;
    char *pRpcKey;
    char *pRpcValue;
    int status = 0;

    do {
        printf("Waiting for client\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
            perror("Accepting denied");
            exit(EXIT_FAILURE);
        }
        printf("Accepted\n");
        BoardMasterGame game;
        do {
            KeyValue userKeyValue;
            KeyValue passKeyValue;

            char *pszUserValue;
            char *pszPassValue;
            read(new_socket, buffer, 1024);
            pRawKey = new RawKeyValueString(buffer);
            pRawKey->getNextKeyValue(rpcKeyValue);
            pRpcKey = rpcKeyValue.getKey();
            pRpcValue = rpcKeyValue.getValue();

            if (strcmp(pRpcKey, "rpc") == 0) {
                if (strcmp(pRpcValue, "connect") == 0) {
                    // Get the next two arguments (user and password);

                    pRawKey->getNextKeyValue(userKeyValue);
                    pszUserValue = userKeyValue.getValue();

                    pRawKey->getNextKeyValue(passKeyValue);
                    pszPassValue = passKeyValue.getValue();

                    // Attempt checkLogin, retrieve validation result (0 / -1)
                    strcpy(response, checkLogin(response, pszUserValue, pszPassValue));

                    send(new_socket, response, strlen(response), 0);
                    printf("Response message sent\n");
                    if(strcmp(response, "-1") == 0){
                        status = false;
                    } else {
                        status = true;
                    }
                } else if (strcmp(pRpcValue, "disconnect") == 0) {
                    printf("Disconnect RPC received\n");
                    const char * disconnect = "You have been disconnected.\n";
                    send(new_socket, disconnect, strlen(disconnect), 0);
                    status = false;
                } else if(strcmp(pRpcValue, "start") == 0){
                    game.startResetGame();
                    const char * start= "New game started!\n";
                    send(new_socket, start, strlen(start), 0);
                } else if(strcmp(pRpcValue, "guess") == 0){
                    const char * state;

                    if(game.getMovesLeft() == 0){
                        state = "No moves remaining. Please start a new game.\n";
                        send(new_socket, state, strlen(state), 0);
                    }

                    KeyValue code;
                    pRawKey->getNextKeyValue(code);
                    char * guess =code.getValue();

                    if(!game.isValidGuess(guess, strlen(guess))){
                        state = "Invalid guess. All guesses must be 4 digits from 1 to 5.\n";
                        send(new_socket, state, strlen(state), 0);
                    } else {
                        //make a guess and determine results to send back
                        game.makeGuess(guess, strlen(guess));
                        if(game.isGameWon()){
                            state = "Perfect guess! You've won the game!\n";
                            send(new_socket, state, strlen(state), 0);
                        } else{
                            string guessStatus = "\nPerfect matches: ";
                            guessStatus += std::to_string(game.getPerfMatches());
                            guessStatus += "\nMatches out of position: ";
                            guessStatus += std::to_string(game.getCharMatches());
                            guessStatus += "\nGuesses remaining: ";
                            guessStatus += std::to_string(game.getMovesLeft());
                            guessStatus += "\n";
                            //check if this was the last move
                            if(game.isGameLost()){
                                guessStatus += "Sorry, you have lost the game.\n";
                            }
                            state = guessStatus.c_str();
                            send(new_socket, state, strlen(state), 0);
                        }
                    }


                } else if(strcmp(pRpcValue, "record") == 0){
                    const char * scores;
                    string totalScores = "Total wins: ";
                    totalScores += std::to_string(game.getTotalGamesWon());
                    totalScores += " Total losses: ";
                    totalScores += std::to_string(game.getTotalGamesLost());
                    totalScores += "\n";
                    scores = totalScores.c_str();
                    send(new_socket, scores, strlen(scores), 0);
                }
                else {
                    printf("Invalid RPC: %s\n", pRpcValue);
                    status = 0;
                }
            }
            delete pRawKey;
        } while (status == 1);
    } while (true);
}