import java.util.*;
import java.io.*;
import java.lang.*;
import java.nio.charset.Charset;

class LifeThread extends Thread
{
	private final int tid, start, stop, DIM;
	private Grid g1;
    private int x, y;

	public LifeThread(int t, int st, int sp, Grid g, int D)
	{
		tid = t;

		start = st;
		stop = sp;

		DIM = D;

		g1 = g;
	}

	public void run()
	{
        try
        {
            processChunk();      
        }
        catch(Exception e)
        {
            System.out.println("Exception: " + e.toString() + " Thread: " + tid + " at: " + x + "," + y);
        }
	}

    private void processChunk()
    {
        int count = 0;
        
        for(x = start; x <= stop; x++)
        {
            for(y = 1; y <= DIM; y++)
            {
                for(int i = -1; i < 2; i++)
                {
                    for(int j = -1; j < 2; j++)
                    {
                        if(!(i == 0 && j == 0))
                        {
                            count += boolToInt(g1.getCell(x + i, y + j));
                        }
                    }
                }

                if(count == 3 || (count == 2 && g1.getCell(x, y) == true))
                {
                    g1.setCell(x, y, true);                    
                }
                else if(count < 2 || count > 3)
                {
                    g1.setCell(x, y, false);
                }
                else if(count == 2)
                {
                    g1.setCell(x, y, g1.getCell(x, y));
                }
                count = 0;
            }
        }
    }
    
	public int getTid()
	{
		return tid;
	}
    
    private int boolToInt(boolean val)
    {
        if(val == true)
            return 1;
        else
            return 0;
    }
}

class Grid
{
	private final int DIM, LIFE;

    private final String FILENAME;

	private boolean[][] grid, new_grid;

    private Random r1;
    

	public Grid(int D, int L, String F)
	{
		DIM = D;
		LIFE = L;
        FILENAME = F;

		grid = new boolean[DIM+2][DIM+2];
		new_grid = new boolean[DIM+2][DIM+2];

        r1 = new Random(2012);
        
		//fillArrayRand();
        readFileIn(FILENAME);
		copyGhostCells();
		//printGrid();

	}

	public void finishGen()
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

	public void printGrid()
	{
		int cellCount = 0;
		int aliveCount = 0;

		for(int x = 1; x <= DIM; x++)
		{
			for(int y = 1; y <= DIM; y++)
			{
				if(getCell(x,y) == true)
					aliveCount++;

				cellCount++;
				//System.out.print(getCell(x,y));
			}
			//System.out.print("\n");
		}

		//System.out.println("Cells: " + cellCount);
		System.out.println("Alive: " + aliveCount);
	}

	public boolean getCell(int x, int y)
	{
		return grid[x][y];
	}

	public void setCell(int x, int y, boolean val)
	{
		new_grid[x][y] = val;
	}

	private void fillArrayRand()
    {
        for(int x = 1; x <= DIM; x++)
        {
          for(int y = 1; y <= DIM; y++)
          {
                
                if (r1.nextInt(LIFE) == 1)
                    grid[x][y] = true;
                else
                    grid[x][y] = false;
          }
        }
    }

	public void copyGhostCells()
	{

		/*copy ghost columns to grid*/
		for(int x = 1; x <= DIM; x++)
		{
			grid[x][DIM+1] = grid[x][1];
			grid[x][0] = grid[x][DIM];
		}

		/*copy ghost rows to grid*/
		for(int y = 0; y <= DIM+1; y++)
		{
			grid[0][y] = grid[DIM][y];
			grid[DIM+1][y] = grid[1][y];
		}
	}

    private void readFileIn(String filename)
    {
        int x = 1, y = 1, r;
        
        try
        {
            Charset encoding = Charset.defaultCharset();
            File file = new File(filename);
            InputStream in = new FileInputStream(file);
            Reader reader = new InputStreamReader(in, encoding);
            Reader buffer = new BufferedReader(reader);
            while((r = buffer.read()) != -1) //similar to reading line in C
            {
                if((char)r == '1')
                {
                    grid[x][y] = true;
                    y++;
                }
                else if((char)r == '0')
                {
                    grid[x][y] = false;
                    y++;
                }
                else if(r == 10)
                {
                    x++;
                    y = 1;
                }
            }
        }
        catch(Exception e)
        {
            System.out.println(e);
        }
    }

}

public class GoL
{

    private static final int THREADS = 4, DIM = 1024, LIFE = 3, GEN = 10000;

    private static final String FILENAME = "1024.dat";

    public static void main(String args[])
    {
		Stopwatch stopwatch = new Stopwatch();
        stopwatch.start();
        Grid g1 = new Grid(DIM, LIFE, FILENAME);
		Vector<LifeThread> life_vec = new Vector<LifeThread>();

        try
        {
            for(int gen = 0; gen < GEN; gen++)
            {
                for(int x = 0; x < THREADS; x++)
                {
                    int start = ((DIM / THREADS) * x) + 1;
                    int stop = (DIM / THREADS) + start - 1;

                    LifeThread life = new LifeThread(x, start, stop, g1, DIM);

                    life_vec.add(x, life);
                }

                for(int x = 0; x < THREADS; x++)
                {
                    life_vec.get(x).start();
                    //System.out.println("Thread: " + life_vec.get(x).getTid() + " at " + gen);
                    
                }   

                for(int x = 0; x < THREADS; x++)
                    life_vec.get(x).join();
                
                //if(gen != 0) //Why?? No one knows!
                //System.out.println( "Gen: " + gen);
                //g1.printGrid();
                g1.finishGen();
              
            }
            stopwatch.stop();
            
            g1.printGrid();
            System.out.println("Time: " + stopwatch);
        }
        catch(Exception e)
        {
            System.out.println("Exception: " + e);
        }
        
    }
}


