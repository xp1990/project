"""Even though Python supports a OOP architecture I do not feel this is the
right paradigm to use considering the other features Pyhton offers.
1. A low level of encapsulation
2. Functional Aspects: List comprehensions, Infinite Lists, Lazy Evaluation
3. Duck-typing, dynamic typing
4. Notice the With block! Similar to try/catch in Java, opens and closes
streams"""

"""None of the functions have side effects (beside conway(), which does
printing), as emphasized by the functional programming style. Also, no function
depends on some global state - hence they are all deterministic. This means that
those functions always return the same output when called with a specific input.
They are a mapping of input to output"""

import pprint
import pickle

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
    return grid

#POI: range gives a reading which is inclusive of the first argument but
#exclusive of the second argument!!
def copyGhostCells(x):
    for j in range(1, dim + 1):
        x[j][dim + 1] = x[j][1]
        x[j][0] = x[j][dim]
    for i in range(dim + 2):
        x[0][i] = x[dim][i]
        x[dim + 1][i] = x[1][i]
    return x

def count(x, i, j):
    #It would be super cool if I could return a lambda function here!!
    return (x[i - 1][j - 1] + x[i - 1][j] + x[i - 1][j + 1]
     + x[i][j - 1] + x[i][j + 1] + x[i + 1][j - 1] +
     x[i + 1][j] + x[i + 1][j + 1])

def countGrid(x):
    return [[count(x, i, j) for j in range(1, len(x[1]) - 1)] for i in range(1, len(x) - 1)]
    #return [[1 for j in range(1, len(x[1]) - 1)] for i in range(1, len(x) - 1) if rules(count(i, j)) == 1]

def change(count_grid, x, i, j):
    if i > 0 and i < dim + 1 and j > 0 and j < dim + 1:
        if count_grid[i - 1][j - 1] == 3 or (count_grid[i - 1][j - 1] == 2 and x[i][j] == 1):
            return 1
        elif count_grid[i - 1][j - 1] < 2 or count_grid[i - 1][j - 1] > 3:
            return 0
        elif count_grid[i - 1][j - 1] == 2:
            return x[i][j]
    else:
        return 0


def evolve(count_grid, x):
    return [[(change(count_grid, x, i, j)) for j in range(dim + 2)] for i in range(dim + 2)]

def sumlist(L):
    return sum(map(sum,L))

def outputFile(gen):
    w = open("gen%i.ppm" % gen, 'w')
    w.write("P1\n%i\n%i\n" % (dim, dim))
    for i in range(1, dim):
        for j in range(1, dim):
            w.write(" " + str(new_grid[i][j]))
        w.write("\n")

dim = 512
x = initGrid()
x = readFile(x, "512.dat")
print("Read file complete")
#pprint.pprint(x)

for gen in range(10):
    x = copyGhostCells(x)
    count_grid = countGrid(x)
    new_grid = evolve(count_grid, x)    
    if gen != 0:
        x = new_grid
    print(gen, "complete", sumlist(new_grid))
    outputFile(gen)

print(gen, sumlist(new_grid))
 
