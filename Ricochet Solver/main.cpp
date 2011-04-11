//
//  main.cpp
//  Ricochet Solver
//
//  Created by Stephen Sisk on 4/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <assert.h>

using namespace std;

#define BOARD_X 5
#define BOARD_Y 5
#define NUM_ROBOTS 3

char ROBOT_COLORS[NUM_ROBOTS][10] = {"red", "green", "orange"};

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
};

struct PieceLocations
{
    Location robots[NUM_ROBOTS];
    Location target;
};

BoardLocType board[BOARD_X][BOARD_Y] = 
{
    { LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED },
    { LEFT_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED},
    { LEFT_BLOCKED, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED},
    { LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED},
    { BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED}
};


void printBoard(BoardLocType board[][BOARD_Y], PieceLocations pieces)
{
    
    ofstream fout("./rico_out.html", ios::out);
    fout << "<html><body>hi<table style=\"border: 3px solid #000\" width=400 height=400>\r\n";
    
    char topChunk[100] = "border-top: 1px solid #000;";
    char leftChunk[100] = "border-left: 1px solid #000;";
    char bottomChunk[100] = "border-bottom: 1px solid #000;";
    char rightChunk[100] = "border-right: 1px solid #000;";

    
    for(int i = 0; i < BOARD_X; i++)
    {
        fout << "<tr>\r\n";
        for(int j = 0; j < BOARD_Y; j++)
        {
            fout << "<td width=80 height=80 style=\"border: 1px solid lightgrey;";
            switch(board[i][j])
            {
                
                case TOP_BLOCKED:
                    fout << topChunk;
                    break;
                case TOP_BLOCKED | RIGHT_BLOCKED:
                    fout << topChunk << rightChunk;
                    break;
                case RIGHT_BLOCKED:
                    fout << rightChunk;
                    break;
                case RIGHT_BLOCKED | BOTTOM_BLOCKED:
                    fout << rightChunk << bottomChunk;
                    break;
                case BOTTOM_BLOCKED:
                    fout << bottomChunk;
                    break;
                case BOTTOM_BLOCKED | LEFT_BLOCKED:
                    fout << bottomChunk << leftChunk;
                    break;
                case LEFT_BLOCKED:
                    fout << leftChunk;
                    break;
                case LEFT_BLOCKED | TOP_BLOCKED:
                    fout << leftChunk << topChunk;
                    break;
                case EMPTY:
                    //do nothing!
                    break;
                    
            }
            
            fout << "\">";
            for(int r = 0; r < NUM_ROBOTS; r++)
            {
                if(pieces.robots[r].x == i && pieces.robots[r].y == j)
                {
                    fout << "<font size=24 color=" << ROBOT_COLORS[r] << ">x</font>";
                }
            }
            
            if(pieces.target.x == i && pieces.target.y == j)
            {
                fout << "<font size=24 color=red>O</font>";
            }
            
            fout << "</td>";
            
        }
        fout << "</tr>\r\n";
    }
    fout << "</table>\r\n";
    
    return;
}

int main (int argc, const char * argv[])
{

    // insert code here...

    PieceLocations pieces;
    
    pieces.robots[0].x = 0;
    pieces.robots[0].y = 0;
    pieces.robots[1].x = 3;
    pieces.robots[1].y = 3;
    pieces.robots[2].x = 2;
    pieces.robots[2].y = 1;
    pieces.target.x = 4;
    pieces.target.y = 3;

    
    
    printBoard(board, pieces);
    
    return 0;
}


/* This function is a helper function for WhereCanThisPieceMove

determine if the pieces define any edges for us. since the pieces aren't on the board, rather than check every piece's x/y coordinates on every single pass of the "can it move here" phase, we will instead just figure out if any pieces lie on the same x or y lines, and make sure that we don't go past those pieces when we're checking the board. 

curRobot - the robot for which we are determining the edges

 
 */
void DetermineEdges(int curRobot, PieceLocations &curPieces, BoardLocType board[][BOARD_Y], BoardLocType x, BoardLocType y)
{

    Location lowerEdge;
    lowerEdge.x = 0;
    lowerEdge.y = 0;
    Location upperEdge;
    upperEdge.x = BOARD_X - 1;
    upperEdge.y = BOARD_Y - 1;
    
    
    for(int r = 0; r < NUM_ROBOTS; r++)
    {
        if(r == curRobot)
        {
            continue;
        }
        
        if(curPieces.robots[r].y == y)
        {
            BoardLocType robX = curPieces.robots[r].x;
            if(robX < x)
            {
                if(robX > lowerEdge.x)
                {
                    lowerEdge.x = robX;
                }
            }
            else if(robX > x)
            {
                if(robX < upperEdge.x)
                {
                    upperEdge.x = robX;
                }
            }
            else
            {
                assert(false); // this means two robots have the same x/y coordinates. bad!
            }
        }
        
        if(curPieces.robots[r].x == x)
        {
            BoardLocType robY = curPieces.robots[r].y;
            if(robY < y)
            {
                if(robY > lowerEdge.y)
                {
                    lowerEdge.y = robY;
                }
            }
            else if(robY > x)
            {
                if(robY < upperEdge.y)
                {
                    upperEdge.y = robY;
                }
            }
            else
            {
                assert(false); // this means two robots have the same x/y coordinates. bad!
            }
        }
        
    }
}


// given a robot on the board, where can it move? Returns a set of x/y coordinates indicating where it can move. 
void WhereCanThisPieceMove(PieceLocations &curPieces, BoardLocType board[][BOARD_Y], int curRobot, vector<Location> &ret)
{
    assert(ret.size() == 0);
    ret.clear();
    
    BoardLocType x = curPieces.robots[curRobot].x;
    BoardLocType y = curPieces.robots[curRobot].y;
    
    Location result;
    

    
    // now walk through the board, checking each direction to see where the piece can move
    for(int i = x; i >= 0; i--)
    {
        if(board[i][y] & TOP_BLOCKED)
        {

            result.x = i;
            result.y = y;
            ret.push_back(result);
            break;
        }
    }
    
    result.x = -1;
    result.y = -1;
    
    
}

/*
bool populateBoard(BoardLocType *board)
{
    for(int i = 0; i < BOARD_X; i++)
    {
        for(int j = 0; j < BOARD_Y; j++)
        {
            
        }
    }
    return true;
}
 */


