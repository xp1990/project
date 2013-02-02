#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>
#include <math.h>

#define MAXGEN 10000
#define DIM 512
#define LIFE 3
#define SEED 2012
#define THREADS 2
#define SHARED 1
#define INLINE 1
#define REGISTER 0

#if REGISTER == 1
register int** grid;
register int** new_grid;
#else
int** grid;
int** new_grid;
#endif

struct timespec begin, end;

unsigned int nthreads;

void print_grid(int**);

#if INLINE == 1
inline void process(int**, int**, int, int, int);
inline void priv_memcpy(int, int, int**, int**);
#else
void process(int**, int**, int, int, int);
void priv_memcpy(int, int, int**, int**);
#endif


int
main (int argc, char *argv[])
{
  int **grid_ptr, **new_grid_ptr, **temp_ptr;

  /*Assign global var to num_threads*/
  nthreads = THREADS;

  /*vars for timing the main game loop*/
  double time_spent;

  /*main game vars*/
  int i, j, gen, cell_count, life_count;

  /*in c a 2d grid is an array of pointers to pointers of ints
   *here we allocate enough space for our temp transfer grid and
   *our original grid
   */
  grid = (int**) malloc(sizeof(int*) * DIM+2);
  new_grid = (int**) malloc(sizeof(int*) * DIM+2);
  for(i=0; i < DIM+2; i++)
  {
    grid[i] = (int*) malloc(sizeof(int*) * DIM+2);
    new_grid[i] = (int*) malloc(sizeof(int*) * DIM+2);
  }

  /*assign arrays to pointers*/
  grid_ptr = grid;
  new_grid_ptr = new_grid;

  /*random seed for life generation
   *incorporate time as a variable for more
   *randomness
   */
  srand(SEED);

  cell_count = 0;
  life_count = 0;

  for(i=1; i <=DIM; i++)
  {
    for(j=1; j <=DIM; j++)
    {
      /*
       *uses the LIFE var as a probability of life existing in
       *the initial grid.
       */
      if(rand() % LIFE == 1)
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

  printf("\nCells: %d\nAlive: %d\n", cell_count, life_count);

  /*private OMP vars*/
  int start, stop, tid;

  /*begin timing*/
  clock_gettime(CLOCK_MONOTONIC, &begin);

  /*
   *Run for MAXGEN iterations of Game.
   */

#if SHARED == 1
  #pragma omp parallel shared(grid_ptr, new_grid_ptr, temp_ptr, nthreads) private(gen, tid, start, stop)  num_threads(nthreads)
  {
    tid = omp_get_thread_num();

    /*define the amount of work for each thread*/
    start = ((DIM / nthreads) * tid) + 1;
    stop = (DIM / nthreads) + start - 1;

    printf("thread %d dimensions \n start: %d \n stop: %d\n\n", tid, start, stop);

    for(gen = 0; gen < MAXGEN; gen++)
    {

      /*only executed by thread 0*/
      if(tid == 0)
      {
       /**
        *There is another way of doing this using nested loops.
        *and if statements however this way is FAR more efficient!
        **/

        /*copy ghost columns to grid*/
        for (i=1; i <=DIM; i++)
        {
          grid_ptr[i][DIM+1] = grid_ptr[i][1];
          grid_ptr[i][0] = grid_ptr[i][DIM];
        }
        /*copy ghost rows to grid*/
        for(j=0; j <=DIM+1; j++)
        {
          grid_ptr[0][j] = grid_ptr[DIM][j];
          grid_ptr[DIM+1][j] = grid_ptr[1][j];
        }

      }

      /*NOTE: Does this need to be here?*/
      #pragma omp barrier

      process(grid_ptr, new_grid_ptr, start, stop, tid);

      #pragma omp barrier

      if(gen != 0 && tid == 0)
      {
        temp_ptr = grid_ptr;
        grid_ptr = new_grid_ptr;
        new_grid_ptr = temp_ptr;

        //print_grid(grid_ptr);
      }
    }
  }//end of omp

  

#else

  

  #pragma omp parallel shared(grid_ptr, new_grid_ptr, temp_ptr, nthreads) private(gen, tid, start, stop)  num_threads(nthreads)
  {
    tid = omp_get_thread_num();
    
    /*private grid variables*/
    int pg, pg_size, x, y;
    int **priv_grid;

    /*define the amount of work for each thread*/
    start = ((DIM / nthreads) * tid) + 1;
    stop = (DIM / nthreads) + start - 1;

    /*number of int pointers need to store*/
    pg_size = stop - start + 1 + 2;

    /*BIG NOTE! Calloc is the key!!
     *It allocates memory and initialises it!! 0's Everywhere!
     *Malloc did not work!
     */
    
    /*private grid declaration*/
    priv_grid = (int**)calloc(pg_size, sizeof(int*) * (pg_size));
    for(pg = start-1; pg <= stop+1; pg++)
    {
      //printf("thread: %d, row: %d\n", tid, pg);
      priv_grid[pg] = (int*)malloc(sizeof(int*) * (DIM+2));
    }

    printf("thread %d dimensions \n start: %d \n stop: %d \n size: %d\n\n", tid, start, stop, pg_size);

    for(gen = 0; gen < MAXGEN; gen++)
    {
      if(tid == 0)
      {
       /**
        *There is another way of doing this using nested loops.
        *and if statements however this way is FAR more efficient!
        **/

        /*copy ghost columns to grid*/
        for (i=1; i <=DIM; i++)
        {
          grid_ptr[i][DIM+1] = grid_ptr[i][1];
          grid_ptr[i][0] = grid_ptr[i][DIM];
        }
        /*copy ghost rows to grid*/
        for(j=0; j <=DIM+1; j++)
        {
          grid_ptr[0][j] = grid_ptr[DIM][j];
          grid_ptr[DIM+1][j] = grid_ptr[1][j];
        }

      }

			//printf("beginning private memory copy...\n");

      priv_memcpy(start, stop, priv_grid, grid_ptr);

			//printf("Copied grid to private thread memory\n");

      #pragma omp barrier

      process(priv_grid, new_grid_ptr, start, stop, tid);

      #pragma omp barrier

      if(gen != 0 && tid == 0)
      {
        temp_ptr = grid_ptr;
        grid_ptr = new_grid_ptr;
        new_grid_ptr = temp_ptr;

        //print_grid(grid_ptr);
      }
    }
  }//end of omp

#endif

  //end = clock();
  clock_gettime(CLOCK_MONOTONIC, &end);
  time_spent = (end.tv_sec - begin.tv_sec);
  time_spent = time_spent + (end.tv_nsec - begin.tv_nsec) / 1000000000.0;

  printf("\nTime taken with %d threads is: %f\n", nthreads, time_spent);

  
  return(0);
}

void
print_grid(int** g)
{
  int i,j;

  for(i=1; i <= DIM; i++)
  {
    for(j=1; j <= DIM; j++)
    {
      printf("%d", g[i][j]);
    }
    printf("\n");
  }
    printf("\n");

}

#if INLINE == 1
inline void
#else
void
#endif
process(int** grid_ptr, int** new_grid_ptr, int start, int stop, int tid)
{

  /*this can be made more efficient! produce two versions using compiler flags.
   *second version will break loop after a condition is met... 
   */
  

  int i,j,count;

  for(i=start; i <= stop; i++)
  {
    for(j=1; j <= DIM; j++)
    {
      count = grid_ptr[i-1][j-1] + grid_ptr[i-1][j] + grid_ptr[i-1][j+1] +
              grid_ptr[i][j-1] + grid_ptr[i][j+1] +
              grid_ptr[i+1][j-1] + grid_ptr[i+1][j] + grid_ptr[i+1][j+1];

      if(count == 3 || (count == 2 && grid[i][j] == 1))
      {
        new_grid_ptr[i][j] = 1;
        //printf("Thread %d creating life at %d,%d\n", tid, i, j);
      }
      else if(count < 2 || count > 3)
      {
        new_grid_ptr[i][j] = 0;
        //printf("Thread %d: destroying life at %d,%d\n", tid, i, j);
      }
    }
  }
}

#if INLINE == 1
inline void
#else
void
#endif
priv_memcpy(int start, int stop, int** priv_grid, int** grid_ptr)
{
  int x, y;
  
  for(x = start-1; x <= stop+1; x++)
  {
    for(y = 0; y <= DIM+1; y++)
    {
      priv_grid[x][y] = grid_ptr[x][y];
    }
  }
}
