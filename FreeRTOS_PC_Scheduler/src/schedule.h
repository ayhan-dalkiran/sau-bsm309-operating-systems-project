/*
 * DOSYA: schedule.h
 * SORUMLU: Kişi 3 (Veri & Kuyruk Yönetimi)
 * AÇIKLAMA: Proje genelindeki veri yapıları, global değişkenler ve 
 * fonksiyon prototipleri burada tanımlanır.
 */

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Maksimum görev sayısı (Örn: 100)
#define MAX_TASKS 100

// GÖREV YAPISI (Struct)
// giris.txt dosyasındaki her satır bu yapıya dönüştürülecek.
typedef struct {
    char taskName[10];  // Görev Adı (T1, T2 vb.)
    int arrivalTime;    // Varış Zamanı
    int priority;       // Öncelik (0: RealTime, 1, 2, 3: User)
    int burstTime;      // Toplam Çalışma Süresi
    int remainingTime;  // Kalan Süre (Simülasyon sırasında azalacak)
    TaskHandle_t handle;
    char *pcColorCode; // FreeRTOS Task Handle'ı (Kontrol için)
} TaskData_t;

// GLOBAL DEĞİŞKENLER (Extern ile diğer dosyalara "burada var" diyoruz)
// Tanımlamaları schedule.c içinde yapılacak.
extern QueueHandle_t xQueue0; // Real-Time (FCFS)
extern QueueHandle_t xQueue1; // High (User - Feedback)
extern QueueHandle_t xQueue2; // Medium (User - Feedback)
extern QueueHandle_t xQueue3; // Low (User - RR)

extern TaskData_t allTasks[MAX_TASKS]; // Tüm görevlerin listesi
extern int globalTaskCount;            // Toplam okunan görev sayısı

// FONKSİYON PROTOTİPLERİ
// Dosyadan okuma fonksiyonu
void loadTasksFromFile(const char *filename);

// Yardımcı Fonksiyonlar (Kişi 3 bunları schedule.c'de dolduracak)
QueueHandle_t getQueueByPriority(int priority); 
void decreaseTaskPriority(TaskData_t *task);

#endif /* SCHEDULE_H */