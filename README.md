# CENG209 Dungeon Adventure Game

**Name:** Miraç Kozan  
**Number:** 20050111009  
**Course:** CENG209 

## Overview

This project is a text-based adventure game implemented in C. The game simulates navigating through a dungeon, collecting items, encountering creatures, and saving/loading game states. It is played entirely via command-line interface commands typed at a custom shell prompt.

## Start

To create executable file, run "gcc -o game main.c game.c". Then run "./game".

## Features

- **Navigation:** Move through interconnected rooms using commands like `move up/down/left/right`.
- **Room Descriptions:** Each room has a name, a description, and may contain items or creatures.
- **Items:** Pick up items from the room and carry them in your inventory. The inventory has a limited capacity.
- **Creatures and Combat:** Some rooms contain creatures that you can engage in simple turn-based combat with the `attack` command.
- **Save/Load Game State:** You can save your progress to a file and load it later to continue.
- **File-based Data:** Room descriptions, items, and creatures are defined in external text files for easy customization.

## Commands

- `help`: Shows the list of available commands.
- `move <direction>`: Moves the player in the given direction (`up`, `down`, `left`, `right`) if possible.
- `look`: Displays the current room’s description, items, and creatures.
- `inventory`: Lists all items currently in your inventory.
- `pickup <item>`: Picks up an item from the current room (if available and if inventory space permits).
- `attack`: Initiates combat if there is a creature in the room.
- `list`: Lists saved game files (dummy command for demonstration).
- `save <filepath>`: Saves the current game state to the specified file.
- `load <filepath>`: Loads a previously saved game state from the specified file.
- `exit`: Exits the game.

## File Structure

- **items.txt**: Contains the list of items (name and description).
- **creatures.txt**: Contains the list of creatures (name, health, strength).
- **rooms.txt**: Defines the rooms, their descriptions, contained items, creatures, and room connections.

Ensure these files are in the same directory as the executable.

