/* DOSYA: tasks.c
 * DURUM: GUNCEL GOREV DAGILIMINA UYGUN VE HATA DUZELTMELI
 * DİKKAT: Bu dosya projenin ana iskeletidir. 
 * Kişi 1 ve Kişi 2 kendi kodlarını buradaki ilgili blokların içine yazacak.
 */

/* ----------------- INCLUDE SIRALAMASI ÖNEMLİDİR ----------------- */
#include "FreeRTOS.h"   // 1. Önce Ana FreeRTOS kütüphanesi
#include "task.h"       // 2. Sonra Task kütüphanesi (TickType_t burada tanınır)
#include "queue.h"      // 3. Kuyruk fonksiyonları için
#include "semphr.h"     // 4. Mutex ve Semaforlar için

#include "schedule.h"   // 5. En son proje özel kütüphaneniz
#include <stdio.h>

/* ---------------------------------------------------------------- */
/* KİŞİ 1'İN ALANI                          */
/* (SCHEDULER / ZAMANLAYICI)                     */
/* ---------------------------------------------------------------- */
/* GÖREV: Hangi taskın çalışacağına karar veren 'vSchedulerTask'    */
/* fonksiyonunu buraya yazacak.                              */
/* ---------------------------------------------------------------- */

void vSchedulerTask(void *pvParameters) {
    (void) pvParameters;

    // Simülasyon zaman sayacı
    static int simTime = 0; 
    
    // Geçici değişkenler
    TaskData_t *selectedTaskPtr = NULL;
    TaskData_t currentTaskData;
    int selectedQueueIndex = -1;

    printf("Sistem: Scheduler Baslatiliyor...\n");

    for(;;) {
        /* ---------------------------------------------------------------
         * ADIM 1: ZAMANI GELEN GÖREVLERİ İLGİLİ KUYRUKLARA EKLE
         * --------------------------------------------------------------- */
        // globalTaskCount, schedule.c/h içinde tanımlı olmalı ve okunmuş olmalı
        for(int i = 0; i < globalTaskCount; i++) {
            if(allTasks[i].arrivalTime == simTime) {
                // Önceliğe göre doğru FreeRTOS kuyruğunu seç
                QueueHandle_t targetQueue = NULL;
                
                switch(allTasks[i].priority) {
                    case 0: targetQueue = xQueue0; break; // Real-Time
                    case 1: targetQueue = xQueue1; break; // High
                    case 2: targetQueue = xQueue2; break; // Medium
                    case 3: targetQueue = xQueue3; break; // Low
                }

                if(targetQueue != NULL) {
                    // Kuyruğa, task verisinin bir kopyasını gönderiyoruz
                    // portMAX_DELAY yerine 0 kullanıyoruz ki scheduler beklemesin
                    xQueueSend(targetQueue, &allTasks[i], 0);
                    printf("[%d sn] Yeni Gorev Eklendi: %s (P%d)\n", simTime, allTasks[i].taskName, allTasks[i].priority);
                }
            }
        }

        /* ---------------------------------------------------------------
         * ADIM 2: HANGİ KUYRUKTAN GÖREV ÇALIŞTIRILACAK?
         * Kural: Q0 (RealTime) > Q1 > Q2 > Q3
         * --------------------------------------------------------------- */
        selectedTaskPtr = NULL;
        selectedQueueIndex = -1;

        // -- Q0 KONTROL (Real-Time / FCFS / Non-Preemptive) --
        // Q0'daki görev bitene kadar kuyruktan SİLİNMESİN (Peek yapıyoruz)
        if(xQueuePeek(xQueue0, &currentTaskData, 0) == pdTRUE) {
            selectedQueueIndex = 0;
        }
        // -- Q1 KONTROL (High) --
        else if(xQueueReceive(xQueue1, &currentTaskData, 0) == pdTRUE) {
            selectedQueueIndex = 1;
        }
        // -- Q2 KONTROL (Medium) --
        else if(xQueueReceive(xQueue2, &currentTaskData, 0) == pdTRUE) {
            selectedQueueIndex = 2;
        }
        // -- Q3 KONTROL (Low) --
        else if(xQueueReceive(xQueue3, &currentTaskData, 0) == pdTRUE) {
            selectedQueueIndex = 3;
        }

        /* ---------------------------------------------------------------
         * ADIM 3: GÖREVİ ÇALIŞTIR, YÖNET VE GERİ BESLEME YAP
         * --------------------------------------------------------------- */
        if(selectedQueueIndex != -1) {
            // FreeRTOS Kuyruğundan aldığımız veri 'kopya'dır. 
            // Orijinal global dizideki (allTasks) veriyi bulup güncelleyeceğiz.
            // Bu sayede remainingTime vb. her yerde tutarlı kalır.
            for(int k = 0; k < globalTaskCount; k++) {
                // TaskHandle_t ile eşleşen orijinal görevi bul
                if(allTasks[k].handle == currentTaskData.handle) {
                    selectedTaskPtr = &allTasks[k];
                    break;
                }
            }
        }

        if(selectedTaskPtr != NULL) {
            // 1. İşçi Göreve (GenericTask) "Uyan ve Çalış" sinyali gönder
            // xTaskNotifyGive, hedef görevin bloğunu kaldırır.
            xTaskNotifyGive(selectedTaskPtr->handle);

            // 2. Bir Simülasyon Saniyesi (Tick) kadar bekle
            // Bu sırada vGenericTask çalışacak ve ekrana yazacak.
            vTaskDelay(pdMS_TO_TICKS(1000));

            // 3. Görevin kalan süresini azalt
            selectedTaskPtr->remainingTime -= 1;

            // 4. Görev Bitti mi?
            if(selectedTaskPtr->remainingTime <= 0) {
                printf("[%d sn] GOREV BITTI: %s\n", simTime+1, selectedTaskPtr->taskName);
                
                // Eğer Real-Time (Q0) görevi bittiyse, şimdi kuyruktan gerçekten silebiliriz.
                if(selectedQueueIndex == 0) {
                    TaskData_t dummy;
                    xQueueReceive(xQueue0, &dummy, 0);
                }
                // Kendini vGenericTask içinde silecek veya burada handle edilecek.
                // Proje formatına göre task tekrar kuyruğa girmez.
            } 
            else {
                // --- GÖREV BİTMEDİ (Feedback Mantığı) ---
                if(selectedQueueIndex == 0) {
                    // Real-Time (Q0): Kuyruktan çıkarmadık (Peek), o yüzden en başta duruyor.
                    // Bir şey yapmaya gerek yok, sonraki döngüde yine o seçilecek (FCFS & Non-Preemptive).
                }
                else if(selectedQueueIndex == 1) {
                    // Q1 -> Q2'ye düşür
                    selectedTaskPtr->priority = 2; 
                    xQueueSend(xQueue2, selectedTaskPtr, 0);
                }
                else if(selectedQueueIndex == 2) {
                    // Q2 -> Q3'e düşür
                    selectedTaskPtr->priority = 3; 
                    xQueueSend(xQueue3, selectedTaskPtr, 0);
                }
                else if(selectedQueueIndex == 3) {
                    // Q3 -> Q3 (Round Robin mantığı: Kuyruğun sonuna tekrar ekle)
                    xQueueSend(xQueue3, selectedTaskPtr, 0);
                }
            }
        } 
        else {
            // Hiçbir kuyrukta görev yoksa CPU boştadır (Idle)
            printf("[%d sn] CPU Bosta (Idle)\n", simTime);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        // Zamanı ilerlet
        simTime++;
    }
}


/* ---------------------------------------------------------------- */
/* KİŞİ 2'NİN ALANI                         */
/* (GENERIC TASK / İŞÇİ GÖREV)                   */
/* ---------------------------------------------------------------- */
/* GÖREV: Ekrana çıktı basan, işi yapan ve süresi bitince kendini   */
/* silen 'vGenericTask' fonksiyonunu buraya yazacak.         */
/* ---------------------------------------------------------------- */

void vGenericTask(void *pvParameters) {
    /* Parametreyi al (Task ismi vb. olabilir) */
    // char *pcTaskName = (char *) pvParameters; (Örnek kullanım)
    (void) pvParameters;

    for(;;) {
        // BURASI Kişi 2 TARAFINDAN DOLDURULACAK

        // Örnek (Dummy) Akış:
        // 1. Scheduler'dan emir bekle: ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // 2. Ekrana yaz: printf("Task Calisiyor\n");
        // 3. 1 sn bekle.
        // 4. Süre bittiyse kendini sil: vTaskDelete(NULL);

        printf("[Generic Task]: Emir bekliyorum... (Kişi 2 bekleniyor)\n");
        
        // Şimdilik kendini siliyoruz ki sonsuz döngüde ekranı kilitlemesin
        vTaskDelay(pdMS_TO_TICKS(1000)); 
        // vTaskDelete(NULL); // Gerçek kodda burada task silinecek veya askıya alınacak
    }
}