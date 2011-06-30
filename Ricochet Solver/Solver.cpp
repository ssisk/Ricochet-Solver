//
//  Solver.cpp
//  Ricochet Solver
//
//  Created by Stephen Sisk on 5/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
// solver is responsible for the algorithm of how we try and solve the board
//
// One of the trickier problems when working with this algorithm was figuring out how to debug it. To that end, there's a fair bit of code in this file devoted to getting some visibility in how the algorithm works.

#include "Solver.h"
#include <stack>
#include <boost/functional/hash.hpp>

// setting this flag makes the program *much* slower, but outputs a lot of useful debug information
//#define ALG_DEBUG_VISIT_COUNTS
//#define ALG_DEBUG_SUGGESTED_MOVES

// print out how many times we've visited a given location on the board
void PrintBoardCounts(BoardState &state, ostream & out, int curRobot, long visitCounts[20][20][NUM_ROBOTS], Board *board)
{
    vector<BoardOverlay> overlays;
    
    BoardOverlay newBO;
    
    for(int x = 0; x < board->GetSize().x; x++)
    {
        for(int y = 0; y < board->GetSize().y; y++)
        {
            for(int curRobot = 0; curRobot < NUM_ROBOTS; curRobot++)
            {
                if(visitCounts[x][y][curRobot] != 0)
                {
                    newBO.robotColor = curRobot;
                    snprintf(newBO.text, sizeof(newBO.text), "%ld", visitCounts[x][y][curRobot]);
                    newBO.loc = Location(x, y);
                    overlays.push_back(newBO);
                }
            }
        }
    }
    
    board->print(state, out, &overlays);
}

// this class makes it possible to count how many times a given location on the board has been visited by a given piece

// This class is useful if you're having trouble figuring out whether or not the algorithm is getting pieces to move into a given area or not, and what the trends are in what part of the board different pieces are exploring, and what piece is moving a lot.

// One improvement I thought about is converting the output into a heat map since it'd be easier to read
class VisitCounter
{
public:
    VisitCounter():
    _visitCounts(),
    _visitCountsChange()
    {
        assert(_visitCounts[0][1][2] == 0); 
        assert(_visitCounts[1][3][0] == 0); 
        
    }
    
    // it is suggested that you only call this PrintCounts function occasionally (on the order of every 10k iterations or so
    void PrintCounts(BoardState &startingState, int winningRobot, ostream &out, Board *board)
    {
#ifdef ALG_DEBUG_VISIT_COUNTS
        out << "<table><tr><td>Piece Visits</td><td>Delta in Piece Visits</td></tr><tr><td>";
        
        //print out the board counts
        PrintBoardCounts(startingState, out, winningRobot, _visitCounts, board);
        
        out << "</td><td>";
        
        PrintBoardCounts(startingState, out, winningRobot, _visitCountsChange, board);
        
        out << "</td></tr></table>";
        
   
        for(int x = 0; x < board->GetSize().x; x++)
        {
            for(int y = 0; y < board->GetSize().y; y++)
            {
                for(int curRobot = 0; curRobot < 2; curRobot++)
                {
                    _visitCountsChange[x][y][curRobot] = 0;
                }
            }
        }
#endif
    }
    
    // increment the counts
    // x, y - the location of the robot
    // r - the index of the robot that's visiting that location
    void IncrementCounts(int x, int y, int r)
    {
        _visitCounts[x][y][r]++;
        _visitCountsChange[x][y][r]++;
    }
    
private:
    long _visitCounts[20][20][NUM_ROBOTS];
    long _visitCountsChange[20][20][NUM_ROBOTS];
};


RicochetSolver::RicochetSolver(Board &board)
{
    this->board = &board;
}



// check to see if possibleMove exists in stackToCheck and updates the cost if curCost is less that stackToCheck's cost
//
// stackToCheck - the stack to check and update
// possibleMove - the location that's being checked
// moveFound - returns whether or not the move was found in stackToCheck
// curCosts
void RicochetSolver::UpdateMoveInStack(vector<MoveCost> &stackToCheck, Location possibleMove, bool &moveFound, int curCost)
{
    // OPPORTUNITY FOR OPTIMIZATION - finding the right location quickly here
    for(int j = 0; j < stackToCheck.size(); j++)
    {
        if(possibleMove == stackToCheck[j].loc)
        {
            moveFound = true;
            if(stackToCheck[j].cost > curCost)
            {
                stackToCheck[j].cost = curCost;
            }
        }
    }
}

