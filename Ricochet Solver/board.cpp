//
//  board.cpp
//  Ricochet Solver
//
//  Created by Stephen Sisk on 4/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "board.h"


Board::Board(short x, short y)
{
    size.x = x;
    size.y = y;
    
    board = new BoardLocType[x * y];
}

Board::~Board()
{
    delete[] board;
}


void Board::print(BoardState state, ostream &fout, vector<Location> *moves)
{
    char ROBOT_COLORS[NUM_ROBOTS][10] = {"red", "green", "orange"};
    
    time_t rawtime;
    time(&rawtime);
    
    fout  << ctime(&rawtime) << "<table style=\"border: 3px solid #000\" width=400 height=400>\r\n";
    
    char topChunk[100] = "border-top: 1px solid #000;";
    char leftChunk[100] = "border-left: 1px solid #000;";
    char bottomChunk[100] = "border-bottom: 1px solid #000;";
    char rightChunk[100] = "border-right: 1px solid #000;";
    
    
    for(int j = 0; j < size.y; j++)
    {
        fout << "<tr>\r\n";
        for(int i = 0; i < size.x; i++)
        {
            fout << "<td width=80 height=80 style=\"border: 1px solid #F3F3F3;";
            switch(get(i, j))
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
                if(state.robots[r].x == i && state.robots[r].y == j)
                {
                    fout << "<font size=24 color=" << ROBOT_COLORS[r] << ">x</font>";
                }
            }
            
            if(moves)
            {
                for(int m = 0; m < moves->size(); m++)
                {
                    if(moves->at(m).x == i && moves->at(m).y == j)
                    {
                        fout << "m" << m;
                    }
                }
            }
            
            if(state.target.x == i && state.target.y == j)
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




/* This function is a helper function for WhereCanThisPieceMove
 
 determine if the pieces define any edges for us. since the pieces aren't on the board, rather than check every piece's x/y coordinates on every single pass of the "can it move here" phase, we will instead just figure out if any pieces lie on the same x or y lines, and make sure that we don't go past those pieces when we're checking the board. 
 
 curRobot: the robot for which we are determining the edges
 
 lowerEdge & upperEdge: returns the farthest place a piece can move in a direction. That is, if no piece is in a direction, it returns 0 for lower edge or BOARD_*  for upperEdge. If a robot is blocking the path, it returns the position just before that robot. 
 
 
 */
void Board::DetermineEdges(int curRobot, BoardState state, BoardLocType x, BoardLocType y, Location &lowerEdge, Location &upperEdge)
{
    
    
    lowerEdge.x = 0;
    lowerEdge.y = 0;
    
    upperEdge.x = size.x - 1;
    upperEdge.y = size.y - 1;
    
    
    for(int r = 0; r < NUM_ROBOTS; r++)
    {
        if(r == curRobot)
        {
            continue;
        }
        
        if(state.robots[r].y == y)
        {
            //okay, we have a robot that's in the same row (y position) as the moving robot
            BoardLocType robX = state.robots[r].x;
            // is the robot to the left or right of the robot?
            if(robX < x)
            {
                // okay, it's to the left of the moving robot. Is to the right of the wall or another robot we've seen before? 
                if(robX >= lowerEdge.x)
                {
                    // put the edge one off of the robot since it can't move onto the robot. 
                    lowerEdge.x = robX + 1;
                }
            }
            else if(robX > x)
            {
                if(robX <= upperEdge.x)
                {
                    upperEdge.x = robX - 1;
                }
            }
            else
            {
                assert(false); // this means two robots have the same x/y coordinates. bad!
            }
        }
        
        if(state.robots[r].x == x)
        {
            BoardLocType robY = state.robots[r].y;
            if(robY < y)
            {
                if(robY >= lowerEdge.y)
                {
                    lowerEdge.y = robY + 1;
                }
            }
            else if(robY > y)
            {
                if(robY <= upperEdge.y)
                {
                    upperEdge.y = robY - 1;
                }
            }
            else
            {
                assert(false); // this means two robots have the same x/y coordinates. bad!
            }
        }
        
    }
}

void Board::TestDetermineEdges()
{
    Board testBoard(5, 5);
    // define base state
    Location lowerEdge;
    Location upperEdge;
    
    BoardState state;
    
    state.robots[0].x = 2;
    state.robots[0].y = 3;
    state.robots[1].x = 2;
    state.robots[1].y = 1;
    state.robots[2].x = 3;
    state.robots[2].y = 3;
    state.target.x = INVALID_LOC;
    state.target.x = INVALID_LOC;
    
    //no move case - piece above & right
    testBoard.DetermineEdges(0, state, 2, 3, lowerEdge, upperEdge);
    assert(lowerEdge.x == 0);
    assert(lowerEdge.y == 2);
    assert(upperEdge.x == 2);
    assert(upperEdge.y == testBoard.size.y - 1);
    lowerEdge.x = INVALID_LOC;
    lowerEdge.y = INVALID_LOC;
    upperEdge.x = INVALID_LOC;
    upperEdge.y = INVALID_LOC;
    
    //no move case - piece below & left
    state.robots[1].x = 2;
    state.robots[1].y = 4;
    state.robots[2].x = 1;
    state.robots[2].y = 3;
    testBoard.DetermineEdges(0, state, 2, 3, lowerEdge, upperEdge);
    assert(lowerEdge.x == 2);
    assert(lowerEdge.y == 0);
    assert(upperEdge.x == testBoard.size.x - 1);
    assert(upperEdge.y == 3);
    lowerEdge.x = INVALID_LOC;
    lowerEdge.y = INVALID_LOC;
    upperEdge.x = INVALID_LOC;
    upperEdge.y = INVALID_LOC;
    
    //two robots on the same axis - later one wins
    state.robots[1].x = 0;
    state.robots[1].y = 3;
    state.robots[2].x = 1;
    state.robots[2].y = 3;
    testBoard.DetermineEdges(0, state, 2, 3, lowerEdge, upperEdge);
    assert(lowerEdge.x == 2);
    assert(lowerEdge.y == 0);
    assert(upperEdge.x == testBoard.size.x - 1);
    assert(upperEdge.y == testBoard.size.y - 1);    
    lowerEdge.x = INVALID_LOC;
    lowerEdge.y = INVALID_LOC;
    upperEdge.x = INVALID_LOC;
    upperEdge.y = INVALID_LOC;
    
    //two robots on the same axis - earlier one wins
    state.robots[1].x = 1;
    state.robots[1].y = 3;
    state.robots[2].x = 0;
    state.robots[2].y = 3;
    testBoard.DetermineEdges(0, state, 2, 3, lowerEdge, upperEdge);
    assert(lowerEdge.x == 2);
    assert(lowerEdge.y == 0);
    assert(upperEdge.x == testBoard.size.x - 1);
    assert(upperEdge.y == testBoard.size.y - 1);    
    lowerEdge.x = INVALID_LOC;
    lowerEdge.y = INVALID_LOC;
    upperEdge.x = INVALID_LOC;
    upperEdge.y = INVALID_LOC;   
    
    
}


// given a robot on the board, where can it move? Returns a set of x/y coordinates indicating where it can move. 
void Board::WhereCanThisPieceMove(BoardState state, int curRobot, vector<Location> &ret)
{
    assert(ret.size() == 0);
    ret.clear();
    
    BoardLocType x = state.robots[curRobot].x;
    BoardLocType y = state.robots[curRobot].y;
    
    Location result;
    
    Location lowerEdge;
    lowerEdge.x = 0;
    lowerEdge.y = 0;
    Location upperEdge;
    upperEdge.x = 0;
    upperEdge.y = 0;
    
    DetermineEdges(curRobot, state, x, y, lowerEdge, upperEdge);
    
    // now walk through the board, checking each direction to see where the piece can move
   
    bool moveHere = false;
    
    //left
    for(int i = x; i >= lowerEdge.x; i--)
    {
        moveHere = false;
        if((get(i, y) & LEFT_BLOCKED))
        {
            // if the current position is blocked to the right, stay in our current spot 
            if(i == x)
            {
                // hey! we didn't move at all - that's not a valid move. 
                break;
            }
            moveHere = true;
        }
        else if(i== lowerEdge.x)
        {
            if(i == x)
            {
                // hey! we didn't move at all - that's not a valid move. 
                break;
            }
            // the position that we're currently on is the farthest we can move - so move there!
            moveHere = true;
        }
        
        if(moveHere == true)
        {
            result.x = i;
            result.y = y;
            ret.push_back(result);
            break;
        }
    }
    
    //right
    for(int i = x; i <= upperEdge.x; i++)
    {
        moveHere = false;
        if((get(i, y) & RIGHT_BLOCKED))
        {
            // if the current position is blocked to the right, stay in our current spot 
            if(i == x)
            {
                // hey! we didn't move at all - that's not a valid move. 
                break;
            }
            moveHere = true;
        }
        else if(i == upperEdge.x)
        {
            if(i == x)
            {
                // hey! we didn't move at all - that's not a valid move. 
                break;
            }
            // the position that we're currently on is the farthest we can move - so move there!
            moveHere = true;
        }
        
        if(moveHere == true)
        {
            result.x = i;
            result.y = y;
            ret.push_back(result);
            break;
        }
    }
    
    //up
    for(int j = y; j >= lowerEdge.y; j--)
    {
        
        moveHere = false;
        if((get(x, j) & TOP_BLOCKED))
        {
            // if the current position is blocked to the right, stay in our current spot 
            if(j == y)
            {
                // hey! we didn't move at all - that's not a valid move. 
                break;
            }
            moveHere = true;
        }
        else if(j== lowerEdge.y)
        {
            if(j == y)
            {
                // hey! we didn't move at all - that's not a valid move. 
                break;
            }
            // the position that we're currently on is the farthest we can move - so move there!
            moveHere = true;
        }
        
        if(moveHere == true)
        {
            result.x = x;
            result.y = j;
            ret.push_back(result);
            break;
        }
        
    }
    
    //down
    for(int j = y; j <= upperEdge.y; j++)
    {
        moveHere = false;
        if((get(x, j) & BOTTOM_BLOCKED))
        {
            // if the current position is blocked to the right, stay in our current spot 
            if(j == y)
            {
                // hey! we didn't move at all - that's not a valid move. 
                break;
            }
            moveHere = true;
        }
        else if(j== upperEdge.y)
        {
            if(j == y)
            {
                // hey! we didn't move at all - that's not a valid move. 
                break;
            }
            // the position that we're currently on is the farthest we can move - so move there!
            moveHere = true;
        }
        
        if(moveHere == true)
        {
            result.x = x;
            result.y = j;
            ret.push_back(result);
            break;
        }
    }
}

void testLocationVector(vector<Location> &expected, vector<Location> &actual, ostream &out, Board& testBoard, BoardState &state, const char* testName)
{
    
    out << "<br><br><table><tr><td colspan=2><font size=24pt>" << testName << "</font><br>";
    bool testSucceeded = true;
    if(!expected.size() == actual.size())
    {
        out << "<font color=red>Failed size test</font><br>";
        testSucceeded = false;
    }
    
    bool foundAll = true;
    for(int i = 0; i < expected.size(); i++)
    {
        bool found = false;
        for(int j = 0; j < actual.size(); j++)
        {
            if(expected[i].x == actual[j].x && expected[i].y == actual[j].y)
            {
                found = true;
            }
        }
        foundAll = foundAll && found;
    }
    
    if(!foundAll)
    {
        out << "<font color=red>Couldn't find all of the moves.</font><br>";
        testSucceeded = false;
    }
    
    out << "</td></tr>";
    //if(!testSucceeded) always show it for now
    {
        out << "<tr><td>Expected</td><td>Actual</td></tr>";
        out << "<tr><td>";
        testBoard.print(state, out, &expected);
        out << "</td><td>";
        testBoard.print(state, out, &actual);
        out << "</td></tr>";
    }
    out << "</table>";
}

void Board::TestGet(ostream &out)
{
    Board testBoard(2, 2);
    
    // define base state
    BoardLocType brd[4] =
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED | BOTTOM_BLOCKED, RIGHT_BLOCKED | BOTTOM_BLOCKED
    };
    
    memcpy(testBoard.board, brd, sizeof(brd));
    
    assert(testBoard.get(0, 0) == (LEFT_BLOCKED | TOP_BLOCKED));
    assert(testBoard.get(0, 1) == (LEFT_BLOCKED | BOTTOM_BLOCKED));
    assert(testBoard.get(1, 0) == (TOP_BLOCKED | RIGHT_BLOCKED));
    assert(testBoard.get(1, 1) == (RIGHT_BLOCKED | BOTTOM_BLOCKED));
    
    Board testBoard2(5, 5);
    
    // define base state
    BoardLocType brd2[25] =
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    memcpy(testBoard2.board, brd2, sizeof(brd2));
    
    assert(testBoard2.get(0, 0) == (LEFT_BLOCKED | TOP_BLOCKED));
    assert(testBoard2.get(2, 1) == (LEFT_BLOCKED | BOTTOM_BLOCKED));
    assert(testBoard2.get(3, 3) == (EMPTY));
    assert(testBoard2.get(0, 4) == (BOTTOM_BLOCKED | LEFT_BLOCKED));
}

void Board::TestWhereCanThisPieceMove(ostream &out)
{
    
    Board testBoard(5, 5);
    
    // define base state
    BoardLocType brd[25] =
    {
         LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
         LEFT_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
         LEFT_BLOCKED, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
         LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
         BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    memcpy(testBoard.board, brd, sizeof(brd));
    
    Location lowerEdge;
    Location upperEdge;
    
    BoardState state;
    
    state.robots[0].x = 2;
    state.robots[0].y = 3;
    state.robots[1].x = 2;
    state.robots[1].y = 1;
    state.robots[2].x = 3;
    state.robots[2].y = 3;
    state.target.x = INVALID_LOC;
    state.target.x = INVALID_LOC;
    
    
    vector<Location> ret;
    vector<Location> expected;
    
    //wall top
    state.robots[0].x = 2;
    state.robots[0].y = 3;
    testBoard.WhereCanThisPieceMove(state, 0, ret);
    expected.push_back(Location(0,3));
    expected.push_back(Location(2, 2));
    expected.push_back(Location(2, 4));
    testLocationVector(expected, ret, out, testBoard, state, "wall top");
    expected.clear();
    ret.clear();
    
    //wall left
    state.robots[0].x = 4;
    state.robots[0].y = 1;
    testBoard.WhereCanThisPieceMove(state, 0, ret);
    expected.push_back(Location(4, 0));
    expected.push_back(Location(3, 1));
    expected.push_back(Location(4, 4));
    testLocationVector(expected, ret, out, testBoard, state, "wall left");
    expected.clear();
    ret.clear();
    
    //wall right
    state.robots[0].x = 1;
    state.robots[0].y = 2;
    testBoard.WhereCanThisPieceMove(state, 0, ret);
    expected.push_back(Location(0, 2));
    expected.push_back(Location(1, 0));
    expected.push_back(Location(2, 2));
    expected.push_back(Location(1, 4));
    testLocationVector(expected, ret, out, testBoard, state, "wall right");
    expected.clear();
    ret.clear();
    
    //wall bottom
    state.robots[0].x = 2;
    state.robots[0].y = 0;
    testBoard.WhereCanThisPieceMove(state, 0, ret);
    expected.push_back(Location(0,0));
    expected.push_back(Location(4,0));
    testLocationVector(expected, ret, out, testBoard, state, "wall bottom");
    expected.clear();
    ret.clear();
    
    //walls all around
    Board testBoardAllWallsAround(5, 5);
    BoardLocType brdAllWallsAround[25] =
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, RIGHT_BLOCKED, TOP_BLOCKED | LEFT_BLOCKED | RIGHT_BLOCKED | BOTTOM_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, TOP_BLOCKED, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    memcpy(testBoard.board, brdAllWallsAround, sizeof(brdAllWallsAround));
    state.robots[0].x = 2;
    state.robots[0].y = 2;
    testBoardAllWallsAround.WhereCanThisPieceMove(state, 0, ret);
    // the piece can't move anywhere
    testLocationVector(expected, ret, out, testBoardAllWallsAround, state, "walls all around");
    expected.clear();
    ret.clear();
    
    //robots all around (part 1 - left/right)
    Board testBoardAllRobotsAround(5, 5);
    BoardLocType brdAllRobotsAround[25] =
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    memcpy(testBoard.board, brdAllRobotsAround, sizeof(brdAllRobotsAround));
    state.robots[0].x = 2;
    state.robots[0].y = 1;
    state.robots[1].x = 3;
    state.robots[1].y = 1;
    state.robots[2].x = 1;
    state.robots[2].y = 1;
    testBoardAllRobotsAround.WhereCanThisPieceMove(state, 0, ret);
    expected.push_back(Location(2,0));
    expected.push_back(Location(2,4));
    testLocationVector(expected, ret, out, testBoardAllRobotsAround, state, "robots all around (part 1 - left/right)");
    expected.clear();
    ret.clear();

    //robots all around (part 2 - top/bottom)
    state.robots[0].x = 2;
    state.robots[0].y = 2;
    state.robots[1].x = 2;
    state.robots[1].y = 1;
    state.robots[2].x = 2;
    state.robots[2].y = 3;
    testBoardAllRobotsAround.WhereCanThisPieceMove(state, 0, ret);
    expected.push_back(Location(0,2));
    expected.push_back(Location(4,2));
    testLocationVector(expected, ret, out, testBoardAllRobotsAround, state, "robots all around (part 2 - top/bottom)");
    expected.clear();
    ret.clear();
    
    
}


void Board::TestPrintBoard(ostream &out)
{
    out << "Test Print board<br>";
    
    BoardState state;
    
    state.robots[0].x = 0;
    state.robots[0].y = 0;
    state.robots[1].x = 3;
    state.robots[1].y = 3;
    state.robots[2].x = 2;
    state.robots[2].y = 1;
    state.target.x = 4;
    state.target.y = 3;
    
    
    Board testBoard(5, 5);
    
    
    
    BoardLocType brd[25] = 
    {
         LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED ,
         LEFT_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
         LEFT_BLOCKED, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
         LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
         BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    memcpy(testBoard.board, brd, sizeof(brd));
    

    
    testBoard.print(state, out);
    
    out << "end Test Print board<br><br><br>";
    
}