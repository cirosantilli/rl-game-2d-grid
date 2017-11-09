# Examples

## Rogue mode

    ./main.out

The world only updates when the player hits a key, as in roguelike games.

You cheat by being able to observe the entire world at once, while actual bots
can only see up to a certain distance around themselves.

Cannot be used if there are so many tiles that each tile is smaller than a pixel.

## Rogue mode limited FOV

    ./main.out -a HumanActor -s 0 -W 550

Limit screen view to what agents can actually see.

550 assumes that the FOV is 5, which requires:

    5 (left) + 1 (player itself) + 5 (right) = 11

squares. And 550 is a multiple of 11 of reasonable screen size.

## Headless simulation

    ./main.out -d -t 100 -p 0 -v

Let your AIs battle it out for 100 ticks, print all scores to stdout, and quit.

No human intervention or GUI, everything runs as fast as possible without
GUI performance interference, only limited by the simulation speed.

This is the major production training mode.

## Simulation visualization

    ./main.out -p 0 -f 2.0

Watch bots fight it out on the GUI, limit FPS to 2.

## Rogue TAS

    ./main.out -i

If a player controller is present, then the world blocks until player makes a
move (`SPACE`).

By TAS, we mean that each input can be carefully constructed, by parts, and the
world only advances when `SPACE` is pressed. in similar fashion to how Tool
Assisted Speedruns are developed

This mode may be too slow to be fun, but it allows for precise controls.

E.g., to move diagonally right up, you can press and release in sequence:

- `RIGHT` (horizontal direction)
- `UP` (vertical direction)
- `SPACE`

The default for each type of direction is to do nothing. E.g., to just move up:

- `UP` (vertical direction)
- `SPACE`

Since there was no horizontal input this time, the default of not moving
horizontally is used.

You can fix some controls half-way. E.g. the following will move up:

- `DOWN` and  `UP` overrides the previous `DOWN` `SPACE`

## Crypt of the NecroDancer mode

    ./main.out -H -b -f 2.0

World updates even if user does nothing, but only at 2 FPS.

## Continuous action game

    ./main.out -H -b -f 20.0 -w 100

Same as Crypt of the NecroDancer, but with wide screen, and faster FPS.

Most engines will implement this mode with floating point positions, but it
could also be done with integers. However, we map states 1-to-1 with screen
while using floating point means that multiple states map to a single screen
after rounding.
