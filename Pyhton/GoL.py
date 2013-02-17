"""Even though Python supports a OOP architecture I do not feel this is the
right paradigm to use considering the other features Pyhton offers.
1. A low level of encapsulation
2. Functional Aspects: List comprehensions, Infinite Lists, Lazy Evaluation
3. Duck-typing, dynamic typing
4. Notice the With block! Similar to try/catch in Java, opens and closes
streams"""

import pprint

def initGrid():
    #list comprehension
    return [[0 for x in range(dim + 2)] for x in range(dim + 2)]


def readFile(grid, filename):
    #Use a with block here!
    f = open(filename)
    x = 1
    y = 1
    while x <= dim:
        while y <= dim:
            grid[x][y] = int(f.read(1))
            y = y + 1
        f.read(1)
        x = x + 1
        y = 1

#POI: range gives a reading which is inclusive of the first argument but
#exclusive of the second argument!!
def copyGhostCells(x):
    for j in range(1, dim + 1):
        x[j][dim + 1] = x[j][1]
        x[j][0] = x[j][dim]
    for i in range(dim + 2):
        x[0][i] = x[dim][i]
        x[dim + 1][i] = x[1][i]

def countGrid(x):
    return [[(x[i - 1][j - 1] + x[i - 1][j] + x[i - 1][j + 1]
     + x[i][j - 1] + x[i][j + 1] + x[i + 1][j - 1] +
     x[i + 1][j] + x[i + 1][j + 1])
     for j in range(1, len(x[1]) - 1)] for i in range(1, len(x) - 1)]

def change(i, j):
    if i > 0 and i < dim + 1 and j > 0 and j < dim + 1:
        if count_grid[i - 1][j - 1] == 3 or (count_grid[i - 1][j - 1] == 2 and x[i][j] == 1):
            return 1
        elif count_grid[i - 1][j - 1] < 2 or count_grid[i - 1][j - 1] > 3:
            return 0
        elif count_grid[i - 1][j - 1] == 2:
            return x[i][j]
    else:
        return 0


def evolve():
    return [[(change(i, j)) for j in range(dim + 2)] for i in range(dim + 2)]

def sumlist(L):
    return sum(map(sum,L))

dim = 512
x = initGrid()
readFile(x, "512.dat")
print("Read file complete")
#pprint.pprint(x)

for gen in range(100):
    copyGhostCells(x)
    count_grid = countGrid(x)
    new_grid = evolve()    
    if gen != 0:
        x = new_grid
    print(gen, "complete", sumlist(new_grid))

print(gen, sumlist(new_grid))

