# SPOOM

A small terminal arcade game written in C.

You control a ship and fight waves of enemies moving down the screen. Shoot them before they reach you.

## Features

* Terminal-based gameplay
* Multiple levels
* Player movement
* Shooting
* Enemy movement
* Collision detection
* Win and lose conditions
* Level restart
* Simple menu system

## Controls

| Key     | Action                    |
| ------- | ------------------------- |
| `A`     | Move left                 |
| `D`     | Move right                |
| `Enter` | Shoot                     |
| `R`     | Restart the current level |

## Building

Clone the repository and run:

```bash
cd build
./build.sh
```

Then start the game:

```bash
./program.o
```

## Project structure

```text
SPOOM/
├── inc/       # Header files
├── src/       # Source code
├── build/     # Build script and build output
├── README.md
├── LICENSE
└── .gitignore
```

## Built with

* C
* Clang
* Linux terminal

## License

SPOOM is licensed under the GNU General Public License v3.0.

See [LICENSE](LICENSE) for details.
