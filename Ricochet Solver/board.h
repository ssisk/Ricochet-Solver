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
//#include <boost/intrusive/unordered_set.hpp>

#include "boost/functional/hash.hpp"

#pragma once

using namespace std;

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

// we keep the board in a separate struct so that it doesn't get passed around 
// we could have just created a pointer to the array inside of BoardState, but the syntax on that is super ugly.



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
    
    //unordered_multiset_member_hook<> member_hook;
};

//This allows the boost::hash functionality to work with BoardState

std::size_t hash_value(BoardState const& b);

struct BoardOverlay
{
    Location loc; // where to put the text
    char text[5];
    int robotColor; // the robot to which this color corresponds. PRINTBOARD_NOCOLOR for black
};


class Board
{
public: 
    void print(BoardState state, ostream &out, vector<BoardOverlay> *overlays = NULL);
  //  Board(short x, short y);
    Board(short x, short y, Location target, BoardLocType brd[] = 0);
    ~Board();
    vector<BoardState> NextStates(BoardState state);
    Location GetTarget()
    {
        return _target;
    }

    static void TestDetermineEdges();
    static void TestWhereCanThisPieceMove(ostream &out);
    static void TestPrintBoard(ostream &out);
    static void TestGet(ostream &out);
    void WhereCanThisPieceMove(BoardState state, int curRobot, vector<Location> &ret);

    Location GetSize()
    {
        return size;
    }
    
    void PrintMoves(BoardState &state,vector<Location> &moves, ostream &out,int color);
private: 
    
    void DetermineEdges(int curRobot, BoardState state, BoardLocType x, BoardLocType y, Location &lowerEdge, Location &upperEdge);
    
    Location size;
    Location _target;
    BoardLocType *board;
    BoardLocType get(short x, short y)
    {
        assert(x < size.x);
        assert(y < size.y);
        return board[(size.x * y) + x];
    }
};

