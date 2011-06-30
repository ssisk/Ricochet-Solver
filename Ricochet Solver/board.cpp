//
//  board.cpp
//  Ricochet Solver
//
//  Created by Stephen Sisk on 4/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
//  The Board class is responsible for maintaining the state of the board 
//  and determining where a piece can move (ie, collision detection between a 
//  piece and the wall, piece and another piece)

#include "board.h"

// x and y are dimensions of the board
// target is the location of the victory location (ie, where the correct 
//      color robot has to move in order for the player to win)
// brd is the board you'd like to solve. it's a one dimensional array used
//      to store a two dimensional board, so the board is stored in the following
//      format:

//      0 1 2
//      3 4 5
//      6 7 8
//
//      (assuming a 3x3 board)
Board::Board(short x, short y, Location target, BoardLocType brd[])
{
    size.x = x;
    size.y = y;
    
    board = new BoardLocType[x * y];
    
    assert(brd); // umm.... we need a board! Don't bother checking for this, 
                    //if the caller didn't pass a brd, it's a serious problem and we should crash
    memcpy(board, brd, (sizeof(BoardLocType) * x * y));

    this->_target = target;
    
}

Board::~Board()
{
    delete[] board;
}

// main function for printing an HTML version of the current board state. 
// state determines where the pieces are
// fout is where the board will be printed
// overlays lets you print other pieces on the board - see the BoardOverlay struct
//      for more details
//
// robots are Xes, the target is an O
void Board::print(BoardState state, ostream &fout, vector<BoardOverlay> *overlays)
{
    char ROBOT_COLORS[10][10] = {"red", "pink", "orange", "grey", "purple", "", "", "", "", "black"};
    
    time_t rawtime;
    time(&rawtime);
    
    int width = size.x * 80;
    int height = size.y * 80;
    
    fout  << ctime(&rawtime) << "<table style=\"border: 3px solid #000\" width=" << width << " height=" << height << ">\r\n";
    
    // the css to get inserted into the style for the given table cell. We use the border of the table to show where the walls in the maze are
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
                    
                case LEFT_BLOCKED | RIGHT_BLOCKED:
                    fout << leftChunk << rightChunk;
                    break;
                case TOP_BLOCKED | BOTTOM_BLOCKED:
                    fout << topChunk << bottomChunk;
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
            
            if(overlays)
            {
                for(int m = 0; m < overlays->size(); m++)
                {
                    if(overlays->at(m).loc.x == i && overlays->at(m).loc.y == j)
                    {
                        
                        fout << "<font size=24 color=" << ROBOT_COLORS[overlays->at(m).robotColor] << ">" << overlays->at(m).text << "</font>";
                    }
                }
            }
            
            if(_target.x == i && _target.y == j)
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


Location Location::Invalid = Location(INVALID_LOC, INVALID_LOC);


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
        
        //rinse and repeat for x movement. should always match the y movement
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
    BoardLocType brd[25]; 
    Board testBoard(5, 5, Location(), brd);
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


// given a robot on the board, what are valid moves? 
// 
// state - current board state, indicating location of the other robots
// curRobot - the robot to move
// ret - returns a set of x/y coordinates indicating where curRobt can move. Ret should be empty when this function is called
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

// given a current boardstate and set of potential moves, print out where a piece can move
// state - current state of the board
// moves - the locations that are available to move onto
// robotColor - color to use when printing. It is suggested that all of the moves be for the robot indicated by the index of robotColor
void Board::PrintMoves(BoardState &state, vector<Location> &moves, ostream &out, int robotColor)
{
    vector<BoardOverlay> overlays;
    
    BoardOverlay newBO;
    newBO.robotColor = robotColor;
    for(int i = 0; i < moves.size(); i++)
    {
        newBO.loc = moves[i];
        snprintf(newBO.text, 5, "m%i", i);
        overlays.push_back(newBO);
        
    }
    
    print(state, out, &overlays);
}

// prints out the result of the unit tests for WhereCanThisPieceMove in HTML format
//
// expected - the correct set of moves for the piece
// actual - what was returned by the function
// out - where to print the HTML
// testBoard & state - the board and current state of the board
// testName - a user friendly name to be printed, describing the test
//
// The output will be a table consisiting of two columns showing expected and actual results
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
        testBoard.PrintMoves(state, expected, out, PRINTBOARD_NOCOLOR);
        out << "</td><td>";
        testBoard.PrintMoves(state, actual, out, PRINTBOARD_NOCOLOR);
        out << "</td></tr>";
    }
    out << "</table>";
}

