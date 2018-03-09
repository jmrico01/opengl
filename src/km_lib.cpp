#include "km_lib.h"

#include <stdio.h>

#define DYNAMIC_ARRAY_START_SIZE 10

#define HASH_MAP_START_SIZE 10

template <typename T>
void DynamicArray<T>::Init()
{
    this->size = 0;
    this->capacity = DYNAMIC_ARRAY_START_SIZE;
    this->data = (T*)malloc(sizeof(T) * this->capacity);
    if (!this->data) {
        // TODO error!
        printf("ERROR: not enough memory!\n");
    }
}

template <typename T>
void DynamicArray<T>::Append(T element)
{
    if (this->size >= this->capacity) {
        this->capacity *= 2;
        this->data = (T*)realloc(this->data, sizeof(T) * this->capacity);
        if (!this->data) {
            // TODO error!
            printf("ERROR: not enough memory!\n");
        }
    }
    this->data[this->size++] = element;
}

template <typename T>
void DynamicArray<T>::Free()
{
    free(this->data);
}