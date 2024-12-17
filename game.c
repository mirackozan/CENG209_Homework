#include "game.h"

static Player player;
static Room *rooms = NULL;
static Item *items = NULL;
static Creature *creatures = NULL;
static int numRooms = 0;
static int numItems = 0;
static int numCreatures = 0;

void init_game() {
    player.health = 20;
    player.strength = 5;
    player.currentRoom = 0;
    player.inventoryCount = 0;
    for (int i = 0; i < MAX_INVENTORY; i++)
        player.inventoryIndices[i] = -1;

    load_items();
    load_creatures();
    load_rooms();
}

void load_items() {
    FILE *fp = fopen("items.txt", "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open items.txt\n");
        exit(1);
    }

    if (fscanf(fp, "%d\n", &numItems) != 1) {
        fprintf(stderr, "Error reading number of items.\n");
        fclose(fp);
        exit(1);
    }

    items = calloc(numItems, sizeof(Item));
    if (!items) {
        fprintf(stderr, "Memory error allocating items.\n");
        exit(1);
    }

    for (int i = 0; i < numItems; i++) {
        items[i].name = safe_read_string(fp);
        items[i].description = safe_read_string(fp);
    }
    fclose(fp);
}

void load_creatures() {
    FILE *fp = fopen("creatures.txt", "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open creatures.txt\n");
        exit(1);
    }

    if (fscanf(fp, "%d\n", &numCreatures) != 1) {
        fprintf(stderr, "Error reading number of creatures.\n");
        fclose(fp);
        exit(1);
    }

    creatures = calloc(numCreatures, sizeof(Creature));
    if (!creatures) {
        fprintf(stderr, "Memory error.\n");
        exit(1);
    }

    for (int i = 0; i < numCreatures; i++) {
        char nameBuf[MAX_LINE_LEN];
        int h, s;
        if (fscanf(fp, "%s %d %d\n", nameBuf, &h, &s) != 3) {
            fprintf(stderr, "Error reading creature.\n");
            fclose(fp);
            exit(1);
        }
        creatures[i].name = strdup(nameBuf);
        creatures[i].health = h;
        creatures[i].strength = s;
    }
    fclose(fp);
}

void load_rooms() {
    FILE *fp = fopen("rooms.txt", "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open rooms.txt\n");
        exit(1);
    }

    if (fscanf(fp, "%d\n", &numRooms) != 1) {
        fprintf(stderr, "Error reading number of rooms.\n");
        fclose(fp);
        exit(1);
    }

    rooms = calloc(numRooms, sizeof(Room));
    if (!rooms) {
        fprintf(stderr, "Memory error.\n");
        exit(1);
    }

    for (int i = 0; i < numRooms; i++) {
        rooms[i].name = safe_read_string(fp);
        rooms[i].description = safe_read_string(fp);

        // Items
        int itemCount;
        if (fscanf(fp, "%d\n", &itemCount) != 1) {
            fprintf(stderr, "Error reading item count for room.\n");
            exit(1);
        }
        rooms[i].itemIndices = malloc((itemCount+1)*sizeof(int));
        if (!rooms[i].itemIndices) {
            fprintf(stderr, "Memory error.\n");
            exit(1);
        }
        for (int j = 0; j < itemCount; j++) {
            char buffer[MAX_LINE_LEN];
            safe_read_line(fp, buffer, MAX_LINE_LEN);
            int idx = find_item_by_name(buffer);
            if (idx == -1) {
                fprintf(stderr, "Warning: item '%s' not found.\n", buffer);
                idx = -1;
            }
            rooms[i].itemIndices[j] = idx;
        }
        rooms[i].itemIndices[itemCount] = -1;

        // Creatures
        int creatureCount;
        if (fscanf(fp, "%d\n", &creatureCount) != 1) {
            fprintf(stderr, "Error reading creature count for room.\n");
            exit(1);
        }
        rooms[i].creatureIndices = malloc((creatureCount+1)*sizeof(int));
        if (!rooms[i].creatureIndices) {
            fprintf(stderr, "Memory error.\n");
            exit(1);
        }
        for (int j = 0; j < creatureCount; j++) {
            char buffer[MAX_LINE_LEN];
            safe_read_line(fp, buffer, MAX_LINE_LEN);
            int found = -1;
            for (int c = 0; c < numCreatures; c++) {
                if (strcasecmp(creatures[c].name, buffer) == 0) {
                    found = c;
                    break;
                }
            }
            if (found == -1) {
                fprintf(stderr, "Warning: creature '%s' not found.\n", buffer);
            }
            rooms[i].creatureIndices[j] = found;
        }
        rooms[i].creatureIndices[creatureCount] = -1;

        if (fscanf(fp, "%d %d %d %d\n", &rooms[i].up, &rooms[i].down, &rooms[i].left, &rooms[i].right) != 4) {
            fprintf(stderr, "Error reading room connections.\n");
            exit(1);
        }
    }
    fclose(fp);
}

