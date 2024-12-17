#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INVENTORY 5
#define MAX_LINE_LEN 256

typedef struct {
    char *name;
    char *description;
} Item;

typedef struct {
    char *name;
    int health;
    int strength;
} Creature;

typedef struct {
    char *name;
    char *description;
    int *itemIndices;
    int *creatureIndices;
    int up, down, left, right;
} Room;

typedef struct {
    int health;
    int strength;
    int currentRoom;
    int inventoryCount;
    int inventoryIndices[MAX_INVENTORY];
} Player;

void init_game();
void load_items();
void load_creatures();
void load_rooms();
void cleanup_game();
void game_loop();
int parse_command(char *input, char *cmd, char *arg);
void do_move(const char *direction);
void do_look();
void do_inventory();
void do_pickup(const char *itemName);
void do_attack();
void do_list_saves();
void do_save(const char *filepath);
void do_load(const char *filepath);
void do_exit_game();

int find_item_by_name(const char *name);
int find_item_in_room(int roomIndex, const char *name);
int remove_item_from_room(int roomIndex, int itemIndex);
void add_item_to_player(int itemIndex);
int find_creature_in_room(int roomIndex);

int safe_read_line(FILE *fp, char *buffer, int size);
char *safe_read_string(FILE *fp);
void trim_newline(char *str);

#endif
