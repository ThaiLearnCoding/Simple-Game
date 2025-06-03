#include<SDL.h>
#include<SDL_ttf.h>
#include<iostream>
#include<stack>

using namespace std;

const int STANDARD_SIZE = 10; // Standard size for the maze
const int MIN_SIZE = 5; // Minimum size for the maze
const int MAX_SIZE = 20; // Maximum size for the maze
const int SCREEN_SIZE_EACH = 50;
const int THICK_NESS = 4;
int MAP_SIZE = STANDARD_SIZE;


class Point
{
public:
    bool u_avail; // Up
	bool d_avail; // Down
	bool l_avail; // Left
	bool r_avail; // Right
    bool connect; // Check loop

	Point(bool u = true, bool d = true, bool l = true, bool r = true)
		: u_avail(u), d_avail(d), l_avail(l), r_avail(r), connect(false) {}

    void connected(int dir)
    {
		switch (dir)
		{
			case 0: u_avail = false; break;
			case 1: d_avail = false; break;
			case 2: l_avail = false; break;
			case 3: r_avail = false; break;
		}
        connect = true;
    }

	// Change the connection of the forward neighbor
    void connectNext(int dir)
    {
        switch(dir)
        {
			case 0: d_avail = false; break;
			case 1: u_avail = false; break;
			case 2: r_avail = false; break;
			case 3: l_avail = false; break;
        }
        connect = true;
    }
};

class Maze
{
private:
    Point** map;      // Draw connection

    // Draw edges 
	// If true -> draw
    bool** VerEd; 
	bool** HorEd;

	pair<int, int> start; // Start point
	pair<int, int> end;   // End point
	pair<int, int> player; // Player position

public:
	Maze()
	{
		// Allocate map
		map = new Point*[MAP_SIZE];
		for (int idx = 0; idx < MAP_SIZE; idx++) {
			map[idx] = new Point[MAP_SIZE];
		}

		// Allocate VerEd
		VerEd = new bool * [MAP_SIZE];
		for (int idx = 0; idx < MAP_SIZE; idx++) {
			VerEd[idx] = new bool[MAP_SIZE - 1];
		}

		// Allocate HorEd
		HorEd = new bool* [MAP_SIZE - 1];
		for (int idx = 0; idx < MAP_SIZE - 1; idx++) {
			HorEd[idx] = new bool[MAP_SIZE];
		}

		// Initialize Points
		for (int i = 0; i < MAP_SIZE; i++)
		{
			map[i][0] = Point(true, true, false, true); // Leftest
			map[i][MAP_SIZE - 1] = Point(true, true, true, false); // Rightest
		}
		for (int j = 0; j < MAP_SIZE; j++)
		{
			map[0][j] = Point(false, true, true, true); // Top 
			map[MAP_SIZE - 1][j] = Point(true, false, true, true); // Bottom
		}
		for (int i = 1; i < MAP_SIZE - 1; i++)
		{
			for (int j = 1; j < MAP_SIZE - 1; j++)
			{
				map[i][j] = Point(); // Inner points
			}
		}


		// Initialize edges
		for (int i = 0; i < MAP_SIZE; i++)
		{
			for (int j = 0; j < MAP_SIZE - 1; j++)
			{
				VerEd[i][j] = true;
			}
		}

		for (int i = 0; i < MAP_SIZE - 1; i++)
		{
			for (int j = 0; j < MAP_SIZE; j++)
			{
				HorEd[i][j] = true;
			}
		}

		start = { 0, 0 };
		player = start;
	}

	~Maze() {

		// Delete map
		for (int i = 0; i < MAP_SIZE; i++) {
			delete[] map[i]; 
		}
		delete[] map; 

		// Delete VerEd
		for (int i = 0; i < MAP_SIZE; i++) {
			delete[] VerEd[i];
		}
		delete[] VerEd;

		// Delete HorEd
		for (int i = 0; i < MAP_SIZE - 1; i++) {
			delete[] HorEd[i];
		}
		delete[] HorEd;
	}

