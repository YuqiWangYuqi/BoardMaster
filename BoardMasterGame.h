//
// Created by Dom on 5/19/2020.
//

#ifndef BOARDMASTER_BOARDMASTERGAME_H
#define BOARDMASTER_BOARDMASTERGAME_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


class BoardMasterGame {
private:
    static int const CODELENGTH = 4;
    static int const VALIDCHARSETSIZE = 5;
    char currCode[CODELENGTH] = {'0', '0', '0', '0'};
    char const VALIDGUESSCHARS[VALIDCHARSETSIZE] = {'1' , '2' ,'3', '4', '5'};
    bool gameInProgress = false;
    bool gameWon = false;
    bool gameLost = false;
    int numPerfMatches = 0;
    int numCharMatches = 0;
    int movesRemaining = 0;
    int totalGamesWon = 0;
    int totalGamesLost = 0;

public:
    bool isValidGuess(char guess[], int size);
    void makeGuess(char guess[], int size);
    void startResetGame();
    int getPerfMatches();
    int getCharMatches();
    int getMovesLeft();
    bool isGameWon();
    bool isGameLost();
    bool getTotalGamesWon();
    bool getTotalGamesLost();

};


#endif //MILESTONE1_BOARDMASTERGAME_H