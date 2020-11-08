#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "nbody.h"

#define PI (3.141592653589793)
#define GCONST (6.67e-11)
#define MAX_LINE_SIZE 1000 // 改小一点，100或者500，数据太大容易导致栈空间溢出

double calculate_distance(struct body *body1, struct body *body2) 
{
    double dx = body1->x - body2->x;
    double dy = body1->y - body2->y;
    double dz = body1->z - body2->z;
    double distance = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
//    printf("d//%0.34f\n",distance);
    return distance;
}

double calculate_bodies_angular_velocity(double G, double M, double R)
{
    return sqrt(G * M / pow(R, 3));
}

double get_current_anglar(struct body* sun, struct body* bodyObj)
{
    double angular;
    double dx = bodyObj->x - sun->x;
    double dy = bodyObj->y - sun->y;
    double radius = sqrt(pow(dx, 2) + pow(dy, 2));
    if (dx > 0 && dy > 0) {
        angular = asin(dy / radius);
    }else if (dx < 0 && dy > 0) {
        angular = PI - asin(dy / radius);
    } else if (dx < 0 && dy < 0) {
        angular = -asin(dy / radius) + PI;
    } else {
        angular = 2 * PI + asin(dy / radius);
    }

    return angular;
}

void update_next_position(struct body* sun, struct body* bodyObj, double angular_velocity, double Ts)
{
    double radius = calculate_distance(bodyObj, sun);
    double curAngular = get_current_anglar(sun, bodyObj);
    double nextAngular = (curAngular + angular_velocity * Ts);

    bodyObj->x = sun->x + radius * cos(nextAngular);
    bodyObj->y = sun->y + radius * sin(nextAngular);    
}

void* RunBystep(void* pTask)
{
    struct task * pTask1 = (struct task *)pTask;
    int bodySize = pTask1->size;
    double dt = pTask1->dt;
    struct Pos** reBackData = pTask1->reBackData;
    struct body** bodies = pTask1->pBody;
    struct body* center = pTask1->center;

    for (int t = 0; t < pTask1->times; t++) {
        for (int i = 0; i < bodySize; i++) {
            double R = calculate_distance(bodies[i], center);
            double angular_velocity = calculate_bodies_angular_velocity(GCONST, center->mass, R);
            update_next_position(center, bodies[i], angular_velocity, dt);
            reBackData[i][t].x = bodies[i]->x;
            reBackData[i][t].y = bodies[i]->y;
        }
    }
    printf("one of thread is run over...\r\n");
}

void mutithread_alloc_tasks(struct body** bodies, struct Pos** reBackData, int tNum, struct get_position* para)
{
	printf("mutithread_alloc_tasks run for %d thread ...\r\n", tNum);

	void* thread_return;
	pthread_t* pthArr = (pthread_t *)malloc(sizeof(pthread_t) * tNum);
	struct task* task_id = (struct task*)malloc(sizeof(struct task) * tNum);
    int num = para->length / tNum;
    int rem = para->length % tNum;    
	for (int i = 0; i < tNum; i++) {
		task_id[i].dt = para->dt;
		task_id[i].center = bodies[0];
		task_id[i].times = para->iterate_times;
		if (i < rem) {
			task_id[i].size = num + 1;
			task_id[i].pBody = &bodies[i * (num + 1)];
			task_id[i].reBackData = &reBackData[i * (num + 1)];
		} else {
			task_id[i].size = num;
			task_id[i].pBody = &bodies[i * num + rem];
			task_id[i].reBackData = &reBackData[i * num + rem];
		}
		if (i == 0) {
			task_id[i].size -= 1;
			task_id[i].pBody = &bodies[1];
			task_id[i].reBackData = &reBackData[1];
		}
		int ret = pthread_create(&pthArr[i], NULL, RunBystep, (void *)&task_id[i]);
		printf("thread %d start! pid is %ld\r\n", i, pthArr[i]);
	}
	
	for (int i = 0; i < tNum; i++) {
		pthread_join(pthArr[i], &thread_return);
	}
	printf("mutithread_alloc_tasks is over ...\r\n");
}

