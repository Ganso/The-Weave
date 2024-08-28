#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "entity.h"

#define MAX_ITEMS 10

typedef struct {
    Entity entity;
    bool interactable;
} Item;

Item obj_item[MAX_ITEMS];
Sprite *spr_item[MAX_ITEMS];

void init_item(u16 nitem); // Initialize an item
void release_item(u16 index); // Release an item

#endif