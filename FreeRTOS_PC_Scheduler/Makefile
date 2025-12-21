# --- PROJE AYARLARI ---
TARGET = freertos_sim
CC = gcc
CFLAGS = -Wall -Wextra -g -DPROJ_COVERAGE_DIST

# --- KLASÖR YOLLARI ---
# Kaynak kodlarimiz
SRC_DIR = src
# FreeRTOS ana klasoru
FREERTOS_DIR = FreeRTOS
# FreeRTOS kaynak dosyalari
FREERTOS_SRC = $(FREERTOS_DIR)
# FreeRTOS header dosyalari
FREERTOS_INC = $(FREERTOS_DIR)/include
# PC Simulasyonu icin gerekli port dosyalari (Linux/Windows POSIX)
PORT_DIR = $(FREERTOS_DIR)/portable/ThirdParty/GCC/Posix
# Hafiza yonetimi (Heap 3 genelde en uyumlusudur)
MEM_MANG = $(FREERTOS_DIR)/portable/MemMang/heap_3.c

# --- INCLUDE TANIMLAMALARI ---
# Derleyiciye .h dosyalarini nerede bulacagini soyluyoruz
INCLUDES = -I$(SRC_DIR) \
           -I$(FREERTOS_INC) \
           -I$(PORT_DIR) \
           -I$(PORT_DIR)/utils

# --- DERLENECEK DOSYALAR ---
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/tasks.c \
       $(SRC_DIR)/schedule.c \
       $(FREERTOS_SRC)/tasks.c \
       $(FREERTOS_SRC)/queue.c \
       $(FREERTOS_SRC)/list.c \
       $(FREERTOS_SRC)/timers.c \
       $(PORT_DIR)/port.c \
       $(PORT_DIR)/utils/wait_for_event.c \
       $(MEM_MANG)

# --- DERLEME KOMUTU ---
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ -lpthread

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)