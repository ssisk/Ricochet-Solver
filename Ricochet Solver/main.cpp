//
//  main.cpp
//  Ricochet Solver
//
//  Created by Stephen Sisk on 4/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "board.h"



/*
 
 
 void CopyBoard(BoardLocType from[][size.y], BoardLocType to[][size.y])
 {
 size_t x = sizeof(BoardLocType[size.x][BOARD_Y]);
 
 memcpy(to, from, x);
 }
 
 */




int main (int argc, const char * argv[])
{
    
    ofstream fout("./rico_out.html", ios::out);
    
    fout << "<html><body>";
    
    Board::TestGet(fout);
    Board::TestPrintBoard(fout);
    Board::TestDetermineEdges();
    Board::TestWhereCanThisPieceMove(fout);
    
    fout << "</body></html>";

    
    return 0;
}


