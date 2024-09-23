#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <cstdlib>
#include <ctime>

void generate_maze(int width, int height, std::vector<std::vector<char> >& maze) {
    // Initialize the maze
    maze = std::vector<std::vector<char> >(height, std::vector<char>(width, '*'));

    // Start position
    std::stack<std::pair<int, int> > stack;
    stack.push(std::make_pair(2, 2));
    maze[2][2] = ' ';

    //random seed
    std::srand(std::time(NULL));

    while (!stack.empty()) {
        int x = stack.top().first;
        int y = stack.top().second;
        std::vector<std::pair<int, int> > neighbors;

        // Check for valid cells (moving 2 cells at a time)
        std::vector<std::pair<int, int> > directions;
        directions.push_back(std::make_pair(-2, 0));
        directions.push_back(std::make_pair(2, 0));
        directions.push_back(std::make_pair(0, -2));
        directions.push_back(std::make_pair(0, 2));

        for (size_t i = 0; i < directions.size(); ++i) {
            int nx = x + directions[i].first;
            int ny = y + directions[i].second;

            if (nx >= 2 && nx < width - 2 && ny >= 2 && ny < height - 2) {
                if (maze[ny][nx] == '*') {
                    neighbors.push_back(std::make_pair(nx, ny));
                }
            }
        }

        if (!neighbors.empty()) {
            // Choose a random cell to create a path
            int rand_index = std::rand() % neighbors.size();
            int nx = neighbors[rand_index].first;
            int ny = neighbors[rand_index].second;

            // Carve a wider path by clearing multiple cells
            int mid_x = (x + nx) / 2;
            int mid_y = (y + ny) / 2;

            maze[mid_y][mid_x] = ' ';
            maze[ny][nx] = ' ';
            maze[y][mid_x] = ' ';
            maze[mid_y][x] = ' ';

            stack.push(std::make_pair(nx, ny));
        } else {
            stack.pop();
        }
    }
}

void print_maze(const std::vector<std::vector<char> >& maze) {
    for (size_t i = 0; i < maze.size(); ++i) {
        for (size_t j = 0; j < maze[i].size(); ++j) {
            std::cout << maze[i][j];
        }
        std::cout << std::endl;
    }
}

int main() {
    // Maze dimensions (should be odd numbers)
    int width = 41;
    int height = 21;

    std::vector<std::vector<char> > maze;

    generate_maze(width, height, maze);

    print_maze(maze);

    return 0;
}