// generate the position for the bodies
void build_bodies(struct body** bodies, struct get_position *getPosition, double VIEW_WIDTH, double VIEW_HEIGHT)
{
    srand(time(NULL));
    bodies[0]->x = VIEW_WIDTH / 2.0;
    bodies[0]->y = VIEW_HEIGHT / 2.0;
    bodies[0]->z = 0;
    bodies[0]->velocity_x = 0;
    bodies[0]->velocity_y = 0;
    bodies[0]->velocity_z = 0;
    bodies[0]->mass = (double) rand() / RAND_MAX * pow(10, 12);
    for (int i = 1; i < getPosition->length; i++) {
        // any requirement for - + for each value?
        bodies[i]->x = (double) rand() / RAND_MAX * VIEW_WIDTH;
        if (VIEW_WIDTH - bodies[i]->x < 20) {
            bodies[i]->x = VIEW_WIDTH - 20;
        } else if (bodies[i]->x < 20) {
            bodies[i]->x = bodies[i]->x + 20;
        }
        bodies[i]->y = (double) rand() / RAND_MAX * VIEW_HEIGHT;
        if (VIEW_HEIGHT - bodies[i]->y < 20) {
            bodies[i]->y = VIEW_HEIGHT - 20;
        } else if (bodies[i]->y < 20) {
            bodies[i]->y = bodies[i]->y + 20;
        }
        bodies[i]->z = 0;
        bodies[i]->velocity_x = (double) rand() / RAND_MAX * 2.0 - 1.0;
        bodies[i]->velocity_y = (double) rand() / RAND_MAX * 2.0 - 1.0;
        bodies[i]->velocity_z = (double) rand() / RAND_MAX * 2.0 - 1.0;
        bodies[i]->mass = (double) rand() / RAND_MAX * pow(10, 7);
    }
}

struct body** malloc_memory_by_body_number(size_t num)
{
    struct body** new_bodies = (struct body**)malloc(sizeof(struct body*) * num);
    for (int i = 0; i < num; i++) {
        new_bodies[i] = (struct body*)malloc(sizeof(struct body));
    }

    return new_bodies;
}

// get the number of bodies in the file
int calculate_number_bodies(struct get_position *getPosition) {
    // 怎么确定文件名在20字节以内呢，修改为直接使用传入的指针
    // char file_name[20];
    // strcpy(file_name, getPosition->file_name); 
    FILE *fp = fopen(getPosition->file_name, "r");
    if (!fp) {
        fprintf(stderr, "failed to open file for reading\n");
        return -1; // 返回-1；因为个数不为负数，小于0表示出错
    }

    int length = 0;
    char line[MAX_LINE_SIZE];
    while (fgets(line, MAX_LINE_SIZE, fp) != NULL) {
        length++;
    }
    fclose(fp);

    printf("record the length: %d\n", length);
    return length;
}

// bodies直接在GUI文件申请内存，传入参数，提高内存效率
void read_file(struct body** bodies, int body_num, struct get_position *getPosition) 
{   
    // char file_name[20];  
    // strcpy(file_name, getPosition->file_name); 这个操作有风险，getPosition->file_name长度大于20，就会出问题
    FILE *fp = fopen(getPosition->file_name, "r"); // 大于20，文件读错误
    if (!fp) {
        fprintf(stderr, "failed to open file for reading\n");
        exit(0);
    }

    int r = body_num, c = 7;
    double *position = (double *) malloc(r * c * sizeof(double));
    
    int k = 0;
    int i = 0;
    int length = 0;
    char line[MAX_LINE_SIZE];
    while (fgets(line, MAX_LINE_SIZE, fp) != NULL) {
        length++;
        char *token = strtok(line, ",");
        char *ptr;
        while (token != NULL) {
            double ret = strtod(token, &ptr);
            if (i > 6) {
                i = 0;
            }
            *(position + k * 7 + i) = ret;
            printf("here: %f", *(position + k * 7 + i));

            i++;
            token = strtok(NULL, ",");
        }
        printf("\n");

        k++;
    }
    fclose(fp);

    printf(".......................................\n");
    for (int i = 0; i < body_num; i++) {
        for (int j = 0; j < 7; j++) {
            printf("below: %f ", *(position + i * 7 + j));
        }
        printf("\n");
    }

    for (int i = 0; i < body_num; i++) {
        bodies[i]->x = *(position + i * 7 + 0);
        bodies[i]->y = *(position + i * 7 + 1);
        bodies[i]->z = *(position + i * 7 + 2);
        bodies[i]->velocity_x = *(position + i * 7 + 3);
        bodies[i]->velocity_y = *(position + i * 7 + 4);
        bodies[i]->velocity_z = *(position + i * 7 + 5);
        bodies[i]->mass = *(position + i * 7 + 6);
    }
    free(position);
}