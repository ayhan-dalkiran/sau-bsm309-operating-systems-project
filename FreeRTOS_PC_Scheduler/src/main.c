/*
 * DOSYA: main.c
 * SORUMLU: Kişi 4 (Entegrasyon & Config)
 * AÇIKLAMA: Sistemi başlatan, kuyrukları ve görevleri oluşturan ana dosya.
 */
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "schedule.h"

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_YELLOW  "\x1b[33m"

SemaphoreHandle_t xPrintMutex;

TaskData_t allTasks[MAX_TASKS];
int globalTaskCount = 0;

QueueHandle_t xQueue0, xQueue1, xQueue2, xQueue3 , xQueue4, xQueue5, xQueue6;

extern void vSchedulerTask(void *pvParameters);
extern void vGenericTask(void *pvParameters);

int main(void) {
    xPrintMutex = xSemaphoreCreateMutex();

    if(xPrintMutex) {
        xSemaphoreTake(xPrintMutex, portMAX_DELAY);
        printf("--- SISTEM BASLATILIYOR ---\n");
        xSemaphoreGive(xPrintMutex);
    }

    // main.c içindeki bu kısmı şu hale getirin:
    loadTasksFromFile("giris.txt");

    // KUYRUKLARI OLUŞTUR (Burası aynı kalacak)
    xQueue0 = xQueueCreate(10, sizeof(TaskData_t));
    xQueue1 = xQueueCreate(10, sizeof(TaskData_t));
    xQueue2 = xQueueCreate(10, sizeof(TaskData_t));
    xQueue3 = xQueueCreate(10, sizeof(TaskData_t));
    xQueue4 = xQueueCreate(10, sizeof(TaskData_t));
    xQueue5 = xQueueCreate(10, sizeof(TaskData_t));
    xQueue6 = xQueueCreate(10, sizeof(TaskData_t));

    // Renkleri önceden atayabilirsiniz ama xTaskCreate'i buradan kaldırın:
    for(int i=0; i<globalTaskCount; i++) {
        char* color;
        switch(i%4){
            case 0: color=COLOR_RED; break;
            case 1: color=COLOR_GREEN; break;
            case 2: color=COLOR_BLUE; break;
            case 3: color=COLOR_YELLOW; break;
        }
        allTasks[i].pcColorCode = color;
        allTasks[i].handle = NULL; // Henüz yaratılmadığını işaretle
    }

    // Sadece Scheduler'ı başlatın
    xTaskCreate(vSchedulerTask, "Scheduler", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 2, NULL);
    vTaskStartScheduler();

    for(int i=0; i<globalTaskCount; i++) {
        char* color;
        switch(i%4){
            case 0: color=COLOR_RED; break;
            case 1: color=COLOR_GREEN; break;
            case 2: color=COLOR_BLUE; break;
            case 3: color=COLOR_YELLOW; break;
        }
        allTasks[i].pcColorCode = color;

        xTaskCreate(vGenericTask, allTasks[i].taskName, configMINIMAL_STACK_SIZE*2, (void*)&allTasks[i], tskIDLE_PRIORITY+1, &allTasks[i].handle);
    }

    printf("Main: Tum görevler olusturuldu. Scheduler başlatılıyor...\n");
    vTaskStartScheduler();

    for(;;);
    return 0;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask; (void)pcTaskName;
    fprintf(stderr, "[HATA] Stack Overflow: %s\n", pcTaskName);
    for(;;);
}

void vApplicationMallocFailedHook(void) {
    fprintf(stderr, "[HATA] Memory Allocation Failed!\n");
    for(;;);
}
