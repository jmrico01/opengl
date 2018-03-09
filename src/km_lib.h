#pragma once

template <typename T>
struct DynamicArray {
    uint32 size;
    uint32 capacity;
    T* data;

    void Init();
    void Append(T element);
    void Free();
};