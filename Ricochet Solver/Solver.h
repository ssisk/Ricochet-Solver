//
//  Solver.h
//  Ricochet Solver
//
//  Created by Stephen Sisk on 5/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "board.h"
#include <boost/unordered_map.hpp>

class RicochetSolver
{
public:
    RicochetSolver(Board &board);

    void SolveDepthFirst_withHashedHistory(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out);
    
    static void TestAllPastStatesInvestigatedFilter(ostream &out);
    static void TestColorBoardForThisPiece(ostream &out);
    static void TestSolveDepthFirst(ostream &out);
    
private:
    Board *board;
    struct MoveCost
    {
        Location loc; 
        short cost;
    };
    void PrintMoveCosts(vector<RicochetSolver::MoveCost> &MoveCosts, BoardState &state, ostream &out, int curRobot);
    void PrintMoves(vector<Location> &Moves, BoardState &state, ostream & out, int curRobot);
    
    void UpdateMoveInStack(vector<MoveCost> &stackToCheck, Location possibleMove, bool &moveFound, int curCost);
    void ColorBoardForThisPiece(BoardState startingState, int curRobot, vector<MoveCost> &ret);
    
    class AllPastStatesInvestigatedFilter
    {
    public: 
        AllPastStatesInvestigatedFilter()
        {
            //set up the hashtable
            
            //nothing for now...
        }
        
        //checks to see whether or not a given boardstate has already been looked at and whether or not the boardstate was arrived at from a shorter distance. Also updates the distance to that boardstate if the new distance is shorter.
        //returns: false if the boardstate has already been visited at a shorter distance. true if the boardstate has not been visited or if it was visited at a larger distance.
        
        bool AllowState(BoardState &potentialMove, short currentDepth)
        {
            bool ret = false;
            
            boost::unordered_map<BoardState, short>::iterator itr= _mapBoardStateToShort.find(potentialMove);
            
            if(itr == _mapBoardStateToShort.end() || itr->second > currentDepth)
            {
                ret = true;
                _mapBoardStateToShort[potentialMove] = currentDepth;
            }
            
            
            return ret;
            
        }
        
        size_t size()
        {
            return _mapBoardStateToShort.bucket_count();
           
        }
        
    private:
                     
        boost::unordered_map<BoardState, short> _mapBoardStateToShort;
    };
    
    // The purpose of this class is to help debug the process whereby the search algorithm takes a set of potential moves and whittles it down to a set of moves it's actually going to do. So it allows you to add a set of moves the algorithm is considering (AddPossibleMoves), then slowly remove them as the algorithm decides not to use them(RemoveMove) and finally output the result to the screen (Print)
    
    // Using this class will slow down the solve process considerably, so it's suggested that it only be used for debugging small boards. In that scenario, it can be highly useful.
    
    class DebugSuggestedMoves
    {
    public:
        // call these functions at the beginning and end of investigating a given boardstate - doing so will ensure that the items for the boardstate all go into one line. 
        // This functionality is currently brittle - I would probably have the class write into a private buffer and output to fout once at EndBoardState() in the future. 
        void StartBoardState(ostream &out); 
        void EndBoardState(ostream &out);
        
        void Print(ostream &out, RicochetSolver *solver, BoardState startingState, int color);
        void AddPossibleMoves(vector<Location> &possibleMoves);
        void RemoveMove(Location &move);
        void Clear();
        

        
    private:
        vector<Location> _possibleMoves;
        vector<Location> _actualMoves;
        
    };

};