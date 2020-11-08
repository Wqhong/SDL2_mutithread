#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "nbody.h"

int main(int argc, char** argv) {
	struct body **new_bodies = NULL;

  	struct get_position getPosition;
	double VIEW_WIDTH = atof(argv[1]);
    double VIEW_HEIGHT = atof(argv[2]);
    getPosition.iterate_times = atoi(argv[3]);
	getPosition.dt = atof(argv[4]);
    double scale = atof(argv[7]);

	if (strcmp(argv[5], "-f") == 0) {
		strcpy(getPosition.file_name, argv[6]);
		getPosition.length = calculate_number_bodies(&getPosition);
		new_bodies = malloc_memory_by_body_number(getPosition.length);
		// new_bodies = (struct body**)malloc(sizeof(struct body*)*getPosition.length); 
		// read_file内部已经申请内存，不用在申请，否则存在内存泄漏
		read_file(new_bodies, getPosition.length, &getPosition);
	} else if (strcmp(argv[5], "-b") == 0) {
		getPosition.length = atoi(argv[6]);
		new_bodies = malloc_memory_by_body_number(getPosition.length);
		build_bodies(new_bodies, &getPosition, VIEW_WIDTH, VIEW_HEIGHT);
	} else {
		printf("The third argv is invalid, please enter again");
	}

	// 申请内存空间，用来存放回访数据
	struct Pos** reBackData = (struct Pos**)malloc(sizeof(struct Pos*) * getPosition.length);
	for (int i = 0; i < getPosition.length; i++) {
		reBackData[i] = (struct Pos*)malloc(sizeof(struct Pos) * getPosition.iterate_times);
	}

	// 调用多线程函数执行
	int tNum = atoi(argv[8]);
	mutithread_alloc_tasks(new_bodies, reBackData, tNum, &getPosition);

	//Your SDL variables and variables for circle data and finished state
	int finished = 0;
	SDL_Event event;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("SDL Template", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		VIEW_WIDTH, VIEW_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	for(int t = 0; t < getPosition.iterate_times && (!finished); t++) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
		SDL_RenderClear(renderer);

	 	for(int i = 0; i<getPosition.length; i++){
			 if (i == 0) {
				filledCircleColor(renderer, new_bodies[0]->x, new_bodies[0]->y, 10, 0xFF0000FF);
				printf("....body%d........x: %lf\n", i, new_bodies[0]->x);
				printf("....body%d........y: %lf\n", i, new_bodies[0]->y);
			 } else {
				filledCircleColor(renderer, reBackData[i][t].x, reBackData[i][t].y, 10, 0xFF0000FF);
				printf("....body%d........x: %lf\n", i, reBackData[i][t].x);
				printf("....body%d........y: %lf\n", i, reBackData[i][t].y);
			 }
		}
		SDL_RenderPresent(renderer);

		if(SDL_PollEvent(&event)) {
			finished = (event.type == SDL_QUIT);
		}
	}
	//Clean up functions
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	for (int i = 0; i < getPosition.length; i++) {
		free(new_bodies[i]);
		free(reBackData[i]);
	}
	free(new_bodies);
	free(reBackData);
	printf("SDL Template over!\r\n");

	return 0;
}
