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

void vGenericTask(void *pvParameters){
    TaskData_t *task = (TaskData_t*) pvParameters;
    if(!task){ vTaskDelete(NULL); return; }

    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

void vSchedulerTask(void *pvParameters) {
    (void) pvParameters;
    TaskData_t currentTask;

    for(;;) {
        // --- 1. ADIM: ZAMANI GELEN GÖREVLERİ YARAT VE KUYRUĞA EKLE ---
        for(int i = 0; i < globalTaskCount; i++) {
            if(allTasks[i].arrivalTime == simTime && allTasks[i].handle == NULL) {
                // Görevi tam şu an yaratıyoruz
                BaseType_t xReturned = xTaskCreate(vGenericTask, 
                                                  allTasks[i].taskName, 
                                                  configMINIMAL_STACK_SIZE * 2, 
                                                  (void*)&allTasks[i], 
                                                  tskIDLE_PRIORITY + 1, 
                                                  &allTasks[i].handle);

                if(xReturned == pdPASS) {
                    QueueHandle_t q = getQueueByPriority(allTasks[i].priority);
                    if(q) {
                        allTasks[i].queuedTime = simTime; // Zaman aşımı sayacı şimdi başladı
                        xQueueSend(q, &allTasks[i], 0);
                    }
                }
            }
        }

        // --- 2. ADIM: ZAMAN AŞIMI KONTROLÜ ---
        for(int i = 0; i < globalTaskCount; i++) {
            if(allTasks[i].handle != NULL && allTasks[i].remainingTime > 0) {
                if(simTime - allTasks[i].queuedTime >= 20) {
                    printf("%s%d.0000 sn proses zaman aşımı    (id: %04d    öncelik: %d    kalan süre: %d sn)%s\n", 
                           allTasks[i].pcColorCode, simTime, atoi(allTasks[i].taskName), allTasks[i].priority, allTasks[i].remainingTime, ANSI_COLOR_RESET);
                    
                    vTaskDelete(allTasks[i].handle);
                    allTasks[i].handle = NULL; // Hayalet görev oluşmaması için handle'ı temizle
                }
            }
        }

        // --- 3. ADIM: KUYRUKTAN GÖREV SEÇİMİ VE ÇALIŞTIRMA ---
        TaskData_t *selectedTask = NULL;
        int selectedQueueIndex = -1;

        if      (xQueueReceive(xQueue0, &currentTask, 0) == pdTRUE) selectedQueueIndex = 0;
        else if (xQueueReceive(xQueue1, &currentTask, 0) == pdTRUE) selectedQueueIndex = 1;
        else if (xQueueReceive(xQueue2, &currentTask, 0) == pdTRUE) selectedQueueIndex = 2;
        else if (xQueueReceive(xQueue3, &currentTask, 0) == pdTRUE) selectedQueueIndex = 3;

        if (selectedQueueIndex != -1) {
            // Liste üzerinden gerçek pointer'a eriş
            for(int k = 0; k < globalTaskCount; k++) {
                if(atoi(allTasks[k].taskName) == atoi(currentTask.taskName)) {
                    selectedTask = &allTasks[k];
                    break;
                }
            }

            // Eğer görev zaman aşımına uğrayıp SİLİNMEDİYSE çalıştır
            if(selectedTask && selectedTask->handle != NULL) {
                if(selectedTask->remainingTime == selectedTask->burstTime) {
                    printf("%s%d.0000 sn proses başladı       (id: %04d    öncelik: %d    kalan süre: %d sn)%s\n", 
                           selectedTask->pcColorCode, simTime, atoi(selectedTask->taskName), selectedTask->priority, selectedTask->remainingTime, ANSI_COLOR_RESET);
                } else {
                    printf("%s%d.0000 sn proses devam         (id: %04d    öncelik: %d    kalan süre: %d sn)%s\n", 
                           selectedTask->pcColorCode, simTime, atoi(selectedTask->taskName), selectedTask->priority, selectedTask->remainingTime, ANSI_COLOR_RESET);
                }

                //vTaskDelay(pdMS_TO_TICKS(1000)); 
                selectedTask->remainingTime -= 1;
                selectedTask->queuedTime = simTime + 1; // Başarılı çalışma, 20sn hakkını yeniledi

                if(selectedTask->remainingTime <= 0) {
                    printf("%s%d.0000 sn proses sonlandı      (id: %04d    öncelik: %d    kalan süre: %d sn)%s\n", 
                           selectedTask->pcColorCode, simTime + 1, atoi(selectedTask->taskName), selectedTask->priority, 0, ANSI_COLOR_RESET);
                    vTaskDelete(selectedTask->handle);
                    selectedTask->handle = NULL;
                } else {
                    // Preemption/Feedback mantığı: Q0 değilse öncelik düşür
                    if(selectedQueueIndex != 0) {
                        int eskiOncelik = selectedTask->priority;
                        if(selectedTask->priority < 3) selectedTask->priority++;
                        
                        printf("%s%d.0000 sn proses askıya al     (id: %04d    öncelik: %d    kalan süre: %d sn)%s\n", 
                               selectedTask->pcColorCode, simTime + 1, atoi(selectedTask->taskName), eskiOncelik, selectedTask->remainingTime, ANSI_COLOR_RESET);
                    }
                    xQueueSend(getQueueByPriority(selectedTask->priority), selectedTask, 0);
                }
            } else {
                // Silinmiş görev kuyruktan gelirse pas geç ve döngüyü işletme (zaman artmasın)
                continue; 
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        simTime++;
    }
}


