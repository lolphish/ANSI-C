/*
 * ICS 53: Principles of System Design
 * Two hour lab project: Create Conway's Game of Life 
 * within lab time period. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
int column = 32;
int row = 120;
int new_board[32][120];


/*
 * Print the board with trailing spaces in order to simulate
 * animation. Optimized for a full screen terminal.
 */
void print_board(int board[column][row])
{
	for (unsigned int i = 0; i < column; ++i)
	{
		for (unsigned int j = 0; j < row; ++j)
		{
			if( board[i][j] == 1)
				printf("\033[36m%d ", 0);
				// printf("%d ", board[i][j]);
			else
				printf(" ");
				// printf("\033[36m%d ", board[i][j]);
				
		}
		printf("\n");
	}

	for(  int h =0; h < 30; ++h)
		printf("\n");
}

/*
 * Rules (From Wikipedia):
 * Any live cell with fewer than two live neighbours dies, as if caused by under-population.
 * Any live cell with two or three live neighbours lives on to the next generation.
 * Any live cell with more than three live neighbours dies, as if by over-population.
 * Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.	
 */
void evolve(int board[column][row])
{
	for (int i = 0; i < column; ++i)
	{
		for(int j = 0; j< row; ++j)
		{
			unsigned int total = 0;
			total += (board[ (i-1 < 0) ? column : i-1][ (j-1 < 0) ? row : j-1] == 1 ? 1 : 0);  // if the left side
			total += (board[ (i-1 < 0)? column : i -1][j] == 1 ? 1 : 0);
			total += (board[ (i-1 < 0) ? column : i -1][(j+1 > row ? 0 : j+1)] == 1 ? 1 : 0);
			total += (board[i][(j-1 < 0) ? row : j -1] == 1 ? 1 : 0);                          // check the top and bottom
			total += (board[i][(j+1 > row) ? 0 : j +1] == 1 ? 1 : 0);
			total +=  (board[ (i+1 > column) ? 0 : i +1][(j-1 < 0) ? row : j-1] == 1 ? 1 : 0); // check the right side
			total += (board[ (i+1 > column) ? 0 : i + 1][j] == 1 ? 1 : 0);
			total += (board[ (i+1 > column) ? 0 : i+ 1][(j+1 > row ? 0 : j+1)] == 1 ? 1 : 0); 

			if (board[i][j] == 1 ) 
				new_board[i][j] = (total < 2 ) ? 0 : (total > 3 ? 0 : 1); 	
			else
				new_board[i][j] = (total == 3) ? 1 : 0;
		}
	}
}

/*
 * Initialize the board with randomly placed cells across the board.
 * Then runs the game until the user exits by a signal command.
 */
void make_game()
{
	int board[column][row];
	for (int i = 0; i < row; ++i)
	{
		for (int j = 0; j < column; ++j)
			board[j][i] = rand() < RAND_MAX / 10 ? 1 : 0;
	}
	while (1){
		print_board(board);
		evolve(board);

		for (int i =0; i < column; ++i){
			for (int j=0; j < row; ++j)
				board[i][j] = new_board[i][j];
		}
		sleep (1);
	}
}

int main(int argc, char * argv[])
{
	make_game();
	return 0;
}

