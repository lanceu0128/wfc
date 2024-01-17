#include <iostream>
#include <string>
#include <tuple>
#include <array>
#include <vector>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <cmath>
#include <random>
#include <iomanip>

enum Dir { Up, Right, Down, Left };

namespace Hash {
    // custom hash function for the rules tuple (char, char, Dir) 
    struct RuleHash {
        size_t operator()(const std::tuple<char, char, Dir>& t) const {
            auto hash1 = std::hash<char>{}(std::get<0>(t));
            auto hash2 = std::hash<char>{}(std::get<1>(t));
            auto hash3 = std::hash<Dir>{}(std::get<2>(t));
            return hash1 ^ hash2 ^ hash3;
        }
    };
}

class Wave
{
private:
    int rows;
    int cols;
    
    // grids
    char **grid;
    double **entropies;
    std::set<char> **superpositions;
    
    // constraints
    std::set<char> positions;
    std::unordered_set<std::tuple<char, char, Dir>, Hash::RuleHash> rules;
    std::unordered_map<char, double> weights;
    double weightTotal = 0;
    // double minEntropy = -1;

    // Dir enum -> string translator
    std::unordered_map<Dir, std::string> dirToString = {
        {Left, "Left"},
        {Right, "Right"},
        {Up, "Up"},
        {Down, "Down"}
    };

public:
    template <size_t constraintRows, size_t constraintCols>
    Wave(int numRows, int numCols, const std::array<std::array<char, constraintCols>, constraintRows>& constraintGrid)
    {
        rows = numRows;
        cols = numCols;
        generateConstraints(constraintGrid);
        generateGrids();
    }

    void generateGrids() {
        // creating references
        grid = new char *[rows]; // new = heap allocation, otherwise same as C
        superpositions = new std::set<char> *[rows]; // new = heap allocation, otherwise same as C
        entropies = new double *[rows];

        for (int r = 0; r < rows; r++) 
        {
            grid[r] = new char[cols];
            superpositions[r] = new std::set<char>[cols];
            entropies[r] = new double[cols];
        }

        // initializing all spaces
        for (int r = 0; r < rows; r++) 
        {
            for (int c = 0; c < cols; c++) 
            {
                grid[r][c] = '*';
                superpositions[r][c] = positions; 
                entropies[r][c] = calculateEntropy(r, c);  
            }
        }
    }

    void collapse(int r, int c) 
    {
        // pick random from superpositions
        int idx = (rand() % superpositions[r][c].size());
        auto it = superpositions[r][c].begin();
        std::advance(it, idx);
        grid[r][c] = *it;

        // clear data and propogate
        superpositions[r][c].clear();
        entropies[r][c] = 0;
        propogate(r, c);
    }

    void propogate(int r, int c) {
        if (r > 0) { compareTiles(grid[r][c], r-1, c, Down); };
        if (c < cols-1) { compareTiles(grid[r][c], r, c+1, Left); };
        if (r < rows-1) { compareTiles(grid[r][c], r+1, c, Up); };
        if (c > 0) { compareTiles(grid[r][c], r, c-1, Right); };
    }

    void compareTiles(char tile1, int tile2_r, int tile2_c, Dir d) {
        if (grid[tile2_r][tile2_c] != '*') { return; };

        std::vector<char> positionsToErase;

        for (const auto& position : superpositions[tile2_r][tile2_c]) {
            std::tuple<char, char, Dir> ruleToFind = std::make_tuple(tile1, position, d);
    
            auto it = rules.find(ruleToFind); 

            if (it == rules.end()) { // rule not found
                positionsToErase.push_back(position);
            }       
        }

        for (const auto& position : positionsToErase) {
            superpositions[tile2_r][tile2_c].erase(position);
        }

        entropies[tile2_r][tile2_c] = calculateEntropy(tile2_r, tile2_c);
    }

    double calculateEntropy(int r, int c) {
        double entropy = 0;
        for (const auto& event : superpositions[r][c]) {
            double prob_event = weights[event] / weightTotal;
            entropy += prob_event * log(prob_event);  
        }

        // if (-entropy < minEntropy || minEntropy == -1) { minEntropy = -entropy; }; 

        return -entropy;
    }

