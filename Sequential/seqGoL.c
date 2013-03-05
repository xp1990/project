#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define MAXGEN 1000
#define DIM 512
#define LIFE 3
#define SEED 2012

struct node
{
    node * next;
    bool val;
};

node * head = NULL;

int **grid;
int **new_grid;

struct timespec begin, end;
int cell_count, life_count;

void print_grid(int **);
void fill_rand(int **);
void process(int **, int **);

int main(int argc, char *argv[])
{
	int **grid_ptr, **new_grid_ptr, **temp_ptr;


	/*vars for timing the main game loop */
	double time_spent;

	/*main game vars */
	int i, j, gen;

	/*in c a 2d grid is an array of pointers to pointers of ints
	 *here we allocate enough space for our temp transfer grid and
	 *our original grid
	 */
	grid = (int **)malloc(sizeof(int *) * DIM + 2);
	new_grid = (int **)malloc(sizeof(int *) * DIM + 2);
	
    for (i = 0; i < DIM + 2; i++)
    {
		grid[i] = (int *)malloc(sizeof(int *) * DIM + 2);
		new_grid[i] = (int *)malloc(sizeof(int *) * DIM + 2);
	}

	/*assign arrays to pointers */
	grid_ptr = grid;
	new_grid_ptr = new_grid;

	/*used if user wants to fill grid using rand */
	fill_rand(grid_ptr);

	print_grid(grid_ptr);
	printf("\nCells: %d\nAlive: %d\n", cell_count, life_count);

	/*begin timing */
	clock_gettime(CLOCK_MONOTONIC, &begin);
    
    for (gen = 0; gen < MAXGEN; gen++)
    {   
            
        /**
        *There is another way of doing this using nested loops.
        *and if statements however this way is FAR more efficient!
        **/

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

        process(grid_ptr, new_grid_ptr);


        if (gen != 0)	//why?
        {
            temp_ptr = grid_ptr;
            grid_ptr = new_grid_ptr;
            new_grid_ptr = temp_ptr;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    time_spent = (end.tv_sec - begin.tv_sec);
    time_spent = time_spent + (end.tv_nsec - begin.tv_nsec) / 1000000000.0;

    print_grid(grid_ptr);
    printf("\nCells: %d\nAlive: %d\n", cell_count, life_count);

    printf("\nTime taken %f\n", time_spent);

    return (0);
}

void fill_rand(int **grid_ptr)
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

void print_grid(int **g)
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

void process(int **grid_ptr, int **new_grid_ptr)
{

	/*this can be made more efficient! produce two versions using compiler flags.
	 *second version will break loop after a condition is met... 
	 */

	int i, j, count;

	for (i = 1; i <= DIM; i++)
    {
		for (j = 1; j <= DIM; j++)
        {
			count =
			    grid_ptr[i - 1][j - 1] + grid_ptr[i - 1][j] +
			    grid_ptr[i - 1][j + 1] + grid_ptr[i][j - 1] +
			    grid_ptr[i][j + 1] + grid_ptr[i + 1][j - 1] +
			    grid_ptr[i + 1][j] + grid_ptr[i + 1][j + 1];

			//printf("%d",count);

			if (count == 3 || (count == 2 && grid_ptr[i][j] == 1)) {
				new_grid_ptr[i][j] = 1;

				//printf("Thread %d creating life at %d,%d\n", tid, i, j);
			} else if (count < 2 || count > 3) {
				new_grid_ptr[i][j] = 0;
				//printf("Thread %d: destroying life at %d,%d\n", tid, i, j);
			} else if (count == 2) {
				new_grid_ptr[i][j] = grid_ptr[i][j];
			}
		}
	}
}
