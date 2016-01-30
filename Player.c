//Daniel Farley, dfarley@ucsc.edu

#include "Player.h"

uint8_t inventory[INVENTORY_SIZE] = {0, 0, 0, 0};

int AddToInventory(uint8_t item)
{
    int i;
    for (i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == 0) {
            inventory[i] = item;
            return SUCCESS;
        }
    }
    return STANDARD_ERROR;
}

int FindInInventory(uint8_t item)
{
    int i;
    for (i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == item) return SUCCESS;
    }
    return STANDARD_ERROR;
}
