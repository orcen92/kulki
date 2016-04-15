#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <random>
#include <string>
#include <vector>
#include <utility>
#include <exception>

#include <list>



extern const int grid_line_width;
extern const unsigned min_goal;

class Random {
	private:
		std::mt19937 mt;
			
	public:
		Random  ();
		~Random();
		int get_int(int a);
		double unit();
};


class BallSprites {
	
	public:
		static const int sprite_width = 50;
		static const int sprite_height = 50;
		static const int ncolors=5;
		static const int nframes = 9;
		static const int RED=0, GREEN=1, BLUE=2, CYAN=3, YELLOW=4;
	
	private:

		std::vector<SDL_Surface*> sprites;
	public:
		BallSprites();
		~BallSprites();
		
		void load(const SDL_PixelFormat *fmt);
		
		SDL_Surface *get(int color, SDL_Rect *ballRect=NULL, int frame=0);
};


class Grid {
	
	public:
		static const int EMPTY = -1;	
		static const int SOURCE_EMPTY = 1;
		static const int DESTINATION_NOT_EMPTY = 2;
	
	private:
		int n;
		int **grid;
		int ncolors;
		
		typedef struct {
			int x;
			int y;
			int d;
		} queue_item;

		
	public:
		Grid(int nn, int nc);
		~Grid();
		
		void clear();
		
		int size() { return n; }

		bool is_full();
		bool is_empty(int x, int y) { return grid[x][y] == EMPTY; }
		int get(int x, int y) { return grid[x][y]; }
		int put_new(Random &r); // returns number of balls put (0 or 1)
		
		void move(int xa, int ya, int xb, int yb);
		void remove(int x, int y);
		
		std::vector<std::pair<int,int>> find_path(int xs, int ys, int xe, int ye);
		
		std::vector<std::pair<int,int>> find_goal(int xs=-1, int ys=-1);
};


void draw_path(std::vector<std::pair<int,int>> &path, SDL_Surface *screen, BallSprites &bs, Grid &g, int start=0, double offset=0);

void draw_grid(SDL_Surface *screen, BallSprites &bs, Grid &g, bool clear=false);

void draw_ball(SDL_Surface *screen, BallSprites &bs, Grid &g, int x, int y, int frame=0, double xoffset=0, double yoffset=0);

void draw_all_balls(SDL_Surface *screen, BallSprites &bs, Grid &g, int ex=-1, int ey=-1);

// first: on spot, second: on existing ball
std::pair<bool, bool> clicked_on_ball(int *x, int *y, BallSprites &bs, Grid &grid, int xm, int ym);


int new_count(std::vector<std::pair<int,double>> news, Random &rand);