void cleanup_game() {
    if (rooms) {
        for (int i = 0; i < numRooms; i++) {
            free(rooms[i].name);
            free(rooms[i].description);
            free(rooms[i].itemIndices);
            free(rooms[i].creatureIndices);
        }
        free(rooms);
    }
    if (items) {
        for (int i = 0; i < numItems; i++) {
            free(items[i].name);
            free(items[i].description);
        }
        free(items);
    }
    if (creatures) {
        for (int i = 0; i < numCreatures; i++) {
            free(creatures[i].name);
        }
        free(creatures);
    }
}

void game_loop() {
    char line[MAX_LINE_LEN];
    char cmd[32], arg[MAX_LINE_LEN];
    for (;;) {
        printf("dungeon> ");
        fflush(stdout);
        if (!fgets(line, MAX_LINE_LEN, stdin)) break;
        trim_newline(line);
        if (strlen(line) == 0) continue;

        cmd[0] = '\0';
        arg[0] = '\0';
        parse_command(line, cmd, arg);

        if (strcmp(cmd, "help") == 0) {
            printf("Commands:\n");
            printf("  move <direction>\n");
            printf("  look\n");
            printf("  inventory\n");
            printf("  pickup <item>\n");
            printf("  attack\n");
            printf("  list\n");
            printf("  save <file>\n");
            printf("  load <file>\n");
            printf("  exit\n");
        } else if (strcmp(cmd, "move") == 0) {
            if (strlen(arg) == 0) {
                printf("Usage: move <direction>\n");
            } else {
                do_move(arg);
            }
        } else if (strcmp(cmd, "look") == 0) {
            do_look();
        } else if (strcmp(cmd, "inventory") == 0) {
            do_inventory();
        } else if (strcmp(cmd, "pickup") == 0) {
            if (strlen(arg) == 0) {
                printf("Usage: pickup <item>\n");
            } else {
                do_pickup(arg);
            }
        } else if (strcmp(cmd, "attack") == 0) {
            do_attack();
        } else if (strcmp(cmd, "list") == 0) {
            do_list_saves();
        } else if (strcmp(cmd, "save") == 0) {
            if (strlen(arg) == 0) {
                printf("Usage: save <file>\n");
            } else {
                do_save(arg);
            }
        } else if (strcmp(cmd, "load") == 0) {
            if (strlen(arg) == 0) {
                printf("Usage: load <file>\n");
            } else {
                do_load(arg);
            }
        } else if (strcmp(cmd, "exit") == 0) {
            do_exit_game();
            break;
        } else {
            printf("Unknown command. Type 'help' for assistance.\n");
        }

        if (player.health <= 0) {
            printf("You have died. Game Over.\n");
            break;
        }
    }
}

