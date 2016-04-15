#include "game.h"
#include <cmath>
#include <ctime>



Random::Random() {
	mt = std::mt19937(time(0));
}

Random::~Random() { }

int Random::get_int(int a) {
	std::uniform_int_distribution<int> d(0,a-1);
	return d(mt);
}

double Random::unit() {
	std::uniform_real_distribution<double> d(0,1);
	return d(mt);
}




BallSprites::BallSprites() {

}


void BallSprites::load(const SDL_PixelFormat *fmt) {
	const char dir[] = "sprites";
	const char files[][32]={"red", "green", "blue", "cyan", "yellow"};
	const char ext[] = "png";
	
	SDL_PixelFormat myfmt = *fmt;
	
	for (int i=0; i<ncolors; i++) {
		std::string name = std::string(dir) + std::string("/") + std::string(files[i]) + std::string(".") + std::string(ext);
		SDL_Surface *raw = IMG_Load(name.c_str());
		SDL_Surface *opt = SDL_ConvertSurface(raw, fmt, 0);				
		sprites.push_back(opt);
		SDL_FreeSurface(raw);
	}
}


BallSprites::~BallSprites() {
	for (int i=0; i<ncolors; i++) {
		SDL_FreeSurface(sprites[i]);
	}

}
	
SDL_Surface* BallSprites::get(int color, SDL_Rect *ballRect, int frame) {
	frame = (frame+4)%nframes;
	if (ballRect != NULL) {
		ballRect->x = sprite_width*frame;
		ballRect->y=0;
		ballRect->w = sprite_width;
		ballRect->h = sprite_height;
	}
	
	return sprites[color];
	
}





Grid::Grid(int nn, int nc) {
	n = nn;
	ncolors = nc;
	
	grid = new int*[n];
	for (int i=0; i<n; i++) {
		grid[i]=new int[n];
		for (int j=0; j<n; j++) {
			grid[i][j]=EMPTY;
		}
	}
}

Grid::~Grid() {
	for (int i=0; i<n; i++) {
		delete[] grid[i];
	}
	delete[] grid;
}
	
void Grid::clear() {
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++) {
			grid[i][j]=0;
		}
	}
}

bool Grid::is_full() {
	int k = 0;
	for (int i=0; i<n; i++) {
		for (int j=0; j<n; j++) {
			k+= (grid[i][j]!=EMPTY) ;
		}
	}
	return k==n*n;
}

int Grid::put_new(Random &r) {
	if (is_full()) return 0;
	
	int x,y;
	bool is_empty = false;
	do {
		x = r.get_int(n);
		y = r.get_int(n);
		if (grid[x][y] == EMPTY) {
			is_empty = true;
		}
	} while (is_empty == false);
	
	int color = r.get_int(ncolors);
	
	grid[x][y] = color;
	return 1;
}




