#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>


#define MAXGEN 10000 //number of generations to process
#define DIM 2048 //dimension of the grid
#define LIFE 3 //probability of life if random generation is used
#define BOOL 1 //use bool for arrays, if not use int.
#define SEED 2012 //seed for random life generation
#define THREADS 4 //number of threads to create for parallel region
#define SHARED 1 //shared or local memory implementation
#define INLINE 1 //use inline functions 
#define REGISTER 0 //instruct compiler to store global state arrays in registers
#define FILENAME "2048.dat" //filename to read from

//var for time measurement
struct timespec begin, end; 
int cell_count, life_count;
double time_spent;

//var for holding threads
unsigned int nthreads;

/*
 * Conditional compilation declaration of function prototypes
 */

#if BOOL == 1
void readFile (char *, bool **);
void printGrid (bool **);
void fillRand (int **);
#else
void readFile (char *, int **);
void printGrid (int **);
void fillRand (int **);
#endif



#if INLINE == 1 && BOOL == 1
inline void copyGhostCells(bool**);
inline void process (bool **, bool **, int, int, int);
inline void privMemCopy (int, int, bool **, bool **);
#elif INLINE == 0 && BOOL == 1
void copyGhostCells(bool**);
void process (bool **, bool **, int, int, int);
void privMemCopy (int, int, bool **, bool **);
#elif INLINE == 0 && BOOL == 0
void copyGhostCells(int**);
void process (int **, int **, int, int, int);
void privMemCopy (int, int, int **, int **);
#elif INLINE == 1 && BOOL == 0
inline void copyGhostCells(int**);
inline void process (int **, int **, int, int, int);
inline void privMemCopy (int, int, int **, int **);
#endif

#if REGISTER == 1 && BOOL == 0
register int **grid;
register int **new_grid;
#elif BOOL == 0
int **grid;
int **new_grid;
#elif BOOL == 1
bool **grid;
bool **new_grid;
#endif

int
main (int argc, char *argv[])
{
    
    #if BOOL == 1
    bool **grid_ptr, **new_grid_ptr, **temp_ptr;
    #else
    int **grid_ptr, **new_grid_ptr, **temp_ptr;
    #endif
    
    /*begin timing */
    clock_gettime (CLOCK_MONOTONIC, &begin);

    /*Assign global var to num_threads
    *using the macro as a parameter causes
    *a floating point exception*/
    nthreads = THREADS;

    /*vars for timing the main game loop */

    /*main game vars */
    int i, j, gen;

    /* Allocate the memory for the global state arrays */
    #if BOOL == 1
    grid = (bool **) calloc (DIM + 2, sizeof (bool *));
    new_grid = (bool **) calloc (DIM + 2, sizeof (bool *));

    for (i = 0; i < DIM + 2; i++)
    {
        grid[i] = (bool *) calloc (DIM + 2, sizeof (bool *));
        new_grid[i] = (bool *) calloc (DIM + 2, sizeof (bool *));
    }
    #else
    grid = (int **) calloc (DIM + 2, sizeof (int *));
    new_grid = (int **) calloc (DIM + 2, sizeof (int *));

    for (i = 0; i < DIM + 2; i++)
    {
        grid[i] = (int *) calloc (DIM + 2, sizeof (int *));
        new_grid[i] = (int *) calloc (DIM + 2, sizeof (int *));
    }
    #endif

    /*assign arrays to pointers */
    grid_ptr = grid;
    new_grid_ptr = new_grid;

    /*used if user wants to fill grid using rand */
    //fillRand(grid_ptr);

    readFile (FILENAME, grid_ptr);

    printGrid (grid_ptr);
    printf ("\nCells: %d\nAlive: %d\n", cell_count, life_count);

    /*private OMP vars */
    int start, stop, tid;

	/**
     * Create a team of threads (defined by nthreads) to work on the following code.
	 * Run for MAXGEN iterations of Game.
	 **/
    #if SHARED == 1
    #pragma omp parallel shared(grid_ptr, new_grid_ptr, temp_ptr, nthreads) private(gen, tid, start, stop)  num_threads(nthreads)
    {
        //store the threads identifier in local memory variable
        tid = omp_get_thread_num ();

        /*define the amount of work for each thread */
        start = ((DIM / nthreads) * tid) + 1;
        stop = (DIM / nthreads) + start - 1;

        printf ("thread %d dimensions \n start: %d \n stop: %d\n\n", tid,
            start, stop);

        for (gen = 0; gen < MAXGEN; gen++)
        {
            /*only executed by one thread */
            #pragma omp single
            {
                //copy the ghost cells
                copyGhostCells(grid_ptr);
            } //implied barrier
            
            //process chunk using start and stop as limits
            process (grid_ptr, new_grid_ptr, start, stop, tid);

            //sync
            #pragma omp barrier
            
            //swap pointers
            #pragma omp single
            {
                temp_ptr = grid_ptr;
                grid_ptr = new_grid_ptr;
                new_grid_ptr = temp_ptr;
            }
        }
    }				//end of omp

#else

    #pragma omp parallel shared(grid_ptr, new_grid_ptr, temp_ptr, nthreads) private(gen, tid, start, stop) num_threads(nthreads)
    {
        tid = omp_get_thread_num ();

        /*private grid variables */
        int pg, pg_size, x, y;
        bool **priv_grid;

        /*define the amount of work for each thread */
        start = ((DIM / nthreads) * tid) + 1;
        stop = (DIM / nthreads) + start - 1;

        /*number of int pointers need to store */
        pg_size = stop - start + 1 + 2;

        /*private grid declaration */
        priv_grid = (bool **) calloc (pg_size, sizeof (bool *) * (pg_size));
        for (pg = start - 1; pg <= stop + 1; pg++)
        {
            priv_grid[pg] = (bool *) malloc (sizeof (bool *) * (DIM + 2));
        }
        
        #pragma omp barrier
        
        for (gen = 0; gen < MAXGEN; gen++)
        {
            #pragma omp single
            {
                copyGhostCells(grid_ptr);
            }
            
            //copy chunk of global state to private memory array
            privMemCopy (start, stop, priv_grid, grid_ptr);

            #pragma omp barrier

            process (priv_grid, new_grid_ptr, start, stop, tid);

            #pragma omp barrier

            #pragma omp single
            {
                temp_ptr = grid_ptr;
                grid_ptr = new_grid_ptr;
                new_grid_ptr = temp_ptr;
            }
        }   
    }//end of omp

#endif
//end selective compilation

    //get time spent
    clock_gettime (CLOCK_MONOTONIC, &end);
    time_spent = (end.tv_sec - begin.tv_sec);
    time_spent = time_spent + (end.tv_nsec - begin.tv_nsec) / 1000000000.0;

    //print alive count
    printGrid (grid_ptr);
    printf ("\nCells: %d\nAlive: %d\n", cell_count, life_count);

    //print time
    printf ("\nTime taken with %d threads is: %f\n", nthreads, time_spent);

    //free memory for global state arrays
    free(grid);
    free(new_grid);
    return (0);
}

