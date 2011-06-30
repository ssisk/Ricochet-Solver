//
//  main.cpp
//  Ricochet Solver
//
//  Created by Stephen Sisk on 4/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


#include "board.h"
#include "solver.h"

int main (int argc, const char * argv[])
{
    ofstream fout("./rico_out.html", ios::out);
    
    fout << "<html><body>";
    
    //Run various unit tests. Not necessary if you just want to solve a board
    //all of these will output their results to html
    Board::TestGet(fout);
    Board::TestPrintBoard(fout);
    Board::TestDetermineEdges();
    Board::TestWhereCanThisPieceMove(fout);
    RicochetSolver::TestColorBoardForThisPiece(fout);

    //for now, this is how you solve a board - modify the code in TestSolveDepthFirst to change the board you'd like to solve
    RicochetSolver::TestSolveDepthFirst(fout);
    
    fout << "</body></html>";

    
    return 0;
     
    
}


