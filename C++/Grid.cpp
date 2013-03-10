#include "stdio.h"
#include "stdlib.h"
#include <string>
#include <vector>

using namespace std;

class Grid
{
    public:
        Grid(char *);
        void setCell(int, int, int);
        int getCell(int, int);
        void printGrid();
        void finishGen();

    private:
        void readFileIn(char*);
        void copyGhostCells();
        

#if VEC == 0
        int **grid;
        int **new_grid;
#else
        vector< vector<int> > grid;
        vector< vector<int> > new_grid;
#endif
        char * FILENAME;
};


Grid::Grid(char * F)
{
    FILENAME = F;
    
#if VEC == 0
	#if MALLOC == 1
		grid = (int **)malloc(sizeof(int *) * DIM + 2);
		new_grid = (int **)malloc(sizeof(int *) * DIM + 2);
		
		for (int i = 0; i < DIM + 2; i++)
		{
			grid[i] = (int *)malloc(sizeof(int *) * DIM + 2);
			new_grid[i] = (int *)malloc(sizeof(int *) * DIM + 2);
		}
	#else
		grid = (int **)calloc(DIM + 2, sizeof(int *));
		new_grid = (int **)calloc(DIM + 2, sizeof(int *));
		
		for (int i = 0; i < DIM + 2; i++)
		{
			grid[i] = (int *)calloc(DIM + 2, sizeof(int *));
			new_grid[i] = (int *)calloc(DIM + 2, sizeof(int *));
		}
	#endif

#else
    
    for (int i = 0; i < DIM + 2; i++)
    {
        vector<int> row; // Create an empty row
        for(int j = 0; j < DIM + 2; j++)
        {
            row.push_back(0); //similar to malloc?! or RATHER closer to calloc!!
        }
        new_grid.push_back(row); // Add the row
        grid.push_back(row);
    }

#endif
    
    readFileIn(FILENAME);
	copyGhostCells();

}

void Grid::finishGen()
{
	//note for loop just as fast as system.arraycopy, clone and copyOf
	
	for(int x = 0; x < DIM+2; x++)
	{
		for(int y = 0; y < DIM+2; y++)
		{
			grid[x][y] = new_grid[x][y];
		}
	}
	
	copyGhostCells();
	//printGrid();
}

void Grid::readFileIn(char *name)
{

	FILE *fp;
	int c, x = 1, y = 1;

	int cell_count = 0;
	int life_count = 0;

	fp = fopen(name, "r");
	if (fp == NULL) 
    {
		printf("File open failed.\n");
		exit(0);
	}

	while ((c = fgetc(fp)) != EOF)
    {
		if (c == 48) 
        {
			grid[x][y] = 0;
			cell_count++;
			y++;
		}
        else if (c == 49)
        {
			grid[x][y] = 1;
			life_count++;
			cell_count++;
			y++;
		}
        else if (c == 10)
        {
			x++;
			y = 1;
		}
	}

	fclose(fp);

}

void Grid::copyGhostCells()
{
	/*copy ghost columns to grid */
    for (int i = 1; i <= DIM; i++)
    {
		grid[i][DIM + 1] = grid[i][1];
		grid[i][0] = grid[i][DIM];
	}
	/*copy ghost rows to grid */
	for (int j = 0; j <= DIM + 1; j++)
    {
		grid[0][j] = grid[DIM][j];
		grid[DIM + 1][j] = grid[1][j];
    }
}

void Grid::setCell(int x, int y, int val)
{
    new_grid[x][y] = val;
}

int Grid::getCell(int x, int y)
{
    return grid[x][y];
}

void Grid::printGrid()
{
	int i, j;

	int cell_count = 0;
	int life_count = 0;

	for (i = 1; i <= DIM; i++) {
		for (j = 1; j <= DIM; j++) {
			if (grid[i][j] == 1)
				life_count++;

			cell_count++;
		}
	}
    printf("Life count: %d\n", life_count);
    printf("Cells: %d\n", cell_count);
}