/*
 given a set of state, determines everywhere a given piece could move without moving other pieces. This was a test function written to investigate whether or not I really liked how the Board class was set up before I started working on the Solve functions, and it's not used to help solve the board at all. 
 
 returns: a vector of MoveCosts representing the potential boardstates and their cost (cost = number of moves to get there)
 
 startingState - the location of all the pieces on the board
 curRobot - the robot which will move on the board
 closedMoves - returns all the locations on the board that the robot visited
 */
void RicochetSolver::ColorBoardForThisPiece(BoardState startingState, int curRobot, vector<MoveCost> &closedMoves)
{
    assert(curRobot < NUM_ROBOTS && curRobot >= 0);
    closedMoves.clear();
    
    //DATA STRUCTURE THOUGHTS
    //closed stack = places that we've already looked at (ie, we've added all potential moves to the open stack)
    //open stack = places we haven't looked at yet (they may have moves available from them that aren't in the open or closed statck yet)
    
    //both open and closed stack have up to date costs for them
    
    vector<MoveCost> openMoves;   

    //this hold the possible moves from a given location - as we move the robot around the board, this will hold the result from WhereCanThisPieceMove, telling us where the robot can go next. It's used temporarily, until the moves are taken from it and put into the open Moves stack, or discarded b/c they're already in the open or closed stacks
    vector<Location> possibleMoves;
    
    
    //push the current position on the open stack
    Location robStartingLoc = startingState.robots[curRobot];
    MoveCost curMove;
    curMove.loc = robStartingLoc;
    curMove.cost = 0;
    openMoves.push_back(curMove);
    

    //loop
    int curCost = 0;
    MoveCost newMove;
    while(openMoves.size() > 0)
    {
        cout << "open moves size: " << openMoves.size() << "\r\n";
        //we'll be reusing the startingState struct, so we're clearing it just to make sure we move the current robot around every time. The rest of the state will remain the same as we check where this robot can move to.
        startingState.robots[curRobot].x = INVALID_LOC;
        startingState.robots[curRobot].y = INVALID_LOC;
        
        //pull a position off of the open stack
        curMove = openMoves.back();
        openMoves.pop_back(); //I find it lame that pop_back doesn't return the value it's popping
        
        //calculate the cost of all the moves that we'll be taking
        curCost = curMove.cost + 1;
        
        cout << "current position: " << (int) curMove.loc.x << ", " << (int) curMove.loc.y << "\r\n";
        
        //WhereCanThisPieceMove for that position. 
        startingState.robots[curRobot] = curMove.loc;
        board->WhereCanThisPieceMove(startingState, curRobot, possibleMoves);
        
        //look for the possibleMoves in the open stack. if a move isn't in the open stack, push it onto the open stack. If one of the moves is in the open stack, compare cost and update it. 
        // also checks to see if the moves are in the closed stack as well
        for (int i = 0; i < possibleMoves.size(); i++ )
        {
            cout << "possible move " << i << ": " << (int) possibleMoves[i].x << ", " << (int) possibleMoves[i].y << " ";
            bool moveFound = false;
            
            
            UpdateMoveInStack(openMoves, possibleMoves[i], moveFound, curCost);
            
            if(!moveFound)
            {
                UpdateMoveInStack(closedMoves, possibleMoves[i], moveFound, curCost);
            }
            
            if(!moveFound)
            {
                cout << "move not found\r\n";
                newMove.loc = possibleMoves[i];
                newMove.cost = curCost;
                openMoves.push_back(newMove);
            }
        }
        
        possibleMoves.clear();
        
        //put the current position in the closed stack
        closedMoves.push_back(curMove);
    }
    
    //at this point, closedMoves contains the proper return values
}

//given a set of moveCosts, prints them out in a user friendly way
void RicochetSolver::PrintMoveCosts(vector<MoveCost> &MoveCosts, BoardState &state, ostream & out, int curRobot)
{
    vector<BoardOverlay> overlays;
    
    BoardOverlay newBO;
    newBO.robotColor = curRobot;
    for(int i = 0; i < MoveCosts.size(); i++)
    {
        snprintf(newBO.text, sizeof(newBO.text), "%i", MoveCosts[i].cost);
        newBO.loc = MoveCosts[i].loc;
        overlays.push_back(newBO);
    }
    board->print(state, out, &overlays);
}


