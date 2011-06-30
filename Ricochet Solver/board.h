//
//  board.h
//  Ricochet Solver
//
//  Created by Stephen Sisk on 4/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <assert.h>
#include <time.h>
#include "boost/functional/hash.hpp"

#pragma once

using namespace std;

// How many robots are on the board? we define the size of several arrays by the NUM_ROBOTS so it needs to be a constant.
#define NUM_ROBOTS 5

#define INVALID_LOC -1
#define PRINTBOARD_NOCOLOR 9


typedef char BoardLocType;
const BoardLocType EMPTY = 0;
const BoardLocType TOP_BLOCKED = 1;
const BoardLocType RIGHT_BLOCKED = 2; 
const BoardLocType BOTTOM_BLOCKED = 4;
const BoardLocType LEFT_BLOCKED = 8;

struct Location
{
    BoardLocType x;
    BoardLocType y;
    Location(BoardLocType x, BoardLocType y)
    {
        this->x = x;
        this->y = y;
    }
    Location()
    {
        this->x = 0;
        this->y = 0;
    }
    
    bool operator==(const Location &other) const {
        return ((x == other.x) && (y == other.y));
    }
    
    bool operator!=(const Location &other) const {
        return !(*this == other);
    }
    
    Location& operator=(const Location &rhs) {
        
        // Only do assignment if RHS is a different object from this.
        if (this != &rhs) {
            this->x = rhs.x;
            this->y = rhs.y;
        }
        
        return *this;
    }
    
    static Location Invalid;
};


// a struct that stores the current location of the 5 robots on the board, plus allows comparison, etc...
struct BoardState
{
    Location robots[NUM_ROBOTS];
    
    bool operator==(const BoardState &other) const 
    {
        bool ret = true;
        for(int i = 0; (i < NUM_ROBOTS) && ret; i++)
        {
            ret = ret && (robots[i] == other.robots[i]);
        }
        
        return ret;
    }
    
    BoardState& operator=(const BoardState &rhs) {
        
        // Only do assignment if RHS is a different object from this.
        if (this != &rhs) 
        {
            for (int i = 0; i < NUM_ROBOTS; i++) 
            {
                this->robots[i]  = rhs.robots[i];
            }
        }
        
        return *this;
    }
};

//This allows the boost::hash functionality to work with BoardState
std::size_t hash_value(BoardState const& b);

// used for adding text to printouts of the board
struct BoardOverlay
{
    Location loc; // where to put the text
    char text[5]; // what you want printed
    int robotColor; // When printing, which color do you want the text printed in? This is the index of the robot to which this color corresponds. PRINTBOARD_NOCOLOR for black
};

// responsible for holding the layout of the static elements of the board like walls/etc...
class Board
{
public: 
    Board(short x, short y, Location target, BoardLocType brd[] = 0);
    ~Board();
    vector<BoardState> NextStates(BoardState state);
    Location GetTarget()
    {
        return _target;
    }
    void WhereCanThisPieceMove(BoardState state, int curRobot, vector<Location> &ret);
    Location GetSize()
    {
        return size;
    }

    // output the contents of the board
    void PrintMoves(BoardState &state,vector<Location> &moves, ostream &out,int color);
    void print(BoardState state, ostream &out, vector<BoardOverlay> *overlays = NULL);
    
    //unit tests
    static void TestDetermineEdges();
    static void TestWhereCanThisPieceMove(ostream &out);
    static void TestPrintBoard(ostream &out);
    static void TestGet(ostream &out);
    
private: 
    
    void DetermineEdges(int curRobot, BoardState state, BoardLocType x, BoardLocType y, Location &lowerEdge, Location &upperEdge);
    
    Location size;
    Location _target;
    BoardLocType *board;
    // size we stored our 2 dimensional board as a one-dimensional array, this function does the translation
    BoardLocType get(short x, short y)
    {
        assert(x < size.x);
        assert(y < size.y);
        return board[(size.x * y) + x];
    }
};

