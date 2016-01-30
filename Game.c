//Daniel Farley, dfarley@ucsc.edu

#include "Game.h"

// Standard libraries
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries - Hardware
#include "HardwareDefs.h"
#include "Adc.h"
#include "Ascii.h"
#include "Buttons.h"
#include "Common.h"
#include "Leds.h"
#include "Oled.h"
#include "OledDriver.h"

// User libraries - Game
#include "Player.h"

//Function declarations
static int GameSetCurrentRoomTitle(void);
static int GameSetCurrentRoomOffset(void);
static int GameSetCurrentRoomDescription(void);
static int GameSetCurrentRoomItems(void);
static int GameSetCurrentRoomExits(void);


//Current Room struct
static struct room{
    char title[GAME_MAX_ROOM_TITLE_LENGTH + 1];
    char desc[GAME_MAX_ROOM_DESC_LENGTH + 1];
    uint8_t exits[4];
    
    uint8_t roomNum;
    char filename[12];
} currRoom;

static FILE *filestream = NULL;


int GameGoNorth(void)
{
    //if North isn't a valid exit, return STANDARD_ERROR
    if (currRoom.exits[0] == 0) return SUCCESS;

    //if you can go north, get the new room's
    int ret = SUCCESS;
    currRoom.roomNum = currRoom.exits[0];
    sprintf(currRoom.filename, "/room%u.txt", currRoom.roomNum);
    filestream = fopen(currRoom.filename, "rb");
    if (filestream == NULL) FATAL_ERROR();

    if (GameSetCurrentRoomTitle() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomOffset() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomDescription() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomItems() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomExits() != SUCCESS) ret = STANDARD_ERROR;
    fclose(filestream);
    filestream = NULL;
    return ret;
}

int GameGoEast(void)
{
    //if East isn't a valid exit, return STANDARD_ERROR
    if (currRoom.exits[1] == 0) return SUCCESS;

    //if you can go east, get the new room's
    int ret = SUCCESS;
    currRoom.roomNum = currRoom.exits[1];
    sprintf(currRoom.filename, "/room%u.txt", currRoom.roomNum);
    filestream = fopen(currRoom.filename, "rb");
    if (filestream == NULL) FATAL_ERROR();

    if (GameSetCurrentRoomTitle() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomOffset() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomDescription() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomItems() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomExits() != SUCCESS) ret = STANDARD_ERROR;
    fclose(filestream);
    filestream = NULL;
    return ret;
}

int GameGoSouth(void)
{
    //if South isn't a valid exit, return STANDARD_ERROR
    if (currRoom.exits[2] == 0) return SUCCESS;

    //if you can go south, get the new room's
    int ret = SUCCESS;
    currRoom.roomNum = currRoom.exits[2];
    sprintf(currRoom.filename, "/room%u.txt", currRoom.roomNum);
    filestream = fopen(currRoom.filename, "rb");
    if (filestream == NULL) FATAL_ERROR();

    if (GameSetCurrentRoomTitle() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomOffset() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomDescription() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomItems() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomExits() != SUCCESS) ret = STANDARD_ERROR;
    fclose(filestream);
    filestream = NULL;
    return ret;
}

int GameGoWest(void)
{
    //if West isn't a valid exit, return STANDARD_ERROR
    if (currRoom.exits[3] == 0) return SUCCESS;

    //if you can go west, get the new room's
    int ret = SUCCESS;
    currRoom.roomNum = currRoom.exits[3];
    sprintf(currRoom.filename, "/room%u.txt", currRoom.roomNum);
    filestream = fopen(currRoom.filename, "rb");
    if (filestream == NULL) FATAL_ERROR();

    if (GameSetCurrentRoomTitle() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomOffset() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomDescription() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomItems() != SUCCESS) ret = STANDARD_ERROR;
    if (GameSetCurrentRoomExits() != SUCCESS) ret = STANDARD_ERROR;
    fclose(filestream);
    filestream = NULL;
    return ret;
}

int GameInit(void)
{
    //Initialize everything and set first movement: North to room 32
    currRoom.title[0] = '\0';
    currRoom.desc[0] = '\0';
    currRoom.exits[0] = STARTING_ROOM;
    currRoom.exits[1] = currRoom.exits[2] = currRoom.exits[3] = 0;
    currRoom.roomNum = 0;
    currRoom.filename[0] = '\0';

    return GameGoNorth();
}

//Sets the current room's title from the file currently open.  After this function, the filestream
// should point to the first version's description length.
static int GameSetCurrentRoomTitle(void)
{
    if (filestream == NULL) {
        *currRoom.title = '\0';
        return STANDARD_ERROR;
    } else {
        int titleLength = fgetc(filestream);
        if (titleLength == EOF) return STANDARD_ERROR;
        int i;
        for (i = 0; i < titleLength; i++) {
            currRoom.title[i] = fgetc(filestream);
            if (currRoom.title[i] == EOF) {
                currRoom.title[i] = '\0';
                return STANDARD_ERROR;
            }
        }
        currRoom.title[i] = '\0';
    }

    //printf("Title: %d: {%s}\n",
    //        (int)strlen(currRoom.title), currRoom.title);
    return SUCCESS;
}

