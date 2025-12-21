/*
 * DOSYA: main.c
 * SORUMLU: Kişi 4 (Entegrasyon & Config)
 * AÇIKLAMA: Sistemi başlatan, kuyrukları ve görevleri oluşturan ana dosya.
 */

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "schedule.h"

/* -----------------------------------------------------------
 * GLOBAL TANIMLAMALAR
 * ----------------------------------------------------------- */
// Printf çakışmasını önlemek için Mutex kilidi
SemaphoreHandle_t xPrintMutex;

// Diğer dosyalarda yazılan Task fonksiyonlarını burada bildiriyoruz
extern void vSchedulerTask(void *pvParameters);
extern void vGenericTask(void *pvParameters);

/* -----------------------------------------------------------
 * MAIN FONKSİYONU
 * ----------------------------------------------------------- */
int main(void)
{
    // 1. Konsol çıktısı için Mutex oluştur
    xPrintMutex = xSemaphoreCreateMutex();

    if(xPrintMutex != NULL) {
        xSemaphoreTake(xPrintMutex, portMAX_DELAY);
        printf("--- SISTEM BASLATILIYOR ---\n");
        xSemaphoreGive(xPrintMutex);
    }

    // 2. Dosyadan Görevleri Oku (Kişi 3'ün yazdığı fonksiyon)
    loadTasksFromFile("giris.txt");

    // 3. Kuyrukları Oluştur (Her biri 10 adet TaskData_t taşıyabilir)
    // Kuyruklar schedule.c içinde tanımlandı, burada create ediyoruz.
    xQueue0 = xQueueCreate(10, sizeof(TaskData_t)); // Real-Time
    xQueue1 = xQueueCreate(10, sizeof(TaskData_t)); // High
    xQueue2 = xQueueCreate(10, sizeof(TaskData_t)); // Medium
    xQueue3 = xQueueCreate(10, sizeof(TaskData_t)); // Low (RR)

    if(xQueue0 == NULL || xQueue1 == NULL || xQueue2 == NULL || xQueue3 == NULL) {
        printf("HATA: Kuyruklar olusturulamadi! Bellek yetersiz olabilir.\n");
        return -1;
    }

    // 4. Scheduler (Yönetici) Görevini Oluştur (Kişi 1'in fonksiyonu)
    xTaskCreate(vSchedulerTask,    // Fonksiyon
                "Scheduler",       // İsim
                configMINIMAL_STACK_SIZE, // Stack Boyutu
                NULL,              // Parametre
                tskIDLE_PRIORITY + 2, // FreeRTOS Önceliği (Scheduler en yüksek olmalı)
                NULL);             // Handle

    // 5. İşçi (Generic) Görevleri Oluştur (Kişi 2'nin fonksiyonu)
    // giris.txt'den okunan her satır için bir FreeRTOS görevi yaratıyoruz.
    for(int i = 0; i < globalTaskCount; i++) {
        
        // Her göreve kendi verisini parametre olarak gönderiyoruz (&allTasks[i])
        xTaskCreate(vGenericTask,
                    allTasks[i].taskName,   // Görev adı (T1, T2...)
                    configMINIMAL_STACK_SIZE, 
                    (void *)&allTasks[i],   // Parametre olarak struct adresi
                    tskIDLE_PRIORITY + 1,   // FreeRTOS önceliği (Yönetilmeye aday)
                    &allTasks[i].handle);   // Handle'ı struct içine kaydet
    }

    printf("Main: Tum gorevler olusturuldu. Scheduler baslatiliyor...\n");

    // 6. FreeRTOS Zamanlayıcıyı Başlat
    vTaskStartScheduler();

    // Kod buraya asla gelmemeli
    for(;;);
    return 0;
}