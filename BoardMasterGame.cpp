//
// Created by Dom on 5/19/2020.
//

#include "BoardMasterGame.h"

/**
 * Method for determining whether a guess provided is valid. A method
 * is valid if it contains 4 characters all consisting of numbers from 1 to 5
 * @param guess: the character sequence representing the guess
 * @param size: the size of the guessed sequence
 */
bool BoardMasterGame::isValidGuess(char *guess, int size) {
    if(size != CODELENGTH){
        return false;
    } else {
        for(int i = 0; i < size; i++){
            bool isValidChar = false;
            for(int j = 0; j < VALIDCHARSETSIZE; j++){
                if(guess[i] == VALIDGUESSCHARS[j]){
                    //current character is in valid set
                    isValidChar = true;
                    break;
                }
            }
            //couldn't find current character in valid set
            if(!isValidChar){
                return false;
            }
        }
        return true;
    }
}

/**
 * Method for inputting a guess into the BoardMasterGame. This method
 * does NOT return the results of the guess, but updates the number of guesses remaining and
 * determines if the game has been lost or won, updating state accordingly. Additionally, the number
 * of perfect matches and number of character matches are also determined and updated.
 * If the guess is not valid, the guess will not be recorded. If the game has not started, or
 * the game is currently won or lost, the guess will not be recorded.
 * @param guess: the character sequence representing the guess
 */
void BoardMasterGame::makeGuess(char *guess, int size) {
    if(!isValidGuess(guess, size)){
        return; //invalid guess, no change
    }
    if(!gameInProgress || gameWon || gameLost){
        return; //game not started, or currently finished with win or loss, no change
    }
    numPerfMatches = 0;
    numCharMatches = 0;
    char currGuess[CODELENGTH];
    bool beenCountedInCode[CODELENGTH] = {false, false, false, false};
    bool beenCountedInGuess[CODELENGTH] ={false, false, false, false};
    strcpy(currGuess, guess);   //copy guess so values can be changed without creating errors

    //sweep for perfect matches
    for(int i = 0; i < CODELENGTH; i++){
        if(currGuess[i] == currCode[i]){
            numPerfMatches++;
            beenCountedInCode[i] = true;
            beenCountedInGuess[i] = true;
        }
    }

    //sweep for in-code, but out of position matches
    for(int i = 0; i < CODELENGTH; i++){
        for(int j = 0; j < CODELENGTH; j++){
            if((currGuess[i] == currCode[j]) && !beenCountedInCode[j] && !beenCountedInGuess[i]){
                beenCountedInCode[j] = true;
                beenCountedInGuess[i] = true;
                numCharMatches++;
                break;  //found matching character, so move on
            }
        }
    }

    //reduce moves left, check if game is won or lost
    movesRemaining--;
    if(numPerfMatches == CODELENGTH){
        gameWon = true; //game is won!
        gameInProgress = false;
        totalGamesWon++;
    } else if(movesRemaining == 0){
        gameLost = true;    //no moves left, game was not won, so game is lost!
        gameInProgress = false;
        totalGamesLost++;
    }
}

/**
 * Method for starting the game if it has not been started yet or resetting the game if a
 * game is currently in progress. A new code will be generated for the game.
 */
void BoardMasterGame::startResetGame() {
    movesRemaining = 8;
    gameInProgress = true;
    srand(time(NULL));
    for(int i = 0; i < CODELENGTH; i++){
        currCode[i] = VALIDGUESSCHARS[rand() % VALIDCHARSETSIZE];
    }

}

/**
 * @return whether the current game has been lost
 */
bool BoardMasterGame::isGameLost() {
    return gameLost;
}

/**
 * @return whether the current game has been won
 */
bool BoardMasterGame::isGameWon() {
    return gameWon;
}

/**
 * @return the total number of games lost
 */
bool BoardMasterGame::getTotalGamesLost() {
    return totalGamesLost;
}

/**
 * @return the total number of games won
 */
bool BoardMasterGame::getTotalGamesWon() {
    return totalGamesWon;
}

/**
 * @return the number of character matches out of position
 */
int BoardMasterGame::getCharMatches() {
    return numCharMatches;
}

/**
 * @return the number of perfect character matches
 */
int BoardMasterGame::getPerfMatches() {
    return numPerfMatches;
}

/**
 * @return the number of moves left in the current game
 */
int BoardMasterGame::getMovesLeft() {
    return movesRemaining;
}
