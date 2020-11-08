#ifndef NBODY_H
#define NBODY_H

struct body {
	double x;
	double y;
	double z;
	double velocity_x;
	double velocity_y;
	double velocity_z;
	double mass;
};

struct get_position {
    int length;
    char file_name[20];
    double dt;
    double unit_vector[3];
    int iterate_times;
    double total_energy;
};

struct Pos {
	double x;
	double y;
};

struct task {
	int size;
	double dt;
	int times;
	struct body** pBody;
	struct body* center;
	struct Pos** reBackData;
};

void read_file(struct body** bodies, int body_num, struct get_position *getPosition);
int calculate_number_bodies(struct get_position *getPosition);
struct body** malloc_memory_by_body_number(size_t num);
void* RunBystep(void* pTask);
void build_bodies(struct body** bodies, struct get_position *getPosition, double VIEW_WIDTH, double VIEW_HEIGHT);
void mutithread_alloc_tasks(struct body** bodies, struct Pos** reBackData, int tNum, struct get_position* para);
#endif