std::vector<std::pair<int,int>> Grid::find_path(int xs, int ys, int xe, int ye) {
//	if (is_empty(xs, ys)) throw SOURCE_EMPTY;
//	if (not is_empty(xs, ys)) throw DESTINATION_EMPTY;
	
	// create a queue for checking points
	std::vector<queue_item> queue;

	// start from endpoint
	queue.push_back({xe, ye, 0});
	unsigned idx=0;
	
	bool found=false;
	int nadded=1;
	//printf("creating queue\n");
	while( not found and idx < queue.size()) {
		nadded = 0;
		// take a point from queue
		queue_item cur = queue[idx];
		//printf("neighbours of point (%d, %d):\n", cur.x, cur.y);
		// add neighbours to temporal queue
		std::vector<queue_item> tmp;
		tmp.push_back({cur.x-1, cur.y, cur.d+1});
		tmp.push_back({cur.x+1, cur.y, cur.d+1});
		tmp.push_back({cur.x, cur.y-1, cur.d+1});
		tmp.push_back({cur.x, cur.y+1, cur.d+1});
		
		// check items
		for (size_t i=0; i<tmp.size() and not found; i++) {
			queue_item item = tmp[i];
			
			// check if it is answer
			if (item.x == xs and item.y == ys) {
				found=true;
			} else {
				// check for out-of-bounds
				if (item.x < 0 or item.x >= n or item.y < 0 or item.y >= n) continue;
				// check for empty space
				if (not is_empty(item.x, item.y)) continue;
				// check if there is point with this coordinates but with lower distance
				bool is_nearer = false;
				for (unsigned j=0; j<queue.size() and is_nearer == false; j++) {
					queue_item qi = queue[j];
					if (qi.x == item.x and qi.y == item.y and qi.d <= item.d) {
						is_nearer = true;
					}
				}
				if (is_nearer) continue;
			}			
			
			// add to main queue
			queue.push_back(item);
			//printf("\t(%d, %d)\n", item.x, item.y);
			nadded++;
		}
		idx++;
	}
	if (nadded == 0) { // no available path
		return std::vector<std::pair<int,int>>();
	}
	
	//printf("full queue:\n");
	for (unsigned i=0; i<queue.size(); i++) {
		//printf("\t(%d, %d, %d)\n", queue[i].x, queue[i].y, queue[i].d);
	}
	
	// ready queue with distances from end to start, last element being start
	
	std::vector<std::pair<int,int>> path;

	//printf("\nfinding path\n");
	
	
	// find path
	idx=queue.size() - 1; // start from start which is always last in queue
	path.push_back(std::make_pair(xs, ys));
	do { 
		//printf("\tstart (%d, %d), current (%d, %d)\n", xs, ys, queue[idx].x, queue[idx].y);
		
		queue_item cur = queue[idx];
		// find neighbour with lowest distance
		int imin=-1;
		int dmin=0;
		for (int i=idx-1; i>=0; i--) {
			if ( (queue[i].x == cur.x-1 and queue[i].y == cur.y) or
			     (queue[i].x == cur.x+1 and queue[i].y == cur.y) or
			     (queue[i].x == cur.x and queue[i].y == cur.y-1) or
			     (queue[i].x == cur.x and queue[i].y == cur.y+1)) {
				if ( imin<0 or queue[i].d < dmin) {
					imin = i;
					dmin = queue[i].d;
				}
			}
		}
		//printf("\tnearest point from (%d, %d)[%d] is (%d, %d)[%d]\n", cur.x, cur.y, idx, queue[imin].x, queue[imin].y, imin);
		// next item is that with lowest distance from current
		idx = imin;
		// add that point to path
		path.push_back(std::make_pair(queue[idx].x, queue[idx].y));
		
	} while(not (queue[idx].x == xe and queue[idx].y == ye)); // until end is reached
	return path;
}

void Grid::remove(int x, int y) {
	grid[x][y] = EMPTY;
}


void Grid::move(int xa, int ya, int xb, int yb) {
	if (is_empty(xb, yb) and not is_empty(xa, ya)) {
		grid[xb][yb] = grid[xa][ya];
		grid[xa][ya] = EMPTY;
	}
}

std::vector<std::pair<int,int>> Grid::find_goal(int xs, int ys) {
	// for every ball, check if there is more than 4 neighbours in any direction
	std::vector<std::pair<int,int>> goal;
	
	int x0 = 0, y0 = 0, x1 = n, y1 = n;
	if (xs>=0 and ys>=0) {
		x0 = xs;
		x1 = xs+1;
		y0 = ys;
		y1 = ys+1;
	}
	
	int ndirs = 8;
	int dirs[][2] = { {1,0}, {-1,0}, {0,1}, {0, -1}, {1,1}, {-1,1}, {1,-1}, {-1,-1}};
	for (int x = x0; x<x1; x++) {
		for (int y = y0; y<y1; y++) {
			//printf("%d %d\n", x,y);
			if (not is_empty(x,y)) {
				int color = grid[x][y];
				for (int k=0; k<ndirs; k++) {
					goal.clear();
					int dx = dirs[k][0], dy = dirs[k][1];
					for (int xn=x, yn=y; xn>=0 and yn>=0 and xn<n and yn<n and grid[xn][yn] == color; xn+=dx, yn+=dy) {
						goal.push_back(std::make_pair(xn, yn));
					}
					if (goal.size() >= min_goal) return goal;
				}
			}
		}
	}
	goal.clear();
	return goal;
}


