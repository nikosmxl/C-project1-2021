#include "set_utils.h"


Pointer set_find_eq_or_greater(Set set, Pointer value){
    SetNode v;
    if ((v = set_find_node(set, value)) != SET_EOF){
        return set_node_value(set, v);
    }
    else{
        set_insert(set, value);
        v = set_next(set, set_find_node(set, value));
        set_remove(set, value);
        value = NULL;
    }
    if (v == SET_EOF){
        return NULL;
    }
    return (set_node_value(set, v));
}

Pointer set_find_eq_or_smaller(Set set, Pointer value){
    SetNode v;
    if ((v = set_find_node(set, value)) != SET_EOF){
        return set_node_value(set, v);
    }
    else{
        set_insert(set, value);
        v = set_previous(set, set_find_node(set, value));
        set_remove(set, value);
    }
    if (v == SET_BOF){
        return NULL;
    }
    return (set_node_value(set, v));
}