int parse_command(char *input, char *cmd, char *arg) {
    char *token = strtok(input, " ");
    if (!token) return 0;
    strcpy(cmd, token);
    char *rest = strtok(NULL, "");
    if (rest) {
        while(*rest && isspace((unsigned char)*rest)) rest++;
        strcpy(arg, rest);
    } else {
        arg[0] = '\0';
    }
    return 1;
}

void do_move(const char *direction) {
    int nextRoom = -1;
    Room *cur = &rooms[player.currentRoom];
    if (strcasecmp(direction, "up") == 0) {
        nextRoom = cur->up;
    } else if (strcasecmp(direction, "down") == 0) {
        nextRoom = cur->down;
    } else if (strcasecmp(direction, "left") == 0) {
        nextRoom = cur->left;
    } else if (strcasecmp(direction, "right") == 0) {
        nextRoom = cur->right;
    } else {
        printf("Invalid direction.\n");
        return;
    }

    if (nextRoom == -1) {
        printf("You can't move that way.\n");
        return;
    }

    player.currentRoom = nextRoom;
    do_look();
}

void do_look() {
    Room *r = &rooms[player.currentRoom];
    printf("You are in %s\n", r->name);
    printf("%s\n", r->description);

    int hasItems = 0;
    if (r->itemIndices) {
        for (int i = 0; r->itemIndices[i] != -1; i++) {
            if (r->itemIndices[i] != -1) {
                if (!hasItems) {
                    printf("Items in the room:\n");
                    hasItems = 1;
                }
                printf("  %s\n", items[r->itemIndices[i]].name);
            }
        }
    }

    int hasCreatures = 0;
    if (r->creatureIndices) {
        for (int i = 0; r->creatureIndices[i] != -1; i++) {
            int cidx = r->creatureIndices[i];
            if (cidx != -1 && creatures[cidx].health > 0) {
                if (!hasCreatures) {
                    printf("You see creatures:\n");
                    hasCreatures = 1;
                }
                printf("  %s\n", creatures[cidx].name);
            }
        }
    }
}

void do_inventory() {
    if (player.inventoryCount == 0) {
        printf("Your inventory is empty.\n");
        return;
    }
    printf("You have:\n");
    for (int i = 0; i < player.inventoryCount; i++) {
        printf("  %s\n", items[player.inventoryIndices[i]].name);
    }
}

void do_pickup(const char *itemName) {
    if (player.inventoryCount >= MAX_INVENTORY) {
        printf("Your inventory is full.\n");
        return;
    }

    int idx = find_item_in_room(player.currentRoom, itemName);
    if (idx == -1) {
        printf("No such item here.\n");
        return;
    }

    if (remove_item_from_room(player.currentRoom, idx) == 0) {
        add_item_to_player(idx);
        printf("You picked up %s.\n", itemName);
    } else {
        printf("Error picking up item.\n");
    }
}

void do_attack() {
    int creatureIndex = find_creature_in_room(player.currentRoom);
    if (creatureIndex == -1) {
        printf("No creature here to attack.\n");
        return;
    }

    Creature *c = &creatures[creatureIndex];
    printf("You attack the %s!\n", c->name);

    while (player.health > 0 && c->health > 0) {
        c->health -= player.strength;
        printf("You deal %d damage. The %s has %d health left.\n", player.strength, c->name, c->health);
        if (c->health <= 0) {
            printf("You have defeated the %s!\n", c->name);
            int count=0; while (rooms[player.currentRoom].creatureIndices[count]!=-1) count++;
            for (int i = 0; i < count; i++) {
                if (rooms[player.currentRoom].creatureIndices[i] == creatureIndex) {
                    for (int j = i; j < count; j++) {
                        rooms[player.currentRoom].creatureIndices[j] = rooms[player.currentRoom].creatureIndices[j+1];
                    }
                    break;
                }
            }
            break;
        }

        // Creature attacks back
        player.health -= c->strength;
        printf("The %s attacks you for %d damage. You have %d health left.\n", c->name, c->strength, player.health);
        if (player.health <= 0) {
            printf("You have been slain by the %s.\n", c->name);
            break;
        }
    }
}