void draw_path(std::vector<std::pair<int,int>> &path, SDL_Surface *screen, BallSprites &bs, Grid &g, int start, double offset) {
	for (unsigned i=start; i<path.size()-1; ++i) {
		int xf = std::get<0>(path[i]);
		int yf = std::get<1>(path[i]);
		int xt = std::get<0>(path[i+1]);
		int yt = std::get<1>(path[i+1]);
		
		SDL_Rect rect;
		if (xf == xt) {
			double yf2 = yf;
			if (i == start)
				yf2 = yf + offset*(yt-yf);
			rect.x = grid_line_width + xf*(bs.sprite_width + grid_line_width) + bs.sprite_width/2 - grid_line_width;
			double y1 = std::min(double(yt), yf2);
			rect.y = grid_line_width + y1*(bs.sprite_width + grid_line_width) + bs.sprite_height/2 - 1;
			rect.h = bs.sprite_width*abs( (yt-yf2)/(yt-yf) ) + grid_line_width;
			rect.w = 1;
		} else if (yf == yt) {
			double xf2 = xf;
			if (i==start)
				xf2 = xf + offset*(xt-xf);
			rect.y = grid_line_width + yf*(bs.sprite_width + grid_line_width) + bs.sprite_height/2 - grid_line_width;
			double x1 = std::min(double(xt), xf2);
			rect.x = grid_line_width + x1*(bs.sprite_width + grid_line_width) + bs.sprite_width/2 - 1;
			rect.w = bs.sprite_width*abs( (xt-xf2)/(xt-xf) ) + grid_line_width;
			rect.h = 1;
		}
//		printf("%d %d %d %d\n", rect.x, rect.y, rect.w, rect.h);
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));
						
	}
}



void draw_grid(SDL_Surface *screen, BallSprites &bs, Grid &grid, bool clear) {
	const int grid_size = grid.size();
	
	Uint32 black, white;
	black = SDL_MapRGB(screen->format, 0,0,0);
	white = SDL_MapRGB(screen->format, 255, 255, 255);
	
	// clear screen
	if (clear) {
		SDL_FillRect(screen, NULL, white);
	}
		
	for (int i=0; i<grid_size+1; i++) {
		SDL_Rect rect;
		// draw horizontal lines
		rect.x = 0;
		rect.y = i*(bs.sprite_width + grid_line_width);
		rect.w = screen->w;
		rect.h = grid_line_width;
		SDL_FillRect(screen, &rect, black);

		// draw vertical lines
		rect.x = i*(bs.sprite_width + grid_line_width);
		rect.y = grid_line_width;
		rect.w = grid_line_width;
		rect.h = screen->h - grid_line_width*2;
		SDL_FillRect(screen, &rect, black);
	}
	
}

void draw_all_balls(SDL_Surface *screen, BallSprites &bs, Grid &grid, int ex, int ey) {
	const int grid_size = grid.size();
	// draw balls
	for (int x=0; x<grid_size; x++) {
		for (int y=0; y<grid_size; y++) {
			if ( not grid.is_empty(x,y) and not (ex==x and ey==y)) {
				draw_ball(screen, bs, grid, x, y, 0, 0, 0);
			}
		}
	}
}


void draw_ball(SDL_Surface *screen, BallSprites &bs, Grid &grid, int x, int y, int frame, double xoffset, double yoffset) { 
	if (grid.is_empty(x,y)) return;
	int color = grid.get(x,y);
	SDL_Rect srcRect;
	SDL_Rect dstRect;
	
	int xo = xoffset*(bs.sprite_width + grid_line_width);
	int yo = yoffset*(bs.sprite_width + grid_line_width);
	
	// get sprite and rect
	SDL_Surface *sprite = bs.get(color, &srcRect, frame);
	// compute destination rectangle
	dstRect.x = grid_line_width + ( bs.sprite_width  + grid_line_width) * x + xo;
	dstRect.y = grid_line_width + ( bs.sprite_height + grid_line_width) * y + yo;
	dstRect.w = bs.sprite_width;
	dstRect.h = bs.sprite_height;
	
	// clear rect
	Uint32 white = SDL_MapRGB(screen->format, 255, 255, 255);
	SDL_FillRect(screen, &dstRect, white);
	// draw
	SDL_BlitSurface(sprite, &srcRect, screen, &dstRect);

}


std::pair<bool, bool> clicked_on_ball(int *x, int *y, BallSprites &bs, Grid &grid, int xm, int ym) {
	
	// subtract initial width
	xm -= grid_line_width;
	ym -= grid_line_width;
	
	if (xm % (bs.sprite_width + grid_line_width) < bs.sprite_width and ym % (bs.sprite_height + grid_line_width) < bs.sprite_height) {
		*x = xm/(bs.sprite_width + grid_line_width);
		*y = ym/(bs.sprite_height + grid_line_width);
		return std::make_pair(true, not grid.is_empty(*x,*y));
	} else {
		return std::make_pair(false, false);
	}

}




int new_count(std::vector<std::pair<int,double>> news, Random &rand) {
	double x = rand.unit();

	double sum = 0;
	for (unsigned i=0; i<news.size(); i++) {
		int n = std::get<0>(news[i]);
		double prob = std::get<1>(news[i]);
		sum += prob;
	//	printf("%d %f %f\n", n, sum, x);
		if (x<sum) { return n; }
	}
	return 1;
}
