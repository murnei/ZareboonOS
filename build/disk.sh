#!/usr/bin/env bash

clear

echo "1 - удалить файл с диска"
echo "2 - очистить диск полностью (FAT12)"
echo "3 - скопировать файл на диск"
echo "4 - скомпилировать скрипт и закинуть на диск"
echo ""

read -p "Введите ответ: " answer

if [ "$answer" -eq 1 ]; then
    echo "Содержимое диска:"
    mdir -i test-disk.img ::

    read -p "Какой файл хотите удалить? " file_input
    file_name=$(echo "$file_input" | tr '[:lower:]' '[:upper:]' | xargs)

    echo "Удаляю: $file_name..."
    mdel -i test-disk.img ::"$file_name"

elif [ "$answer" -eq 2 ]; then
    rm -f test-disk.img
    dd if=/dev/zero of=test-disk.img bs=1k count=1440 2>/dev/null
    mformat -i test-disk.img -f 1440 ::
    echo "Диск пересоздан."

elif [ "$answer" -eq 3 ]; then
    read -p "Введите название файла (с расширением): " file_name
    mcopy -i test-disk.img "$file_name" ::"$file_name"
    echo "Файл $file_name скопирован."

elif [ "$answer" -eq 4 ]; then
    read -p "Введите название файла без расширения: " file_name
    
    if nasm -f bin "$file_name".asm -o "$file_name".zrn; then
        remote_name=$(echo "$file_name.zrn" | tr '[:lower:]' '[:upper:]')
        
        mcopy -o -i test-disk.img "$file_name".zrn ::"$remote_name"
        echo "Файл $remote_name успешно скомпилирован и скопирован."
    else
        echo "Ошибка компиляции!"
    fi
fi

