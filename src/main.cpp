#include "game.h"
#include <cstdio>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>


const int grid_line_width = 1;
//const int screen_width = 600;
//const int screen_height = 600;
const int delay_time = 10;
const unsigned min_goal = 5;

int main() {

	const int grid_size = 10;
	std::vector<std::pair<int, double>> news;
//	news.push_back(std::make_pair(1, 0.7));
	news.push_back(std::make_pair(2, .8));
	news.push_back(std::make_pair(3, .2));

	// initialize sprites	
	BallSprites bs;
	// initialize grid
	Grid grid(grid_size, bs.ncolors);
	// initialize random
	Random rand;
	
	/* video init */
	const int screen_width = grid.size()*bs.sprite_width + (grid.size() + 1)*grid_line_width;
	const int screen_height = grid.size()*bs.sprite_height + (grid.size() + 1)*grid_line_width;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
	SDL_Surface *screen = SDL_GetWindowSurface(window);
	
	IMG_Init(IMG_INIT_PNG);
	
	// load sprites
	bs.load(screen->format);

	// insert 3 balls
	grid.put_new(rand);
	grid.put_new(rand);
	grid.put_new(rand);
	grid.put_new(rand);
	grid.put_new(rand);

	// draw grid
	draw_grid(screen, bs, grid, true);
	draw_all_balls(screen, bs ,grid);
	
	
	// main loop
	bool quit=false;
	int score = 0;
	
	bool jumping = false;
	int jumping_counter=0;
	const int jumping_speed = 3;
	bool moving = false;
	const int moving_speed=6;
	std::vector<std::pair<int, int>> path;
	int xa=0, ya=0; // animating coordinates
	while ( not quit ) {
		if (jumping) { 
			int frame = jumping_counter / jumping_speed;
			if (jumping_counter % jumping_speed == 0) { // redraw
				draw_ball(screen, bs, grid, xa, ya, frame);
			}
			jumping_counter++;
			if (jumping_counter >= jumping_speed * bs.nframes) jumping_counter = 0;
		}
		if (moving) {

			for (unsigned i=1; i<path.size(); i++) {
				int xe = std::get<0>(path[i]);
				int ye = std::get<1>(path[i]);
				for (int j=0; j<moving_speed; j++) {
					double offset = double(j)/moving_speed;
					double xoffset = 0;
					double yoffset = 0;

					if ( ye == ya ) { // moving in X direction
						if (xe > xa) { // moving right
							xoffset = offset;
						} else { // moving left
							xoffset = -offset;
						}
					} else { // moving in Y direction
						if (ye > ya) { // moving up
							yoffset = offset;
						} else { // moving down
							yoffset = -offset;
						}
					}

					draw_grid(screen, bs, grid, true);
					draw_path(path, screen, bs, grid, i-1, offset);
					draw_all_balls(screen, bs, grid, xa, ya);
					draw_ball(screen, bs, grid, xa, ya, 0, xoffset, yoffset);
					SDL_UpdateWindowSurface(window);
					SDL_Delay(delay_time/3);
				}
				grid.move(xa, ya, xe, ye);
				xa = xe;
				ya = ye;
			}				
			moving = false;
			
			// check for goals
			std::vector<std::pair<int,int>> goal = grid.find_goal();
			do {
				draw_grid(screen, bs, grid, true);
				draw_all_balls(screen, bs ,grid);
				SDL_UpdateWindowSurface(window);

				if (goal.size() > 0) { // goal found
					score += goal.size() + (goal.size()-min_goal) ;
					for (unsigned int i=0; i<goal.size(); i++) {
						grid.remove(std::get<0>(goal[i]), std::get<1>(goal[i]));
					}
					SDL_Delay(150);
					printf("Score: %d\n", score);
				} else {
					
					int nnew = new_count(news, rand);
					for (int i=0; i<nnew; i++) {
						if (grid.put_new(rand) == 0) {
							quit = true;
							printf("GAME OVER\n");
							break;
						}
					}
				}
				goal = grid.find_goal();
			} while (goal.size() > 0);
			
			draw_grid(screen, bs, grid, true);
			draw_all_balls(screen, bs ,grid);
			SDL_UpdateWindowSurface(window);


		}

		SDL_UpdateWindowSurface(window);
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0 and not moving) { 
			if (e.type==SDL_QUIT) {
				quit = true;
				break;
			} else if (e.type == SDL_MOUSEBUTTONDOWN) { // mouse clicked
				int xm, ym;
				SDL_GetMouseState(&xm, &ym);
				// check if clicked on existing ball
				int xc=-1, yc=-1;
				std::pair<bool, bool> b = clicked_on_ball(&xc, &yc, bs, grid, xm, ym);
				bool on_spot = std::get<0>(b);
				bool on_ball = std::get<1>(b);
				if (on_ball) {
					if (not jumping) { // start jumping
						jumping = true;
						jumping_counter = 0;
						xa = xc;
						ya = yc;
					} else { // is jumping
						if (xa == xc and ya == yc) { // clicked on jumping again
							jumping = false;
							draw_all_balls(screen, bs ,grid);
						}
					}
				} else if (jumping and on_spot and not on_ball) { // clicked on empty space during jumping
					
					// get path
					path = grid.find_path(xa, ya, xc, yc);
					// TODO
					// print path
					if (path.empty()) {
						//printf("no path from (%d, %d) to (%d, %d)!\n", xa, ya, xc, yc);
					} else {
						jumping=false;
						//printf("path from (%d, %d) to (%d, %d)\n", xa, ya, xc, yc);
						for (std::vector<std::pair<int,int>>::const_iterator iterator = path.begin(); iterator != path.end(); iterator++) {
							//printf("(%d, %d) -> ", std::get<0>(*iterator), std::get<1>(*iterator));
						}
						//printf("done\n");
						
						moving=true;
					}
				}
			}
		}
		SDL_Delay(delay_time);
	}

	SDL_Event e;
	while (SDL_PollEvent(&e)) SDL_Delay(10);

	
	/* video clean up */
	SDL_DestroyWindow(window);
	window = NULL;
	SDL_Quit();
	
}
