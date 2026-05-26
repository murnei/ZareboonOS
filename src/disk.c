#include "stdio.h"

void ide_wait_ready() {
    // Небольшая пауза, чтобы контроллер успел выставить статус после команды
    for (int i = 0; i < 1000; i++) {
        inb(0x1F7); 
    }

    // Ждем, пока уйдет Busy
    while (inb(0x1F7) & 0x80);

    // Ждем, пока появится Ready (DRQ или RDY)
    // 0x40 - это бит RDY (Ready), 0x08 - это DRQ (Data Request)
    while (!(inb(0x1F7) & (0x40 | 0x08)));
}


void read_sector(uint32_t lba, uint8_t* buffer) {
    // Выбираем Master диск
    outb(0x1F6, (uint8_t)(0xE0 | ((lba >> 24) & 0x0F))); 
    outb(0x1F1, 0x00); // Некоторые контроллеры требуют обнуления порта ошибок
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20); // Команда чтения

    ide_wait_ready();

    print("[R]"); // Сигнал, что начали чтение данных
    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(0x1F0);
        buffer[i * 2] = (uint8_t)data;
        buffer[i * 2 + 1] = (uint8_t)(data >> 8);
    }
    print("[OK]");
}

extern void outw(unsigned short port, unsigned short data);

// Не забудь добавить это объявление в начало disk.c
extern void outw(unsigned short port, unsigned short data);

void write_sector(uint32_t lba, uint8_t* buffer) {
    // 1. Выбираем диск и режим LBA
    outb(0x1F6, (uint8_t)(0xE0 | ((lba >> 24) & 0x0F)));
    
    // 2. Ждем, пока диск будет готов принять команду
    ide_wait_ready();

    // 3. Параметры операции
    outb(0x1F2, 1);                  // Пишем 1 сектор
    outb(0x1F3, (uint8_t)lba);       // LBA 0-7
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30);               // Команда 0x30 — ЗАПИСЬ

    // 4. Ждем готовности буфера диска к приему данных (DRQ)
    ide_wait_ready();

    // 5. Отправляем 256 слов (512 байт) в порт данных
    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        outw(0x1F0, data);
    }
    
    // 6. Даем диску время сбросить данные из кэша на блины (Flush)
    outb(0x1F7, 0xE7); // Команда Cache Flush (опционально, но полезно)
    ide_wait_ready();
}
