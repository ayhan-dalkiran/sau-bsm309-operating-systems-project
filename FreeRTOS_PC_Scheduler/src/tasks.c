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
    /* Uyarıyı engellemek için parametreyi void'e cast ediyoruz */
    (void) pvParameters;

    printf("Sistem: Scheduler Baslatiliyor...\n");

    for(;;) {
        // BURASI Kişi 1 TARAFINDAN DOLDURULACAK
        
        // Örnek (Dummy) Akış:
        // 1. Queue0 kontrol et (Real Time)
        // 2. Yoksa Queue1 kontrol et
        // 3. Seçilen taska 'xTaskNotifyGive' ile sinyal gönder.
        
        printf("[Scheduler]: Karar veriyorum... (Kişi 1 bekleniyor)\n");

        // pdMS_TO_TICKS hatası almamak için task.h'in yukarıda ekli olduğundan emin ol.
        vTaskDelay(pdMS_TO_TICKS(1000)); 
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