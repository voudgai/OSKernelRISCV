#ifndef LIST_HPP
#define LIST_HPP

#include "memoryAllocator.hpp"

template <typename T>
class List
{
private:
    using foreachFunc = void (*)(T *, void *);
    using conditionFunc = bool (*)(T *, void *);

    using compareFunc = bool (*)(T *, T *, void *);
    struct Elem
    {
        T *data;
        Elem *next;

        Elem(T *data, Elem *next) : data(data), next(next) {}
    };

    Elem *head, *tail;

    // for sorting...
    Elem *merge(Elem *left, Elem *right, compareFunc cmp, void *aux);
    Elem *getMiddle(Elem *head);
    Elem *mergeSort(Elem *head, compareFunc cmp, void *aux);

public:
    List() : head(nullptr), tail(nullptr) {}

    List(const List<T> &other) = delete;
    List<T> &operator=(const List<T> &other) = delete;

    void foreach (foreachFunc func, void *aux); // for each element calls foreachFunc

    void foreachWhile(foreachFunc func, void *auxFunc, conditionFunc condition, void *auxCond); // for each element calls foreachFunc
                                                                                                // while condition returns true
                                                                                                // first time condition fails, function ends

    void sort(compareFunc cmp, void *aux); // sorts using cmp as comparator

    void addFirst(T *data);
    void addLast(T *data);
    void insert_sorted(T *data, compareFunc cmp, void *aux);

    T *removeFirst();
    T *peekFirst() const;

    T *removeLast();
    T *peekLast() const;

    bool removeSpec(T *ptr);
};

// template <typename T>
// List<T>::~List()
// {
//     while (head)
//     {
//         Elem *temp = head;
//         head = head->next;
//         memoryAllocator::_kmfree(temp);
//     }
// }

template <typename T>
void List<T>::addFirst(T *data)
{
    size_t numOfBlocks = (memoryAllocator::SIZE_HEADER + sizeof(Elem) + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    Elem *elem = (Elem *)(memoryAllocator::_kmalloc(numOfBlocks));
    elem->data = data;
    elem->next = head;

    head = elem;
    if (!tail)
    {
        tail = head;
    }
}

template <typename T>
void List<T>::addLast(T *data)
{
    size_t numOfBlocks = (memoryAllocator::SIZE_HEADER + sizeof(Elem) + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    Elem *elem = (Elem *)(memoryAllocator::_kmalloc(numOfBlocks));
    elem->data = data;
    elem->next = nullptr;

    if (tail)
    {
        tail->next = elem;
        tail = elem;
    }
    else
    {
        head = tail = elem;
    }
}

template <typename T>
T *List<T>::removeFirst()
{
    if (!head)
    {
        return nullptr;
    }

    Elem *elem = head;
    head = head->next;
    if (!head)
    {
        tail = nullptr;
    }

    T *ret = elem->data;
    memoryAllocator::_kmfree(elem);
    return ret;
}

template <typename T>
T *List<T>::peekFirst() const
{
    return head ? head->data : nullptr;
}

template <typename T>
T *List<T>::removeLast()
{
    if (!head)
    {
        return nullptr;
    }

    Elem *prev = nullptr;
    for (Elem *curr = head; /*curr &&*/ curr != tail; curr = curr->next)
    {
        prev = curr;
    }

    Elem *elem = tail;
    if (prev)
    {
        prev->next = nullptr;
        tail = prev;
    }
    else
    {
        head = tail = nullptr;
    }

    T *ret = elem->data;
    memoryAllocator::_kmfree(elem);
    return ret;
}

template <typename T>
bool List<T>::removeSpec(T *ptr)
{
    if (!ptr)
        return false;

    Elem *prev = nullptr;
    for (Elem *curr = head; curr; prev = curr, curr = curr->next)
    {
        if (curr->data == ptr)
        {
            if (prev)
                prev->next = curr->next;
            else
                head = curr->next;

            if (curr == tail)
                tail = prev;

            memoryAllocator::_kmfree(curr);
            return true;
        }
    }
    return false;
}

template <typename T>
T *List<T>::peekLast() const
{
    return tail ? tail->data : nullptr;
}

template <typename T>
void List<T>::insert_sorted(T *data, compareFunc cmp, void *aux)
{
    if (!data)
        return;

    Elem *prev = nullptr;
    Elem *curr = head;

    while (curr != nullptr && cmp(curr->data, data, aux))
    {
        prev = curr;
        curr = prev->next;
    }

    size_t numOfBlocks = (memoryAllocator::SIZE_HEADER + sizeof(Elem) + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    Elem *elem = (Elem *)(memoryAllocator::_kmalloc(numOfBlocks));
    elem->data = data;
    elem->next = curr;

    if (prev == nullptr)
        head = elem;
    else
        prev->next = elem;

    if (curr == nullptr)
        tail = elem;
}

template <typename T>
void List<T>::sort(compareFunc cmp, void *aux)
{
    if (!head || !head->next)
        return;

    head = mergeSort(head, cmp, aux);

    tail = head;
    while (tail->next)
    {
        tail = tail->next;
    }
}

template <typename T>
typename List<T>::Elem *List<T>::mergeSort(Elem *head, compareFunc cmp, void *aux)
{
    if (!head || !head->next)
        return head;

    Elem *middle = getMiddle(head);
    Elem *nextToMiddle = middle->next;

    middle->next = nullptr;

    Elem *left = mergeSort(head, cmp, aux);
    Elem *right = mergeSort(nextToMiddle, cmp, aux);

    return merge(left, right, cmp, aux);
}

template <typename T>
typename List<T>::Elem *List<T>::getMiddle(Elem *head)
{
    if (!head)
        return head;

    Elem *slow = head;
    Elem *fast = head->next;

    while (fast)
    {
        fast = fast->next;
        if (fast)
        {
            slow = slow->next;
            fast = fast->next;
        }
    }

    return slow;
}

template <typename T>
typename List<T>::Elem *List<T>::merge(Elem *left, Elem *right, compareFunc cmp, void *aux)
{
    if (!left)
        return right;
    if (!right)
        return left;

    Elem *result = nullptr;

    if (cmp(left->data, right->data, aux))
    {
        result = left;
        result->next = merge(left->next, right, cmp, aux);
    }
    else
    {
        result = right;
        result->next = merge(left, right->next, cmp, aux);
    }

    return result;
}

template <typename T>
void List<T>::foreach (List<T>::foreachFunc func, void *aux)
{
    for (Elem *curr = head; curr; curr = curr->next)
    {
        func(curr->data, aux);
    }
}

template <typename T>
void List<T>::foreachWhile(List<T>::foreachFunc func, void *auxFunc, List<T>::conditionFunc condition, void *auxCond)
{
    for (Elem *curr = head; curr && condition(curr->data, auxCond); curr = curr->next)
    {
        func(curr->data, auxFunc);
    }
}
#endif // LIST_HPP