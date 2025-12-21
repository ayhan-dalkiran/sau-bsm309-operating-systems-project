#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define MAX_TASKS 100

typedef struct {
    char taskName[10];
    char* pcColorCode;
    int arrivalTime;
    int priority;
    int burstTime;
    int remainingTime;
    TaskHandle_t handle;
    int queuedTime; // Kuyruğa eklenme veya askıya alındığı zamanı tutar
} TaskData_t;

extern QueueHandle_t xQueue0; // Real-Time
extern QueueHandle_t xQueue1; // High
extern QueueHandle_t xQueue2; // Medium
extern QueueHandle_t xQueue3; // Low (RR)

extern TaskData_t allTasks[MAX_TASKS];
extern int globalTaskCount;

void loadTasksFromFile(const char *filename);
QueueHandle_t getQueueByPriority(int priority);
void decreaseTaskPriority(TaskData_t *task, int simTime);

#endif