// unit test for ColorBoardForThisPiece. Does not assert, simply outputs HTML
void RicochetSolver::TestColorBoardForThisPiece(ostream &out)
{
    out << "<h2>Test ColorBoardForThisPiece</h2>";
    
    BoardLocType brd[25] = 
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    Board testBoard(5, 5, Location(), brd);
    RicochetSolver solver(testBoard);
    
    BoardState state;
    state.robots[0].x = 2;
    state.robots[0].y = 3;
    state.robots[1].x = 2;
    state.robots[1].y = 1;
    state.robots[2].x = 3;
    state.robots[2].y = 3;
    
    vector<RicochetSolver::MoveCost> ret;
    
    out << "red<br>";
    solver.ColorBoardForThisPiece(state, 0, ret);
    solver.PrintMoveCosts(ret, state, out, 0);
    ret.clear();

    out << "green<br>";
    solver.ColorBoardForThisPiece(state, 1, ret);
    solver.PrintMoveCosts(ret, state, out, 1);
    ret.clear();
    
    out << "yellow<br>";
    solver.ColorBoardForThisPiece(state, 2, ret);
    solver.PrintMoveCosts(ret, state, out, 2);
    ret.clear();
}

// output the set of moves that the algorithm was considering and what moves it actually kept for investigation
// see class level notes in solver.h 
void RicochetSolver::DebugSuggestedMoves::Print(ostream &out, RicochetSolver *solver, BoardState startingState, int color)
{
#ifdef ALG_DEBUG_SUGGESTED_MOVES
    out << "<td>";
    solver->board->PrintMoves(startingState, _possibleMoves, out, color);
    out << "</td><td>";
    solver->board->PrintMoves(startingState, _actualMoves, out, color);
    out << "</td>";
#endif
}

// see class level notes in solver.h
void RicochetSolver::DebugSuggestedMoves::AddPossibleMoves(vector<Location> &possibleMoves)
{
    #ifdef ALG_DEBUG_SUGGESTED_MOVES
    _possibleMoves.insert(_possibleMoves.end(), possibleMoves.begin(), possibleMoves.end());
    _actualMoves.insert(_actualMoves.end(), possibleMoves.begin(), possibleMoves.end()); 
#endif
}
// see class level notes in solver.h
void RicochetSolver::DebugSuggestedMoves::RemoveMove(Location &move)
{
#ifdef ALG_DEBUG_SUGGESTED_MOVES
    vector<Location>::iterator itr = find(_actualMoves.begin(), _actualMoves.end(), move);
    
    assert(itr != _actualMoves.end());
    _actualMoves.erase(itr);
#endif
}

// see class level notes in solver.h
void RicochetSolver::DebugSuggestedMoves::Clear()
{
#ifdef ALG_DEBUG_SUGGESTED_MOVES
    _possibleMoves.clear();
    _actualMoves.clear(); 
#endif
}

void RicochetSolver::DebugSuggestedMoves::StartBoardState(ostream &out)
{
#ifdef ALG_DEBUG_SUGGESTED_MOVES
    out << "<table><tr>";
#endif
}

void RicochetSolver::DebugSuggestedMoves::EndBoardState(ostream &out)
{
#ifdef ALG_DEBUG_SUGGESTED_MOVES
    out << "</tr></table>";
#endif
    
}


// depth first search impl
// This is where all the sexy stuff happens
//

// Given a starting state, this function will find the smallest number of movest that will move robot 0 into the target location on the board. 
//
// This function uses a depth first search strategy to do so. From the starting position, we look for all possible moves from that position, putting them in a stack. We then go through the stack, looking at each move possible from that state, and storing them in the stack. We store the list of all past states already looked at in a hash table wrapped by the  "all past states investigated" class. Before we add new elements to the stack, we check to see whether we've already stored that state before in the hash table. If we have already stored the state, we don't re-investigate the state *except* when we got to the state in fewer moves this time - in that case, we re-examine it.

