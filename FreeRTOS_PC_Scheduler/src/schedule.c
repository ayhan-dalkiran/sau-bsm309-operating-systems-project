/*
 * DOSYA: schedule.c
 * SORUMLU: Kişi 3
 * GÖREV: Veri okuma ve kuyruk yardımcı fonksiyonlarını yazmak.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schedule.h"


/* * FONKSİYON: loadTasksFromFile
 * GÖREV: giris.txt dosyasını okur ve allTasks dizisine doldurur.
 */
void loadTasksFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Dosya acilamadi");
        exit(1);
    }

    printf("Dosya okunuyor: %s\n", filename);
    
    // Dosya formatı: "isim, varis, oncelik, sure" varsayiyoruz.
    // Örnek: T1, 0, 1, 5
    // Kişi 3: Dosya formatına göre buradaki fscanf'i düzenleyebilirsin.
    while (fscanf(file, "%[^,], %d, %d, %d\n", 
                  allTasks[globalTaskCount].taskName,
                  &allTasks[globalTaskCount].arrivalTime,
                  &allTasks[globalTaskCount].priority,
                  &allTasks[globalTaskCount].burstTime) != EOF) {
        
        // Başlangıçta kalan süre = toplam süre
        allTasks[globalTaskCount].remainingTime = allTasks[globalTaskCount].burstTime;
        allTasks[globalTaskCount].handle = NULL; // Henüz task oluşturulmadı
        
        globalTaskCount++;
        if(globalTaskCount >= MAX_TASKS) break;
    }

    fclose(file);
    printf("Toplam %d gorev yuklendi.\n", globalTaskCount);
}

/* * [GÖREV - KİŞİ 3]: Bu fonksiyonu doldur.
 * Verilen öncelik değerine (0,1,2,3) göre ilgili xQueue handle'ını döndürmeli.
 */
QueueHandle_t getQueueByPriority(int priority) {
    // İPUCU: switch-case yapısı kullan.
    // case 0: return xQueue0;
    
    return NULL; // Şimdilik boş
}

/* * [GÖREV - KİŞİ 3]: Bu fonksiyonu doldur.
 * Bir görevin önceliğini düşürmek (Kuyruk değiştirmek) için mantığı kur.
 * Eğer öncelik < 3 ise bir artır (Öncelik düşer). 
 */
void decreaseTaskPriority(TaskData_t *task) {
    // Kod buraya...
    printf("Helper: %s gorevinin onceligi dusuruluyor...\n", task->taskName);
}