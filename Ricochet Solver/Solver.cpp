//
//  Solver.cpp
//  Ricochet Solver
//
//  Created by Stephen Sisk on 5/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "Solver.h"

#include <stack>
#include <boost/functional/hash.hpp>

#define ALG_DEBUG 1

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
    
    void PrintCounts(BoardState &startingState, int winningRobot, ostream &out, Board *board)
    {
 /*       out << "<table><tr><td>Piece Visits</td><td>Delta in Piece Visits</td></tr><tr><td>";
        
        //print out the board counts
        PrintBoardCounts(startingState, out, winningRobot, _visitCounts, board);
        
        out << "</td><td>";
        
        PrintBoardCounts(startingState, out, winningRobot, _visitCountsChange, board);
        
        out << "</td></tr></table>";
        */
   /*
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
    */
    }
    
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




/*
 given a set of state, determines everywhere a given piece could move
 
 returns: a vector of MoveCosts representing the potential boardstates and their cost (cost = number of moves to get there)
 */
void RicochetSolver::ColorBoardForThisPiece(BoardState startingState, int curRobot, vector<MoveCost> &closedMoves)
{
    assert(curRobot < NUM_ROBOTS && curRobot >= 0);
    closedMoves.clear();
    
    //DATA STRUCTURE THOUGHTS
    //closed stack = places that we've already looked at (ie, we've added their items to open stack)
    //open stack = places we haven't looked at yet (they may have moves available from them that
    
    //both open and closed stack have up to date costs for them
    //an opportunity for efficiency is storing both data structures by the location 
    
    vector<MoveCost> openMoves;   

    //this hold the possible moves from a given location - as we move the robot around the board, this will hold the result from WhereCanThisPieceMove, telling us where the robot can go next. 
    vector<Location> possibleMoves;
    
    
    //ALGORITHM
    
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
        //we'll be reusing the startingState struct, so just making sure we move the current robot around every time. The rest of the state will remain the same as we check where this robot can move to.
        startingState.robots[curRobot].x = INVALID_LOC;
        startingState.robots[curRobot].y = INVALID_LOC;
        
        //pull a position off of the open stack
        curMove = openMoves.back();
        openMoves.pop_back(); //I find it lame that pop doesn't return a value
        
        //calculate the cost of all the moves that we'll be taking
        curCost = curMove.cost + 1;
        
        cout << "current position: " << (int) curMove.loc.x << ", " << (int) curMove.loc.y << "\r\n";
        
        //WhereCanThisPieceMove for that position. 
        
        startingState.robots[curRobot] = curMove.loc;
        board->WhereCanThisPieceMove(startingState, curRobot, possibleMoves);
        
        
        
        //look for the possibleMoves in the open stack. if a move isn't in the open stack, push it onto the open stack. If one of the moves is in the open stack, compare cost and update it. 
        for (int i = 0; i < possibleMoves.size(); i++ )
        {
            cout << "possible move " << i << ": " << (int) possibleMoves[i].x << ", " << (int) possibleMoves[i].y << " ";
            bool moveFound = false;
            // OPPORTUNITY FOR OPTIMIZATION - finding the right location quickly here
            for(int j = 0; j < openMoves.size(); j++)
            {
                if(possibleMoves[i] == openMoves[j].loc)
                {
                    moveFound = true;
                    cout << "move found in open\r\n";
                    if(openMoves[j].cost > curCost)
                    {
                        openMoves[j].cost = curCost;
                    }
                }
            }
            
            for(int j = 0; j < closedMoves.size(); j++)
            {
                if(possibleMoves[i] == closedMoves[j].loc)
                {
                    moveFound = true;
                    cout << "moveFound in closed\r\n";
                    if(closedMoves[j].cost > curCost)
                    {
                        closedMoves[j].cost = curCost;
                    }
                }
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

//depth first search impl
//returns the set of solutions found in allSolutions

//flaw with this solution: it will often traverse the same solution space multiple times. it does not scale to larger sizes
void RicochetSolver::SolveDepthFirst(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out
                                     )
{
    cout << "start\r\n";
    assert(allSolutions.size() == 0);
    allSolutions.clear();

    vector<BoardState> stateToInvestigate;
    vector<BoardState> history;
    
    vector<Location> possibleMoves;
    BoardState currentState;
    BoardState newState;
    
    BoardState PopHistory;
    PopHistory.robots[0] = Location::Invalid;
    
    //our starting state is the first thing we investigate!
    stateToInvestigate.push_back(startingState);
    
    int iterationCount = 0; 
    VisitCounter visitCounts;
    Location boardSize = board->GetSize();
    assert(20 > boardSize.x);
    assert(20 > boardSize.y);
    while (stateToInvestigate.size() > 0)
    {
        iterationCount++;
        
        if(iterationCount >= 10000)
        {
            cout << "10k iterations\r\n";
            iterationCount = 0;
            
            visitCounts.PrintCounts(startingState, winningRobot, out, board);            
            
        }
        
        
        //cout <<"iterating. stateToInvestigate: " << stateToInvestigate.size() << ", history: " << history.size() << "\r\n";
        //get the current board state
        currentState = stateToInvestigate.back();
        stateToInvestigate.pop_back();
        
        //are we at the "clear history" item?
        while(currentState.robots[0] == Location::Invalid)
        {
            assert(history.size() > 0);
            history.pop_back();
            if(stateToInvestigate.size() <= 0)
            {
                return; // we are done here! The first item that will be in the stateToInvestigate is a "pop history" marker
            }
            currentState = stateToInvestigate.back();
            stateToInvestigate.pop_back();
        }
        
        //add it to history
        history.push_back(currentState);
        
        //also, push a marker in the investigate stack so that we remember to pop the history list when we hit this again. 
        stateToInvestigate.push_back(PopHistory);
        
        //for each piece on the board...
        for(int r = NUM_ROBOTS - 1; r >= 0; r--) //why backwards? because otherwise the black piece ends up moving the most, not the red piece
        {
            possibleMoves.clear();
            //get the possible moves
            board->WhereCanThisPieceMove(currentState, r, possibleMoves);
            
            //cout << "possible moves: " << possibleMoves.size() << "\r\n";
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
                        //yay! Now, wasn't that worth it? 
                        history.push_back(newState);
                        allSolutions.push_back(history);
                        history.pop_back();
                        cout << "found a solution\r\n";
                        cout << "setting new max depth" << history.size() << "\r\n";
                        if(maxDepth >= history.size())
                        {
                            maxDepth = (int) history.size() - 1; // I do it -1 since I want to solve for a shorter solution than the current
                        }
                        visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);     
                        
                        continue;
                    }
                }
                
                //are we at max depth?
                if (history.size() >= maxDepth) 
                {  
                    //when we are max depth we don't add a move to the investigate list
                    //cout << "at max depth\r\n";
                    continue;
                }
                else
                {
                    
                    //we do these investigations after checking depth since they are relatively expensive and we're trying to avoid doing them if we can. 
                    
                    //we use a reverse iterator since we expect that it's more likely that we'll move back into the same state in recent history rather than older history - for example, moving to a spot, then immediately moving back. 
                    vector<BoardState>::reverse_iterator itr;
                    
                    // iterator to vector element:
                    itr = find (history.rbegin(), history.rend(), newState);
                    
                    //is the move in my history?
                    if (itr != history.rend())
                    {
                        //cout << "already in history\r\n";
                        //don't move there!
                        continue;
                    }
                    
                    itr = find(stateToInvestigate.rbegin(), stateToInvestigate.rend(), newState);
                    
                    //is the move already in state to investigate?
                    if(itr != stateToInvestigate.rend())
                    {
                        //cout << "already in state to investigate\r\n";
                        // don't move there!
                        continue;
                    }
                    //cout << "pushing state\r\n";
                    stateToInvestigate.push_back(newState);
                    visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);     
                    
                }
                
            }     
        }
        
    }
    
}