void do_list_saves() {
    printf("No game saved yet.\n");
}

void do_save(const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        printf("Failed to save game.\n");
        return;
    }

    fprintf(fp, "%d %d %d\n", player.health, player.strength, player.currentRoom);
    fprintf(fp, "%d\n", player.inventoryCount);
    for (int i = 0; i < player.inventoryCount; i++) {
        fprintf(fp, "%d\n", player.inventoryIndices[i]);
    }

    fclose(fp);
    printf("Game saved to %s\n", filepath);
}

void do_load(const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("Failed to load game.\n");
        return;
    }

    if (fscanf(fp, "%d %d %d\n", &player.health, &player.strength, &player.currentRoom) != 3) {
        printf("Save file corrupted.\n");
        fclose(fp);
        return;
    }

    if (fscanf(fp, "%d\n", &player.inventoryCount) != 1) {
        printf("Save file corrupted.\n");
        fclose(fp);
        return;
    }

    for (int i = 0; i < MAX_INVENTORY; i++) {
        player.inventoryIndices[i] = -1;
    }

    for (int i = 0; i < player.inventoryCount; i++) {
        if (fscanf(fp, "%d\n", &player.inventoryIndices[i]) != 1) {
            printf("Save file corrupted.\n");
            fclose(fp);
            return;
        }
    }

    fclose(fp);
    printf("Game loaded from %s\n", filepath);
    do_look();
}

void do_exit_game() {
    printf("Exiting game.\n");
}

int find_item_by_name(const char *name) {
    for (int i = 0; i < numItems; i++) {
        if (strcasecmp(items[i].name, name) == 0) return i;
    }
    return -1;
}

int find_item_in_room(int roomIndex, const char *name) {
    Room *r = &rooms[roomIndex];
    if (!r->itemIndices) return -1;
    for (int i = 0; r->itemIndices[i] != -1; i++) {
        int idx = r->itemIndices[i];
        if (idx != -1 && strcasecmp(items[idx].name, name) == 0) return idx;
    }
    return -1;
}

int remove_item_from_room(int roomIndex, int itemIndex) {
    Room *r = &rooms[roomIndex];
    if (!r->itemIndices) return -1;
    int count = 0; 
    while (r->itemIndices[count] != -1) count++;

    for (int i = 0; i < count; i++) {
        if (r->itemIndices[i] == itemIndex) {
            for (int j = i; j < count; j++) {
                r->itemIndices[j] = r->itemIndices[j+1];
            }
            return 0;
        }
    }
    return -1;
}

void add_item_to_player(int itemIndex) {
    player.inventoryIndices[player.inventoryCount++] = itemIndex;
}

int find_creature_in_room(int roomIndex) {
    Room *r = &rooms[roomIndex];
    if (!r->creatureIndices) return -1;
    for (int i = 0; r->creatureIndices[i] != -1; i++) {
        int cidx = r->creatureIndices[i];
        if (cidx != -1 && creatures[cidx].health > 0) {
            return cidx;
        }
    }
    return -1;
}

int safe_read_line(FILE *fp, char *buffer, int size) {
    if (!fgets(buffer, size, fp)) {
        return 0;
    }
    trim_newline(buffer);
    return 1;
}

char *safe_read_string(FILE *fp) {
    char buffer[MAX_LINE_LEN];
    if (!fgets(buffer, MAX_LINE_LEN, fp)) {
        fprintf(stderr, "Error reading string.\n");
        exit(1);
    }
    trim_newline(buffer);
    return strdup(buffer);
}

void trim_newline(char *str) {
    char *p = str + strlen(str)-1;
    while (p >= str && (*p == '\n' || *p == '\r')) {
        *p = '\0';
        p--;
    }
}
