package main

import (
    "runtime"
    "strings"
    "io/ioutil"
    "fmt"
    "strconv"
    "time"
    "sync"
)

var dim = 512
var fd = "512.dat"
var threads = 4
var gen = 1000

var grid = make([][]bool, dim + 2)
var new_grid = make([][]bool, dim + 2)

type thread struct{
  Id, St, Sp int
}
var wg sync.WaitGroup
var threadPool = make([]*thread, threads)

func main() {
  for i := range grid {
    grid[i] = make([]bool, dim+2)
    new_grid[i] = make([]bool, dim+2)
  }
  /*necessary to increase the amount of 
   *goroutines that can be mapped to OS threads
   */
  runtime.GOMAXPROCS(threads)
  
  //read in the entire file contents
  content, err := ioutil.ReadFile(fd)
  
  if (err != nil) {
    //error if stuff is wrong
  }
  
  //split strings by newline and create a string slice
  lines := strings.Split(string(content), "\n")
  
  //separate into the array and convert into bool type
  for i, v := range lines{
    y := strings.SplitN(v, "", dim)
    for j, x := range y{
      //offset due to ghost cells
      grid[i+1][j+1], err = strconv.ParseBool(x)
    }
  }
  
  for x:=0; x < threads; x++ {
      st := ((dim / threads) * x) + 1
      sp := (dim / threads) + st - 1
      fmt.Println("Thread ", x," Starts: ", st, " Ends: ", sp)
      threadPool[x] = &thread{ x, st, sp}
  }
  //main game loop!
  i:= 0
  t0 := time.Now()
  for i < gen {
    copyGhostCells()
    for x:= 0; x < threads; x++ {
      wg.Add(1)
      go processGrid(threadPool[x])
    }
    wg.Wait()
    copySlice()
    i++
  }
  t1:= time.Now()
  fmt.Println("Time: ", t1.Sub(t0))
  fmt.Println("Live Cells: ", printGrid())
  
  
}

func printGrid() (count int) {
  count = 0
  x, y := 1, 1
  for x <= dim {
    for y <= dim {
      if (grid[x][y] == true) {
        count++
      }
      y++
    }
    y = 1
    x++
  }
  return
}

func copyGhostCells() {

  for x:= 1; x <= dim; x++ {
    grid[x][dim + 1] = grid[x][1]
    grid[x][0] = grid[x][dim]
  }
  
  for y:= 0; y <= dim + 1; y++ {
    grid[0][y] = grid[dim][y]
    grid[dim + 1][y] = grid[1][y]
  }
}

func copySlice() {
  for x:= range grid {
    for y:= range grid {
      grid[x][y] = new_grid[x][y]
    }
  }
}

func processGrid(t * thread) {
  for x:= t.St; x <= t.Sp; x++ {
    for y:= 1; y <= dim; y++ {
      count := addBool(grid[x - 1][y - 1], grid[x - 1][y], grid[x - 1][y + 1], grid[x][y - 1], grid[x][y + 1], grid[x + 1][y - 1], grid[x + 1][y], grid[x + 1][y + 1])
      switch count {
        case 0,1,4,5,6,7,8:   new_grid[x][y] = false
        case 3:               new_grid[x][y] = true
        case 2:               new_grid[x][y] = grid[x][y]
      }
    }
  }
  wg.Done()
}

func addBool(a ... bool) (count int) {
    count = 0
    for _, v := range a {
      if(v == true) {
          count++
      }
    }
    return
}
