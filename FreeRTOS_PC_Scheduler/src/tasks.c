/* DOSYA: tasks.c
 * DURUM: GUNCEL GOREV DAGILIMINA UYGUN VE HATA DUZELTMELI
 * DİKKAT: Bu dosya projenin ana iskeletidir. 
 * Kişi 1 ve Kişi 2 kendi kodlarını buradaki ilgili blokların içine entegre etmiştir.
 */

/* ----------------- INCLUDE SIRALAMASI ÖNEMLİDİR ----------------- */
#include "FreeRTOS.h"   // 1. Önce Ana FreeRTOS kütüphanesi
#include "task.h"       // 2. Sonra Task kütüphanesi
#include "queue.h"      // 3. Kuyruk fonksiyonları için
#include "semphr.h"     // 4. Mutex ve Semaforlar için

#include "schedule.h"   // 5. Proje özel kütüphanesi (TaskData_t vb. burada)
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "schedule.h"

#ifndef ANSI_COLOR_RESET
#define ANSI_COLOR_RESET "\x1b[0m"
#endif

int simTime = 0;

void vSchedulerTask(void *pvParameters) {
    (void) pvParameters;
    
    TaskData_t currentTask;

    for(;;) {
        // 1. Gelen görevleri kuyruğa ekle
        for(int i=0; i<globalTaskCount; i++){
            if(allTasks[i].arrivalTime == simTime){
                QueueHandle_t q = getQueueByPriority(allTasks[i].priority);
                if(q){
                    xQueueSend(q, &allTasks[i], 0);
                    allTasks[i].queuedTime = simTime; // Kuyruğa eklendiği zamanı kaydet
                    printf("%s%d.0000 sn proses başladı       (id:%s    öncelik:%d    kalan süre:%d sn)%s\n", allTasks[i].pcColorCode, simTime, allTasks[i].taskName, allTasks[i].priority, allTasks[i].remainingTime, ANSI_COLOR_RESET);
                }
            }
        }

        // 2. Kuyruklardan seçim (öncelik sırasına göre)
        TaskData_t *selectedTask = NULL;
        int selectedQueueIndex = -1;

        if(xQueuePeek(xQueue0, &currentTask, 0)==pdTRUE) selectedQueueIndex=0;
        else if(xQueueReceive(xQueue1, &currentTask, 0)==pdTRUE) selectedQueueIndex=1;
        else if(xQueueReceive(xQueue2, &currentTask, 0)==pdTRUE) selectedQueueIndex=2;
        else if(xQueueReceive(xQueue3, &currentTask, 0)==pdTRUE) selectedQueueIndex=3;

        // 3. Seçilen task pointer bul
        if(selectedQueueIndex != -1){
            for(int k=0;k<globalTaskCount;k++){
                if(allTasks[k].handle == currentTask.handle){
                    selectedTask = &allTasks[k];
                    break;
                }
            }

            if(selectedTask){
                selectedTask->queuedTime = simTime; // Çalıştırıldığı an, drop timer sıfırlanır
                xTaskNotifyGive(selectedTask->handle);
                vTaskDelay(pdMS_TO_TICKS(1000));
                selectedTask->remainingTime -= 1;

                if(selectedTask->remainingTime <= 0){
                    if(selectedQueueIndex==0){ TaskData_t dummy; xQueueReceive(xQueue0, &dummy, 0); }
                } else {
                    if(selectedQueueIndex != 0) decreaseTaskPriority(selectedTask, simTime);
                }
            }
        } else {
            //printf("[%d sn] CPU Bosta (Idle)\n", simTime);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        // 4. Kuyrukta 20sn bekleyen taskları drop et
        for(int i=0;i<globalTaskCount;i++){
            TaskData_t task = allTasks[i];

            if(task.handle && task.remainingTime>0 &&
               task.queuedTime >=0 &&
               simTime - task.queuedTime >= 20){
                printf("%s%d.0000 sn proses zamanaşımı    (id:%s    öncelik:%d    kalan süre:%d sn)%s\n", task.pcColorCode, simTime, task.taskName, task.priority, task.remainingTime, ANSI_COLOR_RESET);
                vTaskDelete(task.handle);
                allTasks[i].handle = NULL;
            }
        }

        simTime++;
    }
}

void vGenericTask(void *pvParameters){
    TaskData_t *task = (TaskData_t*) pvParameters;
    if(!task){ vTaskDelete(NULL); return; }

    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if(task->remainingTime>0){
            printf("%s%d.0000 sn proses yürütülüyor   (id:%s    öncelik:%d    kalan süre:%d sn)%s\n", task->pcColorCode, simTime, task->taskName, task->priority, task->remainingTime, ANSI_COLOR_RESET);
        }

        if(task->remainingTime<=0){
            printf("%s%d.0000 sn proses sonlandı      (id:%s    öncelik:%d    kalan süre:%d sn)%s\n", task->pcColorCode, simTime, task->taskName, task->priority, task->remainingTime, ANSI_COLOR_RESET);
            vTaskDelete(NULL);
        }
    }
}