//depth first search impl
//returns the set of solutions found in allSolutions

//flaw with this solution: it will often traverse the same solution space multiple times. it does not scale to larger sizes

/*
 need to fix the visit count mechanism for this guy.
 
void RicochetSolver::SolveDepthFirst_FavorFewerPieceCounts(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out)
{
    cout << "start\r\n";
    assert(allSolutions.size() == 0);
    allSolutions.clear();
    
    vector<BoardState> stateToInvestigate;
    vector<BoardState> history;
    
    vector<Location> possibleMoves;
    BoardState currentState;
    BoardState newState;
    
    BoardState PopHistory;
    PopHistory.robots[0] = Location::Invalid;
    
    //our starting state is the first thing we investigate!
    stateToInvestigate.push_back(startingState);
    

    int iterationCount = 0; 
    VisitCounter visitCounts;
    Location boardSize = board->GetSize();
    assert(20 > boardSize.x);
    assert(20 > boardSize.y);
    
    vector<Location> possibleMovesSortedByPieceCounts;

    while (stateToInvestigate.size() > 0)
    {
#ifdef ALG_DEBUG
        iterationCount++;
        
        if(iterationCount >= 10000)
        {
            cout << "10k iterations\r\n";
            iterationCount = 0;
            
            visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);
            
            
        }
#endif
        //cout <<"iterating. stateToInvestigate: " << stateToInvestigate.size() << ", history: " << history.size() << "\r\n";
        //get the current board state
        currentState = stateToInvestigate.back();
        stateToInvestigate.pop_back();
        
        //are we at the "clear history" item?
        while(currentState.robots[0] == Location::Invalid)
        {
            assert(history.size() > 0);
            history.pop_back();
            if(stateToInvestigate.size() <= 0)
            {
                return; // we are done here! The first item that will be in the stateToInvestigate is a "pop history" marker
            }
            currentState = stateToInvestigate.back();
            stateToInvestigate.pop_back();
        }
        
        //add it to history
        history.push_back(currentState);
        
        //also, push a marker in the investigate stack so that we remember to pop the history list when we hit this again. 
        stateToInvestigate.push_back(PopHistory);
        
        //for each piece on the board...
        for(int r = NUM_ROBOTS - 1; r >= 0; r--) //why backwards? because otherwise the black piece ends up moving the most, not the red piece
        {
            int rToIncrement = 0;
            if(r > 1) 
            {
                rToIncrement = 1;
            }
            
            possibleMoves.clear();
            possibleMovesSortedByPieceCounts.clear();
            //get the possible moves
            board->WhereCanThisPieceMove(currentState, r, possibleMoves);
            
            //this is ugly.
            while(possibleMoves.size() > 0)
            {
                
                int smallest = 0;
                for(int m = 1; m < possibleMoves.size(); m++)
                {
               //     cout << "possible move " << m << ": " visitCounts[possibleMoves[m].x][possibleMoves[m].y][r] << "\r\n";
                    if(visitCounts[possibleMoves[m].x][possibleMoves[m].y][rToIncrement] < visitCounts[possibleMoves[smallest].x][possibleMoves[smallest].y][rToIncrement])
                    {
                        smallest = m;
                    }
                }
                possibleMovesSortedByPieceCounts.push_back(possibleMoves[smallest]);
                possibleMoves.erase(possibleMoves.begin() + smallest);
           //     cout << "possible move chosen " << ": " << (int) possibleMovesSortedByPieceCounts.back().x << "," << (int) possibleMovesSortedByPieceCounts.back().y << "\r\n";
            }
            
            
            
            //cout << "possible moves: " << possibleMoves.size() << "\r\n";
            //for each of the possible moves...
            for(int m = 0; m < possibleMovesSortedByPieceCounts.size(); m++)
            {
                //construct the new boardstate
                newState = currentState;
                newState.robots[r] = possibleMovesSortedByPieceCounts[m];
                
                //is the move a victory move?
                //you can't win if the current robot isn't the robot you're trying to move to the target
                if(r == winningRobot)
                {
                    if(possibleMovesSortedByPieceCounts[m] == board->GetTarget())
                    {
                        //yay! Now, wasn't that worth it? 
                        history.push_back(newState);
                        allSolutions.push_back(history);
                        history.pop_back();
                        cout << "found a solution\r\n";
                        cout << "setting new max depth" << history.size() << "\r\n";
                        if(maxDepth >= history.size())
                        {
                            maxDepth = (int) history.size() - 1; // I do it -1 since I want to solve for a shorter solution than the current
                        }

                        
                        
                        visitCounts[possibleMovesSortedByPieceCounts[m].x][possibleMovesSortedByPieceCounts[m].y][rToIncrement]++;
                        
                        
#ifdef ALG_DEBUG
                        visitCountsChange[possibleMovesSortedByPieceCounts[m].x][possibleMovesSortedByPieceCounts[m].y][rToIncrement]++;
#endif
                        
                        continue;
                    }
                }
                
                //are we at max depth?
                if (history.size() >= maxDepth) 
                {  
                    //when we are max depth we don't add a move to the investigate list
                    //cout << "at max depth\r\n";
                    continue;
                }
                else
                {
                    
                    //we do these investigations after checking depth since they are relatively expensive and we're trying to avoid doing them if we can. 
                    
                    //we use a reverse iterator since we expect that it's more likely that we'll move back into the same state in recent history rather than older history - for example, moving to a spot, then immediately moving back. 
                    vector<BoardState>::reverse_iterator itr;
                    
                    // iterator to vector element:
                    itr = find (history.rbegin(), history.rend(), newState);
                    
                    //is the move in my history?
                    if (itr != history.rend())
                    {
                        //cout << "already in history\r\n";
                        //don't move there!
                        continue;
                    }
                    
                    itr = find(stateToInvestigate.rbegin(), stateToInvestigate.rend(), newState);
                    
                    //is the move already in state to investigate?
                    if(itr != stateToInvestigate.rend())
                    {
                        //cout << "already in state to investigate\r\n";
                        // don't move there!
                        continue;
                    }
                    //cout << "pushing state\r\n";
                    stateToInvestigate.push_back(newState);
                    

                    visitCounts[possibleMovesSortedByPieceCounts[m].x][possibleMovesSortedByPieceCounts[m].y][rToIncrement]++;
                    
                    
#ifdef ALG_DEBUG
                    visitCountsChange[possibleMovesSortedByPieceCounts[m].x][possibleMovesSortedByPieceCounts[m].y][rToIncrement]++;
#endif

                    
                }
                
            }     
        }
        
    }
    
}

*/


