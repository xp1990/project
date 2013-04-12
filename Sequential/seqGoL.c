#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

#define MAXGEN 100
#define DIM 1024
#define LIFE 3
#define SEED 2012
#define USEBOOL 1
#define MALLOC 0

#if USEBOOL == 1
    bool **grid;
    bool **new_grid;
#else
    int **grid;
    int **new_grid;
#endif

struct timespec begin, end;
int cell_count, life_count;

void init();

#if USEBOOL == 1
    void printGrid(bool **);
    void fillRand(bool **);
    void process(bool **, bool **);
    void copyGhostCells(bool **);
    int getCount(bool **grid_ptr, int i, int j);
    void applyRule(int , int , int , bool** , bool **);
#else
    void printGrid(int **);
    void fillRand(int **);
    void process(int **, int **);
    void copyGhostCells(int **)
    int getCount(int **grid_ptr, int i, int j);
#endif

int main(int argc, char *argv[])
{
    #if USEBOOL == 1
	bool **grid_ptr, **new_grid_ptr, **temp_ptr;
    #else
    int **grid_ptr, **new_grid_ptr, **temp_ptr;
    #endif

	/*vars for timing the main game loop */
	double time_spent;

	/*main game vars */
	int i, j, gen;

  init();

	/*assign arrays to pointers */
	grid_ptr = grid;
	new_grid_ptr = new_grid;

	/*used if user wants to fill grid using rand */
	fillRand(grid_ptr);

	printGrid(grid_ptr);
	printf("\nCells: %d\nAlive: %d\n", cell_count, life_count);

	/*begin timing */
	clock_gettime(CLOCK_MONOTONIC, &begin);
    
    for (gen = 0; gen < MAXGEN; gen++)
    {   
        
        copyGhostCells(grid_ptr);

        process(grid_ptr, new_grid_ptr);

        temp_ptr = grid_ptr;
        grid_ptr = new_grid_ptr;
        new_grid_ptr = temp_ptr;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    time_spent = (end.tv_sec - begin.tv_sec);
    time_spent = time_spent + (end.tv_nsec - begin.tv_nsec) / 1000000000.0;

    printGrid(grid_ptr);
    printf("\nCells: %d\nAlive: %d\n", cell_count, life_count);

    printf("\nTime taken %f\n", time_spent);

    return (0);
}


void init()
{	
    /*in c a 2d grid is an array of pointers to pointers of ints
	 *here we allocate enough space for our temp transfer grid and
	 *our original grid
	 */
     
    int i;
     
    #if USEBOOL == 0
        #if MALLOC == 1
        
            grid = (int **)malloc(sizeof(int *) * DIM + 2);
            new_grid = (int **)malloc(sizeof(int *) * DIM + 2);
            
            for (i = 0; i < DIM + 2; i++)
            {
                grid[i] = (int *)malloc(sizeof(int *) * DIM + 2);
                new_grid[i] = (int *)malloc(sizeof(int *) * DIM + 2);
            }
            
        #else
        
            grid = (int **)calloc(DIM + 2, sizeof(int *));
            new_grid = (int **)calloc(DIM + 2, sizeof(int *));
            
            for (i = 0; i < DIM + 2; i++)
            {
                grid[i] = (int *)calloc(DIM + 2, sizeof(int *));
                new_grid[i] = (int *)calloc(DIM + 2, sizeof(int *));
            }
            
        #endif
    #else
        grid = (bool **)calloc(DIM + 2, sizeof(bool *));
        new_grid = (bool **)calloc(DIM + 2, sizeof(bool *));
        
        for (i = 0; i < DIM + 2; i++)
        {
            grid[i] = (bool *)calloc(DIM + 2, sizeof(bool *));
            new_grid[i] = (bool *)calloc(DIM + 2, sizeof(bool *));
        }
    #endif
    
}

#if USEBOOL == 1
void copyGhostCells(bool **grid_ptr)
#else
void copyGhostCells(int **grid_ptr)
#endif
{
    
    int i,j;
    /*copy ghost columns to grid */
    for (i = 1; i <= DIM; i++)
    {
        grid_ptr[i][DIM + 1] = grid_ptr[i][1];
        grid_ptr[i][0] = grid_ptr[i][DIM];
    }
    /*copy ghost rows to grid */
    for (j = 0; j <= DIM + 1; j++)
    {
        grid_ptr[0][j] = grid_ptr[DIM][j];
        grid_ptr[DIM + 1][j] = grid_ptr[1][j];
    }
}

#if USEBOOL == 1
void fillRand(bool **grid_ptr)
#else
void fillRand(int **grid_ptr)
#endif
{
	int i, j;

	/*random seed for life generation
	 *incorporate time as a variable for more
	 *randomness
	 */
	srand(SEED);

	cell_count = 0;
	life_count = 0;

	for (i = 1; i <= DIM; i++) {
		for (j = 1; j <= DIM; j++) {
			/*
			 *uses the LIFE var as a probability of life existing in
			 *the initial grid.
			 */
			if (rand() % LIFE == 1) {
				life_count++;
				grid_ptr[i][j] = 1;
			} else {
				grid_ptr[i][j] = 0;
			}
			cell_count++;
		}
	}
}

#if USEBOOL == 1
void printGrid(bool **g)
#else
void printGrid(int **g)
#endif
{
	int i, j;

	cell_count = 0;
	life_count = 0;

	for (i = 1; i <= DIM; i++) {
		for (j = 1; j <= DIM; j++) {
			if (g[i][j] == 1)
				life_count++;

			cell_count++;
			//printf("%d", g[i][j]);
		}
		//printf("\n");
	}

}

#if USEBOOL == 1
void process(bool **grid_ptr, bool **new_grid_ptr)
#else
void process(int **grid_ptr, int **new_grid_ptr)
#endif
{

	int i, j, count;

	for (i = 1; i <= DIM; i++)
    {
		for (j = 1; j <= DIM; j++)
        {
			count = getCount(grid_ptr, i, j);
      applyRule(count, i, j, grid_ptr, new_grid_ptr);
		}
	}
}

void applyRule(int count, int i, int j, bool** grid_ptr, bool **new_grid_ptr)
{
  if (count == 3 || (count == 2 && grid_ptr[i][j] == 1)) {
    new_grid_ptr[i][j] = 1;
  } else if (count < 2 || count > 3) {
    new_grid_ptr[i][j] = 0;
  } else if (count == 2) {
    new_grid_ptr[i][j] = grid_ptr[i][j];
  }
}

//Adding this function made the program run 4 times slower! Using Gprof!
#if USEBOOL == 1
int getCount(bool **grid_ptr, int i, int j)
#else
int getCount(int **grid_ptr, int i, int j)
#endif
{
    return grid_ptr[i - 1][j - 1] + grid_ptr[i - 1][j] +
			    grid_ptr[i - 1][j + 1] + grid_ptr[i][j - 1] +
			    grid_ptr[i][j + 1] + grid_ptr[i + 1][j - 1] +
			    grid_ptr[i + 1][j] + grid_ptr[i + 1][j + 1];
}
