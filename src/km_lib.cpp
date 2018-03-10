#include "km_lib.h"

#include <stdio.h>

#define DYNAMIC_ARRAY_START_SIZE 10

#define HASH_MAP_START_SIZE 10

template <typename T>
void DynamicArray<T>::Init()
{
    size = 0;
    capacity = DYNAMIC_ARRAY_START_SIZE;
    data = (T*)malloc(sizeof(T) * capacity);
    if (!data) {
        // TODO error!
        printf("ERROR: not enough memory!\n");
    }
}

template <typename T>
DynamicArray<T> DynamicArray<T>::Copy() const
{
    DynamicArray<T> array;

    array.size = size;
    array.capacity = capacity;
    array.data = (T*)malloc(sizeof(T) * array.capacity);
    if (!array.data) {
        // TODO error!
        printf("ERROR: not enough memory!\n");
    }

    for (uint32 i = 0; i < array.size; i++) {
        array.data[i] = data[i];
    }

    return array;
}

template <typename T>
void DynamicArray<T>::Append(T element)
{
    if (size >= capacity) {
        capacity *= 2;
        data = (T*)realloc(data, sizeof(T) * capacity);
        if (!data) {
            // TODO error!
            printf("ERROR: not enough memory!\n");
        }
    }
    data[size++] = element;
}

template <typename T>
void DynamicArray<T>::Free()
{
    free(data);
}