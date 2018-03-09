#pragma once

template <typename T>
struct DynamicArray
{
    uint32 size;
    uint32 capacity;
    T* data;

    void Init();
    void Append(T element);
    void Free();

    T& operator[](int index) const {
        return data[index];
    }
};

/*
template <typename K, typename V>
struct HashNode
{
    K key;
    V value;
    HashNode* next;
}

template <typename K>
struct KeyHash
{
    unsigned long operator()(const K& key) const;
}

template <typename K, typename V, typename F = KeyHash<K>>
struct HashMap
{
    uint32 size;
    HashNode<K, V>* table;
}
*/