#if INLINE == 1 && BOOL == 1
inline void copyGhostCells(bool** grid_ptr)
#elif INLINE == 0 && BOOL == 1
void copyGhostCells(bool** grid_ptr)
#elif INLINE == 0 && BOOL == 0
void copyGhostCells(int** grid_ptr)
#elif INLINE == 1 && BOOL == 0
inline void copyGhostCells(int** grid_ptr)
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

void
fillRand (int **grid_ptr)
{
  int i, j;

  /*random seed for life generation
   *incorporate time as a variable for more
   *randomness
   */
  srand (SEED);

  cell_count = 0;
  life_count = 0;

    for (i = 1; i <= DIM; i++)
    {
        for (j = 1; j <= DIM; j++)
        {
            /*
            *uses the LIFE var as a probability of life existing in
            *the initial grid.
            */
            if (rand () % LIFE == 1)
            {
                life_count++;
                grid_ptr[i][j] = 1;
            }
            else
            {
                grid_ptr[i][j] = 0;
            }
            cell_count++;
        }
    }
}

#if BOOL == 1
void
printGrid (bool ** g)
#else
void
printGrid (int **g)
#endif
{
  int i, j;

  cell_count = 0;
  life_count = 0;

    for (i = 1; i <= DIM; i++)
    {
        for (j = 1; j <= DIM; j++)
        {
            if (g[i][j] == 1)
            life_count++;
            cell_count++;
        }
    }
}

#if INLINE == 1
inline void
#else
void
#endif
#if BOOL == 1
process (bool ** grid_ptr, bool ** new_grid_ptr, int start, int stop, int tid)
#else
process (int **grid_ptr, int **new_grid_ptr, int start, int stop, int tid)
#endif
{

    int i, j, count;

    //iterate on the rows selected by the for start and stop vars
    for (i = start; i <= stop; i++)
    {
        for (j = 1; j <= DIM; j++)
        {
            //add up the moor-neighbourhood
            count =
            grid_ptr[i - 1][j - 1] + grid_ptr[i - 1][j] +
            grid_ptr[i - 1][j + 1] + grid_ptr[i][j - 1] +
            grid_ptr[i][j + 1] + grid_ptr[i + 1][j - 1] +
            grid_ptr[i + 1][j] + grid_ptr[i + 1][j + 1];

            //apply the rules and write-back the result to the new_grid
            if (count == 3)
            {
                new_grid_ptr[i][j] = 1;
            }
            else if (count < 2 || count > 3)
            {
                new_grid_ptr[i][j] = 0;
            }
            else if (count == 2)
            {
                new_grid_ptr[i][j] = grid_ptr[i][j];
            }
        }
    }
}

#if INLINE == 1
inline void
#else
void
#endif
#if BOOL == 1
privMemCopy (int start, int stop, bool ** priv_grid, bool ** grid_ptr)
#else
privMemCopy (int start, int stop, int **priv_grid, int **grid_ptr)
#endif
{
    int x, y;
    for (x = start - 1; x <= stop + 1; x++)
    {
        for (y = 0; y <= DIM + 1; y++)
        {
            priv_grid[x][y] = grid_ptr[x][y];
        }
    }
}

#if BOOL == 1
void
readFile (char *name, bool ** grid)
#else
void
readFile (char *name, int **grid)
#endif
{
    FILE *fp;
    int c, x = 1, y = 1;

    cell_count = 0;
    life_count = 0;

    fp = fopen (name, "r");
    if (fp == NULL)
    {
        printf ("File open failed.\n");
        exit (0);
    }

    while ((c = fgetc (fp)) != EOF)
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
        else if (c = 10)
        {
            x++;
            y = 1;
        }
    }
    fclose (fp);
}
