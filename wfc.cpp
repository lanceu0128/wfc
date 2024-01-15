#include <iostream>
#include <string>
#include <tuple>
#include <array>
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
    std::unordered_set<std::tuple<char, char, Dir>, Hash::RuleHash> rules;

public:
    Wave(int numRows, int numCols)
    {
        rows = numRows;
        cols = numCols;

        grid = new char *[rows]; // new = heap allocation, otherwise same as C
        for (int r = 0; r < rows; r++) 
        {
            grid[r] = new char[cols];
        }

        for (int r = 0; r < rows; r++) 
        {
            for (int c = 0; c < cols; c++) 
            {
                grid[r][c] = '*';
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
            }
        }
    }

    void printConstraints() {
        std::unordered_map<Dir, std::string> dirToString = {
            {Left, "Left"},
            {Right, "Right"},
            {Up, "Up"},
            {Down, "Down"}
        };

        for (const auto& rule : rules) {
            std::cout << "Tuple: (" << std::get<0>(rule) << ", " << std::get<1>(rule)
                  << ", " << dirToString[std::get<2>(rule)] << ")" << std::endl;
        }
    }
};

int main()
{
    Wave wave(10, 10);

    std::array<std::array<char,3>,3> constraintGrid {{
        {{'S','S','S'}},
        {{'C','C','S'}},
        {{'L','L','C'}}
    }};

    // std::array<std::array<char,2>,1> constraintGrid {{
    //     {{'S','C'}}
    // }};

    wave.generateConstraints(constraintGrid);
    wave.printConstraints();

    // TESTS FOR STL:

    // unordered_set<char> s1;
    // s1.insert('A');
    // s1.insert('B');
    // s1.insert('C');
    // s1.insert('A');

    // tuple<string, string, Dir> t1 = {"bruh", "bruh2", Down};

    // std::cout << "First string: " << std::get<0>(t1) << std::endl;
    // std::cout << "Second string: " << std::get<1>(t1) << std::endl;
    // std::cout << "Direction: " << std::get<2>(t1) << std::endl;

    // std::unordered_set<std::tuple<char, char, Dir>, Hash::RuleHash> rule;
    // rule.insert({'S', 'C', Down});

    // auto it = rule.find({'S', 'C', Up});
    // if (it != rule.end()) {
    //     std::cout << "Element " << " is in the set." << std::endl;
    // } else {
    //     std::cout << "Element " << " is not in the set." << std::endl;
    // }
};