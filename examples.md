# Examples

## Rogue mode

    ./main.out -a HumanActor -v 0 -W 550

Here 550 assumes that the FOV is 5, which requires:

    5 + 1 (player itself) + 5 = 11

squares. And 550 is a multiple of 11 with of reasonable screen size.

The world only updates when the player hits a key.

## Rogue mode all seeing

    ./main.out

Cheat by being able to observe the entire world at once.

Impossible is there are so many tiles that each tile is smaller than a pixel.

## Non-interactive simulation

    ./main.out -d -t 100 -p 0 -V

Let your AIs battle it out for 10 ticks, print all scores to stdout, and quit.

No human intervention or GUI, everything happens as fast as possible.

This is the major production mode.

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

## Rogue-like TAS mode

    ./main.out -i

If a player controller is present, then the world blocks until player makes a
move (`SPACE`).

By TAS, we mean that each input can be carefully constructed, by parts, and the
world only advances when `SPACE` is pressed. in similar fashion to how Tool
Assisted Speedruns are developed

This mode may be too slow to be fun, but it allows for precise controls.

E.g., to move diagonally right up, you do:

- `RIGHT` (horizontal direction) `UP` (vertical direction) `SPACE`

The default for each type of direction is to do nothing. E.g., to just move up:

- `UP` (vertical direction) `SPACE`

Since there was no horizontal input, the default of not moving horizontally is
used.

You can fix some controls half-way. E.g. the following will move up:

- `DOWN` `UP`, overrides the previous `DOWN` `SPACE`