	void playerMove(int direction, SDL_Renderer* renderer)
	{
		int x = player.first;
		int y = player.second;
		bool moved = false;

		switch (direction)
		{
		case 0: // Up
			if (x > 0 && !map[x][y].u_avail) {
				player.first--;
				moved = true;
			}
			break;
		case 1: // Down
			if (x < MAP_SIZE - 1 && !map[x][y].d_avail) {
				player.first++;
				moved = true;
			}
			break;
		case 2: // Left
			if (y > 0 && !map[x][y].l_avail) {
				player.second--;
				moved = true;
			}
			break;
		case 3: // Right
			if (y < MAP_SIZE - 1 && !map[x][y].r_avail) {
				player.second++;
				moved = true;
			}
			break;
		}

		if (moved)
		{
			// Render the previous player position
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
			SDL_Rect prevPlayerRect = { y * SCREEN_SIZE_EACH + 10, x * SCREEN_SIZE_EACH + 10, SCREEN_SIZE_EACH - 20, SCREEN_SIZE_EACH - 20 };
			SDL_RenderFillRect(renderer, &prevPlayerRect);

			// Render the player at the new position
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue
			SDL_Rect playerRect = { player.second * SCREEN_SIZE_EACH + 10, player.first * SCREEN_SIZE_EACH + 10, SCREEN_SIZE_EACH - 20, SCREEN_SIZE_EACH - 20 };
			SDL_RenderFillRect(renderer, &playerRect); 
		}
		
	}

	bool gameComplete(SDL_Window* window) const
	{
		if (player == end) {
			return true;
		}
		return false;
	}

	int findAvail(int x, int y) const
	{
		int avail[4]{};
		for (int idx = 0; idx < 4; idx++)
		{
			avail[idx] = -1; // Initialize all directions as unavailable
		}
		int idx = 0;
		const Point& point = map[x][y]; // Get the current point
		if (point.u_avail && x > 0 && map[x - 1][y].connect == false)
			avail[idx++] = 0;
		if (point.d_avail && x < MAP_SIZE - 1 && map[x + 1][y].connect == false)
			avail[idx++] = 1;
		if (point.l_avail && y > 0 && map[x][y - 1].connect == false)
			avail[idx++] = 2;
		if (point.r_avail && y < MAP_SIZE - 1 && map[x][y + 1].connect == false)
			avail[idx++] = 3;

		// Here, idex equals to size
		if (idx == 0)
			return -1; // No available direction

		return avail[rand() % idx];
	}

	// Other methods for the Map class would go here
	void drawMaze()
	{
		stack<pair<int, int>> stack;
		stack.push(start);
		int max_path = 0;
		int curr_path = 0;
		end = start; // Initialize end point

		while (!stack.empty())
		{
			pair<int, int> current = stack.top();
			int x = current.first;
			int y = current.second;
			int direction = findAvail(x, y);

			// The end of the path, backtrack now
			if (direction == -1)
			{
				if (curr_path > max_path)
				{
					max_path = curr_path;
					
					// Update end point
					end.first = x; 
					end.second = y;
				}
				stack.pop();
				curr_path--;
				continue;
			}
			else
			{
				curr_path++;
				
				int x_next = x;
				int y_next = y;
				switch (direction)
				{
					// Up
					case 0: 
						x_next--; 
						HorEd[x_next][y_next] = false;
						break;	
					// Down
					case 1: 
						x_next++; 
						HorEd[x][y] = false;
						break;	
					// Left
					case 2: 
						y_next--; 
						VerEd[x_next][y_next] = false;
						break;	
					// Right
					case 3: 
						y_next++; 
						VerEd[x][y] = false;
						break;		
				}
				map[x_next][y_next].connectNext(direction);
				map[x][y].connected(direction);

				stack.push({ x_next, y_next });
			}
		}
	}

	void renderMap(SDL_Renderer* renderer)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set black for edges
		for (int i = 0; i < MAP_SIZE; i++)
		{
			for (int j = 0; j < MAP_SIZE - 1; j++)
			{
				if (VerEd[i][j])
				{
					// Draw vertical edge
					for (int thick = 0; thick < THICK_NESS; thick++)
						SDL_RenderDrawLine(renderer,(j + 1) * SCREEN_SIZE_EACH + thick , i * SCREEN_SIZE_EACH, (j + 1) * SCREEN_SIZE_EACH + thick, (i + 1) * SCREEN_SIZE_EACH);
				}
			}
		}