//Increments the filestream to the version of the room that is accessible based on the player's
// inventory.  After this function, the filestream should point to the length of the description.
static int GameSetCurrentRoomOffset(void)
{
    if (filestream == NULL) return STANDARD_ERROR;
    while (1) {
        int numItems = fgetc(filestream);
        if (numItems == EOF) return STANDARD_ERROR;
        bool foundAllItems = true;

        //check all required items
        int i;
        for (i = 0; i < numItems; i++) {
            uint8_t chkItem = (uint8_t)fgetc(filestream);
            if (chkItem == EOF) return STANDARD_ERROR;
            if (FindInInventory(chkItem) != SUCCESS) foundAllItems = false;
        }

        //if foundAllItemss == true here then this is the correct offset and filestream should
        // point at the length for this version's description
        if (foundAllItems == true) {
            return SUCCESS;
        } else { //if not, then discard this version of the room and try again
            //eat the description (nom nom nom!)
            uint8_t descLen = (uint8_t)fgetc(filestream), i;
            if (descLen == EOF) return STANDARD_ERROR;
            for (i = 0; i < descLen; i++) {
                if(fgetc(filestream) == EOF) return STANDARD_ERROR;
            }

            //eat the items (itemLen) and exits (+4)
            uint8_t itemLen = (uint8_t)fgetc(filestream);
            if (itemLen == EOF) return STANDARD_ERROR;
            for (i = 0; i < itemLen + 4; i++) {
                if(fgetc(filestream) == EOF) return STANDARD_ERROR;
            }
        }
    }
}

//Sets the current room's description from the file currently open.  After this function, the
// filestream should point to the items in the room.
static int GameSetCurrentRoomDescription(void)
{
    if (filestream == NULL) {
        *currRoom.desc = '\0';
        return STANDARD_ERROR;
    } else {
        int descLength = fgetc(filestream);
        if (descLength == EOF) return STANDARD_ERROR;
        int i;
        for (i = 0; i < descLength; i++) {
            currRoom.desc[i] = fgetc(filestream);
            if (currRoom.desc[i] == EOF) {
                currRoom.desc[i] = '\0';
                return STANDARD_ERROR;
            }
        }
        currRoom.desc[i] = '\0';
    }

    //printf("Description: %d: {%s}\n",
    //        (int)strlen(currRoom.desc), currRoom.desc);
    return SUCCESS;
}

//Adds this room's items to the player's inventory.  After this function, the filestream should
// point to this room's exits.
static int GameSetCurrentRoomItems(void)
{
    uint8_t itemLen = (uint8_t)fgetc(filestream), i;
    if (itemLen == EOF) return STANDARD_ERROR;
    for (i = 0; i < itemLen; i++) {
        uint8_t addThis = (uint8_t)fgetc(filestream);
        if (addThis == EOF) return STANDARD_ERROR;
        AddToInventory(addThis);
        //printf("Added {%u} to inventory\n", addThis);
    }
    return SUCCESS;
}
//Sets the current room's exits from the currently open file.  After this function, the filestream
// should point to the next version of the room or an EOF.
static int GameSetCurrentRoomExits(void)
{
    int i;
    for (i = 0; i < 4; i++) {
        currRoom.exits[i] = fgetc(filestream);
        if (currRoom.exits[i] == EOF) return STANDARD_ERROR;
    }
    //printf("Exits: %u, %u, %u, %u\n",
    //        currRoom.exits[0], currRoom.exits[1], currRoom.exits[2], currRoom.exits[3]);
    return SUCCESS;
}

int GameGetCurrentRoomTitle(char *title)
{
    if (strcpy(title, currRoom.title) == NULL) {
        *title = '\0';
    }
    return strlen(title);
}

int GameGetCurrentRoomDescription(char *desc)
{
    if (strcpy(desc, currRoom.desc) == NULL) {
        *desc = '\0';
    }
    return strlen(desc);
}

uint8_t GameGetCurrentRoomExits(void)
{
    uint8_t ret = 0;
    if (currRoom.exits[0] != 0) ret |= GAME_ROOM_EXIT_NORTH_EXISTS;
    if (currRoom.exits[1] != 0) ret |= GAME_ROOM_EXIT_EAST_EXISTS;
    if (currRoom.exits[2] != 0) ret |= GAME_ROOM_EXIT_SOUTH_EXISTS;
    if (currRoom.exits[3] != 0) ret |= GAME_ROOM_EXIT_WEST_EXISTS;
    return ret;
}