// Why do we store past states seen? If we don't, we'll continually re-examine the same states again and again. Once you get a fair sized board, the history stack isn't enough to prevent you from re-examining the same areas again and again.

// Why do we store the number of moves it took to get to a given location? If the first time you get to a given board state it wasn't the most efficient way to get there, you will block yourself from finding the more efficient way to get there later. 

// One avenue for future investigation is whether breadth first search or depth first search are more efficient in terms of storage & # of iterations it takes to find the solution. 

// maxDepth - the solution found will have a certain # of moves. maxDepth defines the maximum number of moves that will be considered for a solution
// startingState - the position of the pieces at the start
// allSolutions - Returns a list of histories. Each history is stored as a vector of boardstates. Those boardstates represent the moves that were made to get to the solution. The shortest one should be considered the true solution. Should be empty when passed in to the function. 
// out - debug out stream. Some comments will go to cout, but any debugging which involves printing out the state of the board will be streamed to this parameter
void RicochetSolver::SolveDepthFirst_withHashedHistory(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out)
{
    cout << "start\r\n";
    assert(allSolutions.size() == 0);
    allSolutions.clear();
    
    // Treated as a stack. Holds the complete set of moves that we're going to investigate at some point in the future + directions on how to handle
    vector<BoardState> stateToInvestigate;
    
    //when we see this state pop off the stateToInvestigate stack, it means that we need to pop an element off of the history stack. When we are investigating a state, we push this element on stateToInvestigate, and then push the set of states that represent potential moves from that location. At the same time, we push the current state on the history stack. 
    
    BoardState PopHistory;
    PopHistory.robots[0] = Location::Invalid;
    
    // Treated as a stack. As the algorithm investigates various potential solutions to the board, this holds the complete history of how we got here. This will be added to allSolutions if the current solution turns out to be true
    vector<BoardState> history;
    
    // from the current position, what are all the moves that we could make, regardless of whether we've been to them before
    vector<Location> possibleMoves;
    BoardState currentState;
    BoardState newState;
    
    //our starting state is the first thing we investigate!
    stateToInvestigate.push_back(startingState);
    VisitCounter visitCounts;
    int iterationCount = 0; 

    Location boardSize = board->GetSize();
    assert(20 > boardSize.x);
    assert(20 > boardSize.y);
    
    // the states filter tells us wheter or not we're allowed to investigate a given move
    AllPastStatesInvestigatedFilter statesFilter;
    DebugSuggestedMoves debugSuggestedMoves;
    
    while (stateToInvestigate.size() > 0)
    {
        iterationCount++;
        
        if(iterationCount >= 10000)
        {
            cout << "10k iterations. hashtable size: " << statesFilter.size() << "\r\n";
            
            iterationCount = 0;
            
            visitCounts.PrintCounts(startingState, winningRobot, out, board);
        }

        //get the current board state
        currentState = stateToInvestigate.back();
        stateToInvestigate.pop_back();
        
        //are we at the "clear history" item?
        while(currentState.robots[0] == Location::Invalid)
        {
            //cout << "popping history\r\n";
            assert(history.size() > 0);
            history.pop_back();
            if(stateToInvestigate.size() <= 0)
            {
                return; // we are done here! We finish on this because the first item that will be in the stateToInvestigate is a "pop history" marker
            }
            currentState = stateToInvestigate.back();
            stateToInvestigate.pop_back();
        }
        
        //add it to history
        history.push_back(currentState);
        
        //also, push a marker in the investigate stack so that we remember to pop the history list when we hit this again. 
        stateToInvestigate.push_back(PopHistory);
       
        debugSuggestedMoves.StartBoardState(out);
        
        //for each piece on the board...
        for(int r = 0; r < NUM_ROBOTS; r++) 
        {       
            debugSuggestedMoves.Clear();
            possibleMoves.clear();
            //get the possible moves
            board->WhereCanThisPieceMove(currentState, r, possibleMoves);
            debugSuggestedMoves.AddPossibleMoves(possibleMoves);
            
            //for each of the possible moves...
            for(int m = 0; m < possibleMoves.size(); m++)
            {
                //construct the new boardstate
                newState = currentState;
                newState.robots[r] = possibleMoves[m];
                
                //is the move a victory move?
                //you can't win if the current robot isn't the robot you're trying to move to the target
                if(r == winningRobot)
                {
                    if(possibleMoves[m] == board->GetTarget())
                    {
                        //yay! We found the solution! Now, wasn't that worth it? 
                        history.push_back(newState);
                        allSolutions.push_back(history);
                        history.pop_back();
                        cout << "found a solution\r\n";
                        cout << "setting new max depth with -1: " << history.size() << "\r\n";
                        if(maxDepth >= history.size())
                        {
                            maxDepth = (int) history.size() - 1; // I do it -1 since I want to look for a shorter solution than the current
                        }
#ifdef ALG_DEBUG_VISIT_COUNTS
                        visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);
#endif
                        
                        continue;
                    }
                }
                
                //are we at max depth?
                if (history.size() >= maxDepth) 
                {  
                    //when we are max depth we don't add a move to the investigate list
                    //cout << "at max depth\r\n";
                    debugSuggestedMoves.RemoveMove(possibleMoves[m]);
                    continue;
                }
                else
                {
                    //is the move already in state to investigate?
                    if(statesFilter.AllowState(newState, history.size()))
                    {
                        //cout << "pushing state\r\n";
                        stateToInvestigate.push_back(newState);
#ifdef ALG_DEBUG_VISIT_COUNTS           
                        visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);
#endif
                        
                    }
                    else
                    {
#ifdef ALG_DEBUG_SUGGESTED_MOVES
                        debugSuggestedMoves.RemoveMove(possibleMoves[m]);
#endif
                    }
                }
                
                
            } 
#ifdef ALG_DEBUG_SUGGESTED_MOVES
           debugSuggestedMoves.Print(out, this, currentState, r); 
#endif
        } //end of robot loop
       
        
        debugSuggestedMoves.EndBoardState(out);
        
    }
    
}

