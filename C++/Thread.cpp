#define VEC 0
#define MALLOC 0
#define DIM 1024
#define MAXGEN 10000
#define THREADS 4
#define FD "1024.dat"


#include "boost/thread.hpp"
#include "Grid.cpp"


struct timespec begin, end;
double time_spent;

class ThreadClass
{
public: // methods
    /** Constructor
    *
    * starts the internal thread running.
    */
    ThreadClass(int, int, int, Grid*);
    void run();
    int getTid();
    void join();
    
    /** Destructor
    *
    * Blocks until the thread has finished executing, if it hasn't
    * finished already.
    */
    ~ThreadClass();

private: // methods
    /** This is the function that the thread executes.
    *
    * The thread will finish when this function returns.
    */
    void threadMain();
    void processChunk();



private: // data
    
    boost::thread internalThread_;
    int tid; 
    int start;
    int stop;
    Grid * g1;

};

//-----------------------------------------------------------------------------
ThreadClass::ThreadClass(int t, int st, int sp, Grid * g)
{
	tid = t;
	start = st;
	stop = sp;
	g1 = g;

} // Constructor

ThreadClass::~ThreadClass()
{
  internalThread_.interrupt();
  internalThread_.join(); // make damn sure that the internal thread is gone
                          // before we destroy the class data.
} // Destructor

//-----------------------------------------------------------------------------
void ThreadClass::threadMain()
{
  try
  {
    /* add whatever code you want the thread to execute here. */
	processChunk();
  }
  catch (boost::thread_interrupted& interruption)
  {
    // thread was interrupted, this is expected.

  }
  catch (std::exception& e)
  {
    // an unhandled exception reached this point, this constitutes an error

  }

} // threadMain

void ThreadClass::run()
{
	internalThread_ = boost::thread(&ThreadClass::threadMain, this);
}

void ThreadClass::join()
{
    internalThread_.join();
}

void ThreadClass::processChunk()
{
    for(int x = start; x <= stop; x++)
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

int ThreadClass::getTid()
{
    return tid;
}

//-----------------------------------------------------------------------------

int main()
{
	Grid * grid = new Grid((char*)FD);
    vector<ThreadClass*> threadPool;
    
    grid->printGrid();
    
    
    clock_gettime(CLOCK_MONOTONIC, &begin);
    
    for (int gen = 0; gen < MAXGEN; gen ++)
    {
        for(int x = 0; x < THREADS; x++)
        {
            int start = ((DIM / THREADS) * x) + 1;
            int stop = (DIM / THREADS) + start - 1;

            ThreadClass * life = new ThreadClass(x, start, stop, grid);
            
            threadPool.push_back(life);
        }
        
        for (int x = 0; x < THREADS; x++)
        {
            threadPool[x]->run();
        }
        
        for (int x = 0; x < THREADS; x++)
        {
            threadPool[x]->join();
        }
        
        if(gen != 0)
            grid->finishGen();
    }
    
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_spent = (end.tv_sec - begin.tv_sec);
    time_spent = time_spent + (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
    
    
    grid->printGrid();
    
    printf("\nTime taken with %d threads is: %f\n", THREADS, time_spent);
}
