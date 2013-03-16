#define VEC 0
#define BOOL 1
#define DIM 1024
#define MAXGEN 10000
#define THREADS 4
#define FD "1024.dat"

#include <thread>
#include "Grid.cpp"



struct timespec time_start, time_stop;
double time_spent;

//-----------------------------------------------------------------------------

inline void processChunkExtern(int, int, Grid*);

int main()
{
	Grid * grid = new Grid((char*)FD);
    
    vector<std::thread> threadPool;
    
    grid->printGrid();

    clock_gettime(CLOCK_MONOTONIC, &time_start);
    
    for (int gen = 0; gen < MAXGEN; gen ++)
    {
        for(int x = 0; x < THREADS; x++)
        {
            int start = ((DIM / THREADS) * x) + 1;
            int stop = (DIM / THREADS) + start - 1;

            threadPool.push_back(std::thread(processChunkExtern, start, stop, grid));
            
        
        }
  
        for (auto &t : threadPool)
        {
            if(t.joinable())
                t.join();
        }
        
        if(gen != 0)
            grid->finishGen();
    }
    
    clock_gettime(CLOCK_MONOTONIC, &time_stop);
    time_spent = (time_stop.tv_sec - time_start.tv_sec);
    time_spent = time_spent + (time_stop.tv_nsec - time_start.tv_nsec) / 1000000000.0;


    
    grid->printGrid();
    
    printf("\nTime taken with C++11 and %d threads is: %f\n", THREADS, time_spent);
}


inline void processChunkExtern(int st, int sp, Grid * g1)
{
    for(int x = st; x <= sp; x++)
    {
        for(int y = 1; y <= DIM; y++)
        {
            int count = g1->getCell(x-1,y-1) + g1->getCell(x-1,y) + g1->getCell(x-1, y+1) +
                                    g1->getCell(x,y-1) + g1->getCell(x,y+1) +
                                    g1->getCell(x+1,y-1) + g1->getCell(x+1,y) + g1->getCell(x+1,y+1);

            if(count == 3 || (count == 2 && g1->getCell(x, y) == 1))
            {
                g1->setCell(x, y, 1);                    
            }
            else if(count < 2 || count > 3)
            {
                g1->setCell(x, y, 0);
            }
            else if(count == 2)
            {
                g1->setCell(x, y, g1->getCell(x, y));
            }
        }
    }
}
