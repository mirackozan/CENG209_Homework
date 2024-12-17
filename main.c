#include "game.h"

int main() {
    init_game();
    printf("Welcome to the Dungeon Adventure!\n");
    printf("Type 'help' for a list of commands.\n");
    game_loop();
    cleanup_game();
    return 0;
}
