#include <iostream>
#include <string>
#include <tuple>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>

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
    char **grid;
    std::unordered_set<char> positions;
    std::unordered_set<char> **superpositions;
    std::unordered_set<std::tuple<char, char, Dir>, Hash::RuleHash> rules;
    std::unordered_map<char, int> weights;

public:
    template <size_t constraintRows, size_t constraintCols>
    Wave(int numRows, int numCols, const std::array<std::array<char, constraintCols>, constraintRows>& constraintGrid)
    {
        rows = numRows;
        cols = numCols;
        generateConstraints(constraintGrid);

        // creating references
        grid = new char *[rows]; // new = heap allocation, otherwise same as C
        superpositions = new std::unordered_set<char> *[rows]; // new = heap allocation, otherwise same as C
        for (int r = 0; r < rows; r++) 
        {
            grid[r] = new char[cols];
            superpositions[r] = new std::unordered_set<char>[cols];
        }

        // initializing all spaces
        for (int r = 0; r < rows; r++) 
        {
            for (int c = 0; c < cols; c++) 
            {
                grid[r][c] = '*';
                superpositions[r][c] = positions; // copy constructor
            }
        }

    }

    void setTile(int r, int c, char newValue) 
    {
        grid[r][c] = newValue;
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

                // ADD TO POSITIONS
                positions.insert( constraintGrid[r][c]);
            }
        }
    }

    void printGrid() const 
    {
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
        std::unordered_map<Dir, std::string> dirToString = {
            {Left, "Left"},
            {Right, "Right"},
            {Up, "Up"},
            {Down, "Down"}
        };

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
    //     {{'S','C'}}
    // }};

    std::array<std::array<char,3>,3> constraintGrid {{
        {{'S','S','S'}},
        {{'C','C','S'}},
        {{'L','L','C'}}
    }};

    Wave wave(10, 10, constraintGrid);
    wave.printGrid();
    wave.printConstraints();
    wave.printSuperpositions();
};