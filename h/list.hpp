#ifndef LIST_HPP
#define LIST_HPP

#include "memoryAllocator.hpp"

template <typename T>
class List
{
private:
    struct Elem
    {
        T *data;
        Elem *next;

        Elem(T *data, Elem *next) : data(data), next(next) {}
    };

    Elem *head, *tail;

public:
    List() : head(0), tail(0) {}

    List(const List<T> &) = delete;

    List<T> &operator=(const List<T> &) = delete;

    void addFirst(T *data)
    {
        Elem *elem = (Elem *)memoryAllocator::_kmalloc(sizeof(Elem));
        elem->data = data;
        elem->next = head;

        head = elem;
        if (!tail)
        {
            tail = head;
        }
    }

    void addLast(T *data)
    {

        Elem *elem = (Elem *)memoryAllocator::_kmalloc(sizeof(Elem));
        elem->data = data;
        elem->next = 0;

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

    T *removeFirst()
    {
        if (!head)
        {
            return 0;
        }

        Elem *elem = head;
        head = head->next;
        if (!head)
        {
            tail = 0;
        }

        T *ret = elem->data;
        // delete elem;
        memoryAllocator::_kmfree(elem);
        //
        return ret;
    }

    T *peekFirst()
    {
        if (!head)
        {
            return 0;
        }
        return head->data;
    }

    T *removeLast()
    {
        if (!head)
        {
            return 0;
        }

        Elem *prev = 0;
        for (Elem *curr = head; curr && curr != tail; curr = curr->next)
        {
            prev = curr;
        }

        Elem *elem = tail;
        if (prev)
        {
            prev->next = 0;
        }
        else
        {
            head = 0;
        }
        tail = prev;

        T *ret = elem->data;

        memoryAllocator::_kmfree(elem);
        //
        return ret;
    }

    int removeSpec(T *ptr)
    {
        if (ptr == nullptr)
            return 0;
        Elem *cur = head, *prev = nullptr;
        while (cur != nullptr)
        {
            if (cur->data == ptr)
            {
                break;
            }
            prev = cur;
            cur = cur->next;
        }
        if (cur == nullptr)
            return -1;
        if (prev == nullptr)
        {
            head = cur->next;
        }
        else
        {
            prev->next = cur->next;
        }
        if (cur->next == nullptr)
        {
            tail = prev;
        }
        memoryAllocator::_kmfree(cur);
        return 0;
    }

    T *peekLast()
    {
        if (!tail)
        {
            return 0;
        }
        return tail->data;
    }
};

#endif // OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_LIST_HPP
