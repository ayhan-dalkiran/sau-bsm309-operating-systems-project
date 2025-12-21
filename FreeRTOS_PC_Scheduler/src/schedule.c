#include <stdio.h>
#include <stdlib.h>
#include "schedule.h"

#ifndef ANSI_COLOR_RESET
#define ANSI_COLOR_RESET "\x1b[0m"
#endif

void loadTasksFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if(!file){ perror("Dosya açılamadı"); exit(1); }

    while(fscanf(file, "%d, %d, %d\n",
                 &allTasks[globalTaskCount].arrivalTime,
                 &allTasks[globalTaskCount].priority,
                 &allTasks[globalTaskCount].burstTime) != EOF) {
        snprintf(allTasks[globalTaskCount].taskName, sizeof(allTasks[globalTaskCount].taskName), "%d", globalTaskCount);
        allTasks[globalTaskCount].remainingTime = allTasks[globalTaskCount].burstTime;
        allTasks[globalTaskCount].handle = NULL;
        allTasks[globalTaskCount].queuedTime = -1;
        globalTaskCount++;
        if(globalTaskCount >= MAX_TASKS) break;
    }
    fclose(file);
    printf("Toplam %d görev yüklendi.\n", globalTaskCount);
}

QueueHandle_t getQueueByPriority(int priority) {
    switch(priority) {
        case 0: return xQueue0;
        case 1: return xQueue1;
        case 2: return xQueue2;
        case 3: return xQueue3;
        case 4: return xQueue4;
        case 5: return xQueue5;
        case 6: return xQueue6;
        default: return NULL;
    }
}

// Decrease priority ve yeniden kuyruk ekleme
void decreaseTaskPriority(TaskData_t *task, int simTime) {
    task->priority += 1;
    QueueHandle_t q = getQueueByPriority(task->priority);
    if(q) {
        task->queuedTime = simTime; // Kuyruğa yeniden eklendi, 20sn drop timer sıfırlandı
        xQueueSend(q, task, 0);
        printf("%s%d.0000 sn Task%d proses askıda        (id: %04d    öncelik: %d    kalan süre: %d sn)%s\n", task->pcColorCode, simTime, atoi(task->taskName) + 1, atoi(task->taskName), task->priority, task->remainingTime, ANSI_COLOR_RESET);
    }
}