//given a set of moves represented by states, output all of the unique positions that robots occupied on the board
void printBoardStates(Board &board, vector<BoardState> &states, ostream &out)
{
    vector<BoardOverlay> overlays;
    BoardOverlay newBO;
    
    BoardState lastState = states[0];
    for(int i = 1; i < states.size(); i++)
    {
        int changedRobot = -1;
        //what changed between the last state and this one? 
        for(int r = 0; r < NUM_ROBOTS; r++)
        {
            if(lastState.robots[r] != states[i].robots[r])
            {
                changedRobot = r;
                break;
            }
        }
        assert(changedRobot != -1);
        
        newBO.robotColor = changedRobot;
        newBO.loc = states[i].robots[changedRobot];
        snprintf(newBO.text, 5, " %i", i);
        overlays.push_back(newBO);
        
        lastState = states[i];
    }
    board.print(states[0], out, &overlays);
}

//for now, this function is the only way to run the solver. Put your board and target in as shown below

void RicochetSolver::TestSolveDepthFirst(ostream &out)
{
    vector<vector<BoardState> > allSolutions;
    
    BoardLocType brd[] = 
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    Board board(3, 3, Location(1, 1), brd); 
    BoardState startingState;
    startingState.robots[0] = Location(2, 0);
    startingState.robots[1] = Location(0, 0);
    startingState.robots[2] = Location(0, 2);
    startingState.robots[3] = Location(1, 1);
    startingState.robots[4] = Location(2, 2);
    RicochetSolver solver(board);
    solver.SolveDepthFirst_withHashedHistory(6, startingState, 0, allSolutions, out);
    
    out << "<h1>Depth First Solver</h1>";
    out << "# of solutions: " << allSolutions.size() << "<br>";
    for(int i = 0; i < allSolutions.size(); i++)
    {
        out << "<h3>solution " << i << " turns: " << allSolutions[i].size() << "</h3>";
        printBoardStates(board, allSolutions[i], out);
        
    }
    
    allSolutions.clear();
    BoardLocType brdMedium[] = 
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED | BOTTOM_BLOCKED, LEFT_BLOCKED, EMPTY, EMPTY, BOTTOM_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, RIGHT_BLOCKED, LEFT_BLOCKED, TOP_BLOCKED, EMPTY, EMPTY, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, EMPTY, BOTTOM_BLOCKED, BOTTOM_BLOCKED, EMPTY, BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, TOP_BLOCKED, EMPTY, RIGHT_BLOCKED, LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, TOP_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, EMPTY, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, EMPTY, BOTTOM_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED | TOP_BLOCKED, EMPTY, EMPTY, EMPTY, TOP_BLOCKED, TOP_BLOCKED, EMPTY, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, BOTTOM_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, BOTTOM_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, TOP_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | TOP_BLOCKED, EMPTY, TOP_BLOCKED, EMPTY, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    //Board boardMedium(10, 10, Location(2, 7), brdMedium); 
    //Board boardMedium(10, 10, Location(3, 9 ), brdMedium); 
    Board boardMedium(10, 10, Location(3, 7), brdMedium); 
    BoardState startingStateMedium;
    startingStateMedium.robots[0] = Location(9, 0);
    startingStateMedium.robots[1] = Location(0, 0);
    startingStateMedium.robots[2] = Location(0, 9);
    startingStateMedium.robots[3] = Location(9, 9);
    startingStateMedium.robots[4] = Location(1, 1);
    RicochetSolver solverMedium(boardMedium);

    solverMedium.SolveDepthFirst_withHashedHistory(20, startingStateMedium, 0, allSolutions, out);
    
    out << "<h1>Depth First Solver - Medium</h1>";
    out << "# of solutions: " << allSolutions.size() << "<br>";
    for(int i = 0; i < allSolutions.size(); i++)
    {
        out << "<h3>solution " << i << " turns: " << allSolutions[i].size() << "</h3>";
        printBoardStates(boardMedium, allSolutions[i], out);
        
    }
/*
    allSolutions.clear();
    BoardLocType brdMediumTricky[] = 
    {
        LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, EMPTY, EMPTY, RIGHT_BLOCKED | BOTTOM_BLOCKED, LEFT_BLOCKED, EMPTY, EMPTY, BOTTOM_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, RIGHT_BLOCKED, LEFT_BLOCKED, TOP_BLOCKED, EMPTY, EMPTY, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, EMPTY, BOTTOM_BLOCKED, BOTTOM_BLOCKED, EMPTY, BOTTOM_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, TOP_BLOCKED, EMPTY, RIGHT_BLOCKED, LEFT_BLOCKED | TOP_BLOCKED, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, TOP_BLOCKED, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, EMPTY, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, EMPTY, BOTTOM_BLOCKED, RIGHT_BLOCKED,
        LEFT_BLOCKED | TOP_BLOCKED, EMPTY, EMPTY, EMPTY, TOP_BLOCKED, TOP_BLOCKED, EMPTY, EMPTY, TOP_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED | RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, BOTTOM_BLOCKED | RIGHT_BLOCKED, LEFT_BLOCKED, BOTTOM_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | BOTTOM_BLOCKED, EMPTY, EMPTY, RIGHT_BLOCKED,
        LEFT_BLOCKED, EMPTY, TOP_BLOCKED, RIGHT_BLOCKED, LEFT_BLOCKED | TOP_BLOCKED, EMPTY, TOP_BLOCKED, EMPTY, EMPTY, RIGHT_BLOCKED,
        BOTTOM_BLOCKED | LEFT_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED, BOTTOM_BLOCKED | RIGHT_BLOCKED
    };
    
    //Board boardMedium(10, 10, Location(2, 7), brdMedium); 
    Board boardMediumTricky(10, 10, Location(3, 9 ), brdMediumTricky); 
    BoardState startingStateMediumTricky;
    startingStateMediumTricky.robots[0] = Location(9, 0);
    startingStateMediumTricky.robots[1] = Location(0, 0);
    startingStateMediumTricky.robots[2] = Location(0, 9);
    RicochetSolver solverMediumTricky(boardMediumTricky);
    
    solverMediumTricky.SolveDepthFirst(5, boardMediumTricky, startingStateMediumTricky, 0, allSolutions);
    
    out << "<h1>Depth First Solver - Medium Tricky</h1>";
    out << "# of solutions: " << allSolutions.size() << "<br>";
    for(int i = 0; i < allSolutions.size(); i++)
    {
        out << "<h3>solution " << i << " turns: " << allSolutions[i].size() << "</h3>";
        printBoardStates(boardMedium, allSolutions[i], out);
        
    }
*/
    
}
