# Overview

This is a draft document on how to organize this project.

Pipeline: \
Program scanner -> Data recognition -> Data update -> Decision -> Program input

# Program I/O
Program
- (constructor)(string program_name)
- screenshot()
- input(key/click)

Classes: Input, Image


# Data recognition
Hexceed
- (constructor)(screenshot)
- _update(screenshot, area) ?
- grid, mines
- exec(action)

Info
- pos, type (reveal)


# Data
Level
- Grid<Cell> + Walls
- Mines // Mines left

Cell
- Type: Mine / Neighbour / Centered / Frontal / Linear / Countdown<Type> / Unknown(Hidden) / Empty(OutOfBounds)

Grid
- HexaGrid
- Wall List
- field_of_view(Cell)
  * Mine -> exc
  * Neighbour -> 6 cells minus walls
  * Centered -> cells | dist 2
  * Frontal -> triangle
  * Linear -> raycast linear
  * Countdown<Type> -> exc
  * Unknown -> exc
  * Empty -> exc


# Solver
Solver
- (constructor)
- Level
- update(List<Info> new_info)
- solve() -> action
- batch action -> opening multiple

Action
- (constructor)
- Type: Mine / Reveal
- Position on grid
