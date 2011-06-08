//
//  Solver.h
//  Ricochet Solver
//
//  Created by Stephen Sisk on 5/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "board.h"
#include <boost/unordered_set.hpp>

class RicochetSolver
{
public:
    RicochetSolver(Board &board);
    void SolveDepthFirst(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out);

    void SolveDepthFirst_BlackMovesFirst(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out);
    
    void SolveDepthFirst_FavorFewerPieceCounts(int maxDepth, BoardState startingState, int winningRobot, vector<vector<BoardState> > &allSolutions, ostream &out);
    
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
    void ColorBoardForThisPiece(BoardState startingState, int curRobot, vector<MoveCost> &ret);
    
    class AllPastStatesInvestigatedFilter
    {
    public: 
        AllPastStatesInvestigatedFilter()
        {
            //set up the hashtable
            
            //nothing for now...
        }
        
        //checks to see if the state is in the history, and if it is, returns 
        bool AllowState(BoardState &potentialMove)
        {
            bool ret = false;
            
            boost::unordered_set<BoardState>::iterator itr= _tableBoardState.find(potentialMove);
            
            
            
            if(itr == _tableBoardState.end())
            {
                ret = true;
                _tableBoardState.insert(potentialMove);
            }
            else
            {
                assert(*itr == potentialMove);
            }
            
            return ret;
            
        }
        
        size_t size()
        {
            return _tableBoardState.bucket_count();
        }
        
    private:
        boost::unordered_set<BoardState> _tableBoardState;
    };
    
    class DebugSuggestedMoves
    {
    public:
        void Print(ostream &out, RicochetSolver *solver, BoardState startingState, int color);
        void AddPossibleMoves(vector<Location> &possibleMoves);
        void RemoveMove(Location &move);
        void Clear();
        
    private:
        vector<Location> _possibleMoves;
        vector<Location> _actualMoves;
        
    };

};