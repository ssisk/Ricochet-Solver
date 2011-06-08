//
//  main.cpp
//  Ricochet Solver
//
//  Created by Stephen Sisk on 4/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


#include "board.h"
#include "solver.h"

#include <boost/unordered_set.hpp>

int main (int argc, const char * argv[])
{
    
    
    ofstream fout("./rico_out.html", ios::out);
    
    fout << "<html><body>";
    
  /*  
    boost::unordered_set<int> table;
    
    table.insert(1);
    table.insert(2);
    table.insert(3);
    
    table.insert(47);
    table.insert(38);
    table.insert(99);
    
    for(int i = 0; i < 100; i++)
    {
         //table.insert(math.random() 
    }
    
    for(int i = 0; i < 300; i++)
    {
        if(table.find(i) != table.end())
        {
            cout << "found " << i << "\r\n";
        }
    }
    */
    
    /*
    Board::TestGet(fout);
    Board::TestPrintBoard(fout);
    Board::TestDetermineEdges();
    Board::TestWhereCanThisPieceMove(fout);
    RicochetSolver::TestColorBoardForThisPiece(fout);
     */
    
    
    
    RicochetSolver::TestSolveDepthFirst(fout);
    
    
    
    fout << "</body></html>";

    
    return 0;
     
    
}