void RicochetSolver::DebugSuggestedMoves::Print(ostream &out, RicochetSolver *solver, BoardState startingState, int color)
{
    out << "<td>";
    solver->board->PrintMoves(startingState, _possibleMoves, out, color);
    out << "</td><td>";
    solver->board->PrintMoves(startingState, _actualMoves, out, color);
    out << "</td>";
}

void RicochetSolver::DebugSuggestedMoves::AddPossibleMoves(vector<Location> &possibleMoves)
{
    _possibleMoves.insert(_possibleMoves.end(), possibleMoves.begin(), possibleMoves.end());
    _actualMoves.insert(_actualMoves.end(), possibleMoves.begin(), possibleMoves.end()); 
}

void RicochetSolver::DebugSuggestedMoves::RemoveMove(Location &move)
{
    vector<Location>::iterator itr = find(_actualMoves.begin(), _actualMoves.end(), move);
    
    assert(itr != _actualMoves.end());
    _actualMoves.erase(itr);
}

void RicochetSolver::DebugSuggestedMoves::Clear()
{
    _possibleMoves.clear();
    _actualMoves.clear(); 
}


//depth first search impl
//returns the set of solutions found in allSolutions

//flaw with this solution: it will often traverse the same solution space multiple times. it does not scale to larger sizes
void RicochetSolver::SolveDepthFirst_withHashedHistory(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out)
{
    cout << "start\r\n";
    assert(allSolutions.size() == 0);
    allSolutions.clear();
    
    vector<BoardState> stateToInvestigate;
    vector<BoardState> history;
    
    vector<Location> possibleMoves;
    BoardState currentState;
    BoardState newState;
    
    BoardState PopHistory;
    PopHistory.robots[0] = Location::Invalid;
    
    //our starting state is the first thing we investigate!
    stateToInvestigate.push_back(startingState);
    VisitCounter visitCounts;
    int iterationCount = 0; 

    Location boardSize = board->GetSize();
    assert(20 > boardSize.x);
    assert(20 > boardSize.y);
    
    AllPastStatesInvestigatedFilter statesFilter;
    //DebugSuggestedMoves debugSuggestedMoves;
    
    while (stateToInvestigate.size() > 0)
    {
        iterationCount++;
        
        if(iterationCount >= 10000)
        {
            cout << "10k iterations. hashtable size: " << statesFilter.size() << "\r\n";
            
            iterationCount = 0;
            
            visitCounts.PrintCounts(startingState, winningRobot, out, board);
        }

       // cout <<"iterating. stateToInvestigate: " << stateToInvestigate.size() << ", history: " << history.size() << ", hash table: " <<  statesFilter.size() << "\r\n";
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
        
       // out << "<table><tr>";
        
        //for each piece on the board...
        for(int r = 0; r < NUM_ROBOTS; r++) 
        {       
            //debugSuggestedMoves.Clear();
            possibleMoves.clear();
            //get the possible moves
            board->WhereCanThisPieceMove(currentState, r, possibleMoves);
            //debugSuggestedMoves.AddPossibleMoves(possibleMoves);
            
            //cout << "possible moves: " << possibleMoves.size() << "\r\n";
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
                        //yay! Now, wasn't that worth it? 
                        history.push_back(newState);
                        allSolutions.push_back(history);
                        history.pop_back();
                        cout << "found a solution\r\n";
                        cout << "setting new max depth with -1: " << history.size() << "\r\n";
                        if(maxDepth >= history.size())
                        {
                            maxDepth = (int) history.size() - 1; // I do it -1 since I want to solve for a shorter solution than the current
                        }
                        
                        visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);

                        
                        continue;
                    }
                }
                
                //are we at max depth?
                if (history.size() >= maxDepth) 
                {  
                    //when we are max depth we don't add a move to the investigate list
                    //cout << "at max depth\r\n";
                    //debugSuggestedMoves.RemoveMove(possibleMoves[m]);
                    continue;
                }
                else
                {
                    //is the move already in state to investigate?
                    if(statesFilter.AllowState(newState, history.size()))
                    {
                        //cout << "pushing state\r\n";
                        stateToInvestigate.push_back(newState);
                        
                        visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);
                        
                    }
                    else
                    {
                        //debugSuggestedMoves.RemoveMove(possibleMoves[m]);
                    }
                }
                
                
            } 
           //debugSuggestedMoves.Print(out, this, currentState, r); 
        } //end of robot loop
       // out << "</tr></table>";
        
        
        
    }
    
}