    template <size_t constraintRows, size_t constraintCols>
    void generateConstraints(const std::array<std::array<char, constraintCols>, constraintRows>& constraintGrid) 
    {
        for (int r=0; r < constraintRows; r++) 
        {
            for (int c=0; c < constraintCols; c++) 
            {
                // LOOK UP
                if (r > 0) { rules.insert( {constraintGrid[r][c], constraintGrid[r-1][c], Down} ); };

                // LOOK RIGHT
                if (c < constraintCols-1) { rules.insert( {constraintGrid[r][c], constraintGrid[r][c+1], Left} ); };

                // LOOK DOWN
                if (r < constraintRows-1) { rules.insert( {constraintGrid[r][c], constraintGrid[r+1][c], Up} ); };

                // LOOK LEFT
                if (c > 0) { rules.insert( {constraintGrid[r][c], constraintGrid[r][c-1], Right} ); };

                // ADD TO WEIGHTS
                if (weights[ constraintGrid[r][c] ] == 0) {
                    weights[ constraintGrid[r][c] ] = 1;
                } else {
                    weights[ constraintGrid[r][c] ] = weights[ constraintGrid[r][c] ] + 1;
                }
                weightTotal += 1;

                // ADD TO POSITIONS
                positions.insert( constraintGrid[r][c]);
            }
        }
    }

    bool step() {
        // find minimal nonzero entropy
        double minEntropy = -1;
        for (int r=0; r < rows; r++) 
        {
            for (int c=0; c < cols; c++) 
            {
                if ((minEntropy == -1 || entropies[r][c] < minEntropy) && entropies[r][c] != 0) { 
                    minEntropy = entropies[r][c];
                }
            }
        } 

        // break if all cells are empty
        if (minEntropy == -1) { return false; }

        // find all elements with minimal entropy
        std::vector<std::tuple<int, int>> minEntropyTiles;
        for (int r=0; r < rows; r++) 
        {
            for (int c=0; c < cols; c++) 
            {
                if (entropies[r][c] == minEntropy && entropies[r][c] != 0) { 
                    minEntropyTiles.push_back(std::make_tuple(r, c));
                }; 
            }
        } 

        // collapse and propogate random cell with minimum entropy
        int idx = (rand() % minEntropyTiles.size());
        std::tuple<int, int> randMinEntropyTile = minEntropyTiles[idx];
        collapse(std::get<0>(randMinEntropyTile), std::get<1>(randMinEntropyTile));

        return true;
    }

    void printGrid() const 
    {
        std::cout << std::endl;
        for (int r=0; r < rows; r++) 
        {
            for (int c=0; c < cols; c++) 
            {
                std::cout << grid[r][c] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    void printEntropies() const
    {
        for (int r=0; r<rows; r++) 
        {
            for (int c=0; c<cols; c++)
            {
                std::cout << std::fixed << std::setprecision(10) << entropies[r][c] << " ";
            }
            std::cout << std::endl;
        }
        // std::cout << "Minimum Entropy: " << minEntropy << std::endl;
    }

    void printSuperpositions() const 
    {
        for (int r=0; r < rows; r++) 
        {
            for (int c=0; c < cols; c++) 
            {
                std::cout << "[";
                for (const auto& element : superpositions[r][c]) {
                    std::cout << element;
                }
                std::cout << "] ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    void printConstraints() {
        std::cout << "Rules:" << std::endl;
        for (const auto& rule : rules) {
            std::cout << "(" << std::get<0>(rule) << ", " << std::get<1>(rule)
                  << ", " << dirToString[std::get<2>(rule)] << ")" << std::endl;
        }

        std::cout << "\nPositions:" << std::endl;
        for (const auto& position : positions) {
            std::cout << position << std::endl;
        }

        std::cout << "\nWeights:" << std::endl;
        for (const auto& [key, value] : weights) {
            std::cout << '[' << key << "] = " << value << std::endl;
        }
    }
};

int main()
{
    // std::array<std::array<char,2>,1> constraintGrid {{
    //     {{'C','S'}}
    // }};

    std::srand(std::time(0));

    std::array<std::array<char,3>,3> constraintGrid {{
        {{'C','S','S'}},
        {{'L','C','S'}},
        {{'L','L','C'}}
    }};

    Wave wave(4, 4, constraintGrid);

    bool retval = true;
    while (retval) {
        wave.printGrid();
        wave.printEntropies();
        retval = wave.step();
    }
};