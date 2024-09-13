#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>

using namespace std;

class Solution {
public:
    void maze(vector<vector<char>>& map) {
        // Initialize the maze
        for (int i = 0; i < map.size(); ++i) {
            for (int j = 0; j < map[0].size(); ++j) {
                if (i % 2 == 0) {
                    map[i][j] = '+';
                } else {
                    map[i][j] = ' '; 
                }

                if (i % 2 == 0 && j % 2 != 0) {
                    map[i][j] = '-';
                } else if (i % 2 != 0 && j % 2 == 0) {
                    map[i][j] = '|';
                }
            }
        }


        _maze(map, 1, 1);
    }

    void showMaze(const vector<vector<char>>& map) const {
        for (const auto& row : map) {
            for (const auto& cell : row) {
                cout << cell;
            }
            cout << endl;
        }
    }

    void _maze(vector<vector<char>>& map, int i, int j) {
        static const int directions[4][2] = {{0, 2}, {0, -2}, {2, 0}, {-2, 0}}; // Right, Left, Down, Up
        int visitOrder[4] = {0, 1, 2, 3};

        // Random map maker
        shuffle(begin(visitOrder), end(visitOrder), mt19937{random_device{}()});

        for (int k = 0; k < 4; ++k) {
            int ni = i + directions[visitOrder[k]][0];
            int nj = j + directions[visitOrder[k]][1];

            
            if (ni > 0 && nj > 0 && ni < map.size() - 1 && nj < map[0].size() - 1 && map[ni][nj] == ' ') {
                continue; 
            }

            
            if (ni > 0 && nj > 0 && ni < map.size() - 1 && nj < map[0].size() - 1) {
               
                map[i + (ni - i) / 2][j + (nj - j) / 2] = ' ';
                map[ni][nj] = ' ';

                
                _maze(map, ni, nj);
            }
        }
    }
};

int main() {
    Solution s;
    int height = 11; // Must be odd 
    int width = 11;  // Must be odd 

    vector<char> row(width);
    vector<vector<char>> map(height, row);

    s.maze(map);     // Generate the maze
    s.showMaze(map); // Display the maze

    return 0;
}