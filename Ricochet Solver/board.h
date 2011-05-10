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

#pragma once

using namespace std;

#define NUM_ROBOTS 3
#define INVALID_LOC -1



typedef short BoardLocType;
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
};

// we keep the board in a separate struct so that it doesn't get passed around 
// we could have just created a pointer to the array inside of BoardState, but the syntax on that is super ugly.



struct BoardState
{
    Location robots[NUM_ROBOTS];
    Location target;
};


class Board
{
public: 
    void print(BoardState state, ostream &out, vector<Location> *moves = NULL);
    Board(short x, short y);
    ~Board();
    vector<BoardState> NextStates(BoardState state);

    static void TestDetermineEdges();
    static void TestWhereCanThisPieceMove(ostream &out);
    static void TestPrintBoard(ostream &out);
    static void TestGet(ostream &out);
    
    
private: 
    
    void DetermineEdges(int curRobot, BoardState state, BoardLocType x, BoardLocType y, Location &lowerEdge, Location &upperEdge);

    
    
    void WhereCanThisPieceMove(BoardState state, int curRobot, vector<Location> &ret);
    
    Location size;
    BoardLocType *board;
    BoardLocType get(short x, short y)
    {
        assert(x < size.x);
        assert(y < size.y);
        return board[(size.x * y) + x];
    }
};