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

/* Renk Kodları (Eğer schedule.h içinde yoksa diye güvenlik amacıyla) */
#ifndef ANSI_COLOR_RESET
#define ANSI_COLOR_RESET   "\x1b[0m"
#endif

/* ---------------------------------------------------------------- */
/* KİŞİ 1'İN ALANI                                                  */
/* (SCHEDULER / ZAMANLAYICI)                                        */
/* ---------------------------------------------------------------- */
/* GÖREV: Hangi taskın çalışacağına karar veren 'vSchedulerTask'    */
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
            // Orijinal global dizideki (allTasks) veriyi bulup güncelleyeceğiz.
            for(int k = 0; k < globalTaskCount; k++) {
                if(allTasks[k].handle == currentTaskData.handle) {
                    selectedTaskPtr = &allTasks[k];
                    break;
                }
            }
        }

        if(selectedTaskPtr != NULL) {
            // 1. İşçi Göreve (GenericTask) "Uyan ve Çalış" sinyali gönder
            xTaskNotifyGive(selectedTaskPtr->handle);

            // 2. Bir Simülasyon Saniyesi (Tick) kadar bekle
            vTaskDelay(pdMS_TO_TICKS(1000));

            // 3. Görevin kalan süresini azalt
            selectedTaskPtr->remainingTime -= 1;

            // 4. Görev Bitti mi?
            if(selectedTaskPtr->remainingTime <= 0) {
                // Not: Bitiş mesajını vGenericTask içinde de bastırabiliriz,
                // ama burada kontrol etmek flow açısından daha güvenlidir.
                
                // Eğer Real-Time (Q0) görevi bittiyse, şimdi kuyruktan gerçekten silebiliriz.
                if(selectedQueueIndex == 0) {
                    TaskData_t dummy;
                    xQueueReceive(xQueue0, &dummy, 0);
                }
            } 
            else {
                // --- GÖREV BİTMEDİ (Feedback Mantığı) ---
                if(selectedQueueIndex == 0) {
                    // Real-Time (Q0): Kuyrukta kalmaya devam eder (Peek yapıldı)
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
                    // Q3 -> Q3 (Round Robin mantığı)
                    xQueueSend(xQueue3, selectedTaskPtr, 0);
                }
            }
        } 
        else {
            // Idle Durumu
            printf("[%d sn] CPU Bosta (Idle)\n", simTime);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        // Zamanı ilerlet
        simTime++;
    }
}


/* ---------------------------------------------------------------- */
/* KİŞİ 2'NİN ALANI                                                 */
/* (GENERIC TASK / İŞÇİ GÖREV)                                      */
/* ---------------------------------------------------------------- */
/* GÖREV: Ekrana çıktı basan, işi yapan ve süresi bitince kendini   */
/* silen 'vGenericTask' fonksiyonunu buraya yazacak.                */
/* ---------------------------------------------------------------- */

void vGenericTask(void *pvParameters) {
    /* * Task oluşturulurken parametre olarak kendi veri yapısının 
     * adresi (TaskData_t*) gönderildiği varsayılmıştır.
     */
    TaskData_t *pxParams = (TaskData_t *) pvParameters;

    if (pxParams == NULL) {
        printf("HATA: Task parametresi NULL!\n");
        vTaskDelete(NULL);
    }

    for(;;) {
        /* * 1. BEKLEME MODU (Blocking)
         * Scheduler "xTaskNotifyGive" diyene kadar burada uyur.
         * Bu sayede CPU'yu meşgul etmez.
         */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* * 2. İŞLEM MODU (Working)
         * Uyandığında çalışır ve ekrana basar.
         */
        if (pxParams->remainingTime > 0) {
            /* Renkli Çıktı Formatı */
            // Not: Scheduler zaten 1 sn delay ve süre azaltma yaptığı için
            // burada sadece durumu raporluyoruz.
            printf("%s%s calisiyor. Kalan Sure: %d sn%s\n", 
                   pxParams->pcColorCode, // schedule.h içinde tanımlı renk değişkeni
                   pxParams->taskName, 
                   pxParams->remainingTime, 
                   ANSI_COLOR_RESET);
        }
        
        /* * 3. BİTİŞ KONTROLÜ
         * Eğer süre 0 veya altına indiyse görev tamamlanmıştır.
         */
        if (pxParams->remainingTime <= 0) {
            printf("%s%s TAMAMLANDI ve task siliniyor.%s\n", 
                   pxParams->pcColorCode, 
                   pxParams->taskName, 
                   ANSI_COLOR_RESET);
            
            // Görevi bellekten tamamen sil
            vTaskDelete(NULL);
        }
        
        /* * ÖNEMLİ NOT: 
         * Buraya vTaskDelay(1000) KOYMUYORUZ. 
         * Çünkü Scheduler (Kişi 1) zaten bizim yerimize zamanı sayıyor.
         * Biz sadece "Uyan -> Yaz -> Uyu" döngüsündeyiz.
         */
    }
}