/*
//depth first search impl
//returns the set of solutions found in allSolutions

//flaw with this solution: it will often traverse the same solution space multiple times. it does not scale to larger sizes
void RicochetSolver::SolveDepthFirst_BlackMovesFirst(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out)
{
    cout << "start\r\n";
    assert(allSolutions.size() == 0);
    allSolutions.clear();
    
    vector<BoardState> stateToInvestigate;
    vector<BoardState> history;
    
    vector<Location> possibleMoves;
    BoardState currentState;
    BoardState newState;
    
    BoardState PopHistory;
    PopHistory.robots[0] = Location::Invalid;
    
    //our starting state is the first thing we investigate!
    stateToInvestigate.push_back(startingState);
    
#ifdef ALG_DEBUG
    int iterationCount = 0; 
    VisitCounter visitCounts;
    Location boardSize = board->GetSize();
    assert(20 > boardSize.x);
    assert(20 > boardSize.y);
#endif
    while (stateToInvestigate.size() > 0)
    {
#ifdef ALG_DEBUG 
        
        iterationCount++;
        
        if(iterationCount >= 1000)
        {
            iterationCount = 0;
            
            visitCounts.PrintCounts(startingState, winningRobot, out, board);
            
            
        }
#endif
        
        //cout <<"iterating. stateToInvestigate: " << stateToInvestigate.size() << ", history: " << history.size() << "\r\n";
        //get the current board state
        currentState = stateToInvestigate.back();
        stateToInvestigate.pop_back();
        
        //are we at the "clear history" item?
        while(currentState.robots[0] == Location::Invalid)
        {
            assert(history.size() > 0);
            history.pop_back();
            if(stateToInvestigate.size() <= 0)
            {
                return; // we are done here! The first item that will be in the stateToInvestigate is a "pop history" marker
            }
            currentState = stateToInvestigate.back();
            stateToInvestigate.pop_back();
        }
        
        //add it to history
        history.push_back(currentState);
        
        //also, push a marker in the investigate stack so that we remember to pop the history list when we hit this again. 
        stateToInvestigate.push_back(PopHistory);
        
        //for each piece on the board...
        for(int r = 0; r < NUM_ROBOTS; r++) //why backwards? because otherwise the black piece ends up moving the most, not the red piece
        {
            possibleMoves.clear();
            //get the possible moves
            board->WhereCanThisPieceMove(currentState, r, possibleMoves);
            
            //cout << "possible moves: " << possibleMoves.size() << "\r\n";
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
                        //yay! Now, wasn't that worth it? 
                        history.push_back(newState);
                        allSolutions.push_back(history);
                        history.pop_back();
                        cout << "found a solution\r\n";
                        cout << "setting new max depth\r\n";
                        if(maxDepth >= history.size())
                        {
                            maxDepth = (int) history.size() - 1; // I do it -1 since I want to solve for a shorter solution than the current
                        }
                        visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);
                        continue;
                    }
                }
                
                //are we at max depth?
                if (history.size() >= maxDepth) 
                {  
                    //when we are max depth we don't add a move to the investigate list
                    //cout << "at max depth\r\n";
                    continue;
                }
                else
                {
                    
                    //we do these investigations after checking depth since they are relatively expensive and we're trying to avoid doing them if we can. 
                    
                    //we use a reverse iterator since we expect that it's more likely that we'll move back into the same state in recent history rather than older history - for example, moving to a spot, then immediately moving back. 
                    vector<BoardState>::reverse_iterator itr;
                    
                    // iterator to vector element:
                    itr = find (history.rbegin(), history.rend(), newState);
                    
                    //is the move in my history?
                    if (itr != history.rend())
                    {
                        //cout << "already in history\r\n";
                        //don't move there!
                        continue;
                    }
                    
                    itr = find(stateToInvestigate.rbegin(), stateToInvestigate.rend(), newState);
                    
                    //is the move already in state to investigate?
                    if(itr != stateToInvestigate.rend())
                    {
                        //cout << "already in state to investigate\r\n";
                        // don't move there!
                        continue;
                    }
                    //cout << "pushing state\r\n";
                    stateToInvestigate.push_back(newState);
                    visitCounts.IncrementCounts(possibleMoves[m].x, possibleMoves[m].y, r);

                    
                }
                
            }     
        }
        
    }
    
}
*/

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
