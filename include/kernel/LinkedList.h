template<class T> struct KLinkedListNode {
    KLinkedListNode<T>* next;
    KLinkedListNode<T>* prev;
    T* data;
};

template<class T> class KLinkedList {
public:
    KLinkedList() {
        list = NULL;
    }

    int AddItem(T* item) {
        KLinkedListNode<T>* node = (KLinkedListNode<T>*)
            malloc(sizeof(KLinkedListNode<T>));

        if(node == NULL)
            return 1;

        node->prev = NULL;
        node->next = list;
        node->data = item;

        if(list != NULL)
            list->prev = node;

        list = node;
        return 0;
    }

    int RemoveItem(KLinkedListNode<T>* item) {
        KLinkedListNode<T>* prev = item->prev;
        KLinkedListNode<T>* next = item->next;

        if(prev != NULL)
            prev->next = next;
        else
            list = next;

        if(next != NULL)
            next->prev = prev;

        return 0;
    }

    ~KLinkedList() {
        KLinkedListNode<T>* p = list;

        while(p != NULL) {
            KLinkedListNode<T>* next = p->next;
            free(p); p = next;
        }
    }

    KLinkedListNode<T>* list;
};

// Keeps reference counting for KAutoObj's.
template<class T> class KLinkedRefList : public KLinkedList<T> {
public:
    typedef KLinkedList<T> super;

    int AddItem(T* item) {
        item->AcquireReference();
        return super::AddItem(item);
    }

    int RemoveItem(KLinkedListNode<T>* item) {
        item->data->ReleaseReference();
        return super::RemoveItem(item);
    }

    ~KLinkedRefList() {
        KLinkedListNode<T>* p = super::list;

        while(p != NULL) {
            KLinkedListNode<T>* next = p->next;

            p->data->ReleaseReference();
            free(p); p = next;
        }
    }
};