		for (int i = 0; i < MAP_SIZE - 1; i++)
		{
			for (int j = 0; j < MAP_SIZE; j++)
			{
				if (HorEd[i][j])
				{
					// Draw horizontal edge
					for (int thick = 0; thick < THICK_NESS; thick++)
						SDL_RenderDrawLine(renderer, j * SCREEN_SIZE_EACH, (i + 1) * SCREEN_SIZE_EACH + thick, (j + 1) * SCREEN_SIZE_EACH, (i + 1) * SCREEN_SIZE_EACH + thick);
				}
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
		SDL_Rect endRect = { end.second * SCREEN_SIZE_EACH + 10, end.first * SCREEN_SIZE_EACH + 10, SCREEN_SIZE_EACH - 20, SCREEN_SIZE_EACH - 20 };
		SDL_RenderFillRect(renderer, &endRect);

		player.first = start.first;
		player.second = start.second;
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue
		SDL_Rect playerRect = { player.second * SCREEN_SIZE_EACH + 10, player.first * SCREEN_SIZE_EACH + 10, SCREEN_SIZE_EACH - 20, SCREEN_SIZE_EACH - 20 };
		SDL_RenderFillRect(renderer, &playerRect);
	}
};

static int setupGame()
{
    cout << "Choosing options for playing game:\n";
    cout << "1. Standard (" << STANDARD_SIZE << "x" << STANDARD_SIZE << ")\n";
    cout << "2. Customized\n";
    cout << "Please typing the number of your choice:\n";
    int choice;     cin >> choice;

	while (choice != 1 && choice != 2) {
        cout << "Invalid choice. Please type again:\n";
        cin >> choice;
	}

    int size;
    if (choice == 1)
    {
		size = STANDARD_SIZE;
	}
    else
    {
		cout << "Please enter the size of the maze (between " << MIN_SIZE << " and " << MAX_SIZE << "):\n"; 
		cin >> size;
        while (size < MIN_SIZE || size > MAX_SIZE) {
            cout << "Invalid size. Please enter a size between " << MIN_SIZE << " and " << MAX_SIZE << ":\n";
            cin >> size;
        }
    }

    return size;
}

int main(int argc, char* argv[])
{
    /**
	* Setting up the game
    */
    MAP_SIZE = setupGame();
    srand(time(0));

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Error: SDL_Init\n";
		return -1;
	}
	if (TTF_Init() < 0) {
		std::cerr << "Error: TTF_Init\n";
		return -1;
	}

    SDL_Window* window = SDL_CreateWindow("Amaze SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAP_SIZE * SCREEN_SIZE_EACH, MAP_SIZE * SCREEN_SIZE_EACH, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("font.ttf", 48);
    if (!font) {
        std::cerr << "Error: font\n";
        return -1;
    }

    SDL_Event event;



    // Write code here
	Maze* maze = new Maze();
	
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);   // Set background color to white
	SDL_RenderClear(renderer); // Clear the screen

	maze->drawMaze(); // Generate the maze
	maze->renderMap(renderer); // Render the maze

	bool quit = false;

    while (!quit)
    {
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = true; // Exit the program
				break;
			}
			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_UP:
					maze->playerMove(0, renderer);
					break;
				case SDLK_DOWN:
					maze->playerMove(1, renderer);
					break;
				case SDLK_LEFT:
					maze->playerMove(2, renderer);
					break;
				case SDLK_RIGHT:
					maze->playerMove(3, renderer);
					break;
				}


			}
		}
		SDL_RenderPresent(renderer); // Update the screen				

		if (maze->gameComplete(window)) {
			cout << "Congratulations! You reached the end of the maze!\n";
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Maze Completed", "You have successfully navigated through the maze!", window);
			quit = true; // Exit the program if the game is complete
		}
    }

    /**
    * Close game
    */
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}