// test the board::get function
void Board::TestGet(ostream &out)
{
    
    
    // define base state
    BoardLocType brd[4] =
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED | BOTTOM_BLOCKED, RIGHT_BLOCKED | BOTTOM_BLOCKED
    };
    
    Board testBoard(2, 2, Location(), brd);
    
    assert(testBoard.get(0, 0) == (LEFT_BLOCKED | TOP_BLOCKED));
    assert(testBoard.get(0, 1) == (LEFT_BLOCKED | BOTTOM_BLOCKED));
    assert(testBoard.get(1, 0) == (TOP_BLOCKED | RIGHT_BLOCKED));
    assert(testBoard.get(1, 1) == (RIGHT_BLOCKED | BOTTOM_BLOCKED));
    
    
    
    // define base state
    BoardLocType brd2[25] =
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    Board testBoard2(5, 5, Location(), brd2);
    
    assert(testBoard2.get(0, 0) == (LEFT_BLOCKED | TOP_BLOCKED));
    assert(testBoard2.get(2, 1) == (LEFT_BLOCKED | BOTTOM_BLOCKED));
    assert(testBoard2.get(3, 3) == (EMPTY));
    assert(testBoard2.get(0, 4) == (BOTTOM_BLOCKED | LEFT_BLOCKED));
}

// test the WhereCanThisPieceMove function. Note that it does not assert, but rather
// outputs the results to out for user inspection. 
void Board::TestWhereCanThisPieceMove(ostream &out)
{
    
    
    
    // define base state
    BoardLocType brd[25] =
    {
         LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
         LEFT_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
         LEFT_BLOCKED, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
         LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
         BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    Board testBoard(5, 5, Location(), brd);
    
    
    
    Location lowerEdge;
    Location upperEdge;
    
    BoardState state;
    
    state.robots[0].x = 2;
    state.robots[0].y = 3;
    state.robots[1].x = 2;
    state.robots[1].y = 1;
    state.robots[2].x = 3;
    state.robots[2].y = 3;

    
    
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
    BoardLocType brdAllWallsAround[25] =
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, RIGHT_BLOCKED, TOP_BLOCKED | LEFT_BLOCKED | RIGHT_BLOCKED | BOTTOM_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, TOP_BLOCKED, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    Board testBoardAllWallsAround(5, 5, Location(), brdAllWallsAround);
    state.robots[0].x = 2;
    state.robots[0].y = 2;
    state.robots[1].x = 0;
    state.robots[1].y = 0;
    state.robots[2].x = 4;
    state.robots[2].y = 4;
    testBoardAllWallsAround.WhereCanThisPieceMove(state, 0, ret);
    // the piece can't move anywhere
    testLocationVector(expected, ret, out, testBoardAllWallsAround, state, "walls all around");
    expected.clear();
    ret.clear();
    
    //robots all around (part 1 - left/right)
    
    BoardLocType brdAllRobotsAround[25] =
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    Board testBoardAllRobotsAround(5, 5, Location(), brdAllRobotsAround);
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

// tests the Board::print function
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
    
    BoardLocType brd[25] = 
    {
         LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED ,
         LEFT_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
         LEFT_BLOCKED, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
         LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
         BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    Board testBoard(5, 5, Location(4, 3), brd);
    

    
    testBoard.print(state, out);
    
    out << "end Test Print board<br><br><br>";
    
}

// produces a hash for the boost library's hashing functions. Allows boardstates to be used as keys in a hashtable. 
std::size_t hash_value(BoardState const& b)
{
    size_t seed = 0;
    for(int i = 0; i < NUM_ROBOTS; i++)
    {
        boost::hash_combine(seed, b.robots[i].x);
        boost::hash_combine(seed, b.robots[i].y);
    }
    return seed;
}
