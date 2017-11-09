# CLI options

## Interactive play options

-   `-a <string>`, `-showFovActorType <string>`: only consider objects of the given
    type for the `-showFovId` option.

-   `-b`: (Block) don't block on player input.

    If given, if he player does not give any input until before the current
    frame is over, and empty input is used, and the simulation progresses.

    Not setting this option makes the game more Rogue-like.

-   `-d`: (Display) turn display off. Might make simulation faster or runnable
    in device without display.

    User input is only available with display.

    If not present, simulation runs as fast as possible.

    Implies `-p 0`, since it is currently impossible to control players without a
    display.

-   `-f <double>`: (Fps) limit FPS to `double` FPS. Default: -1.0.

    -1.0 means unlimited.

    Helps humans visualize non-interactive simulations that are too fast. E.g.
    `-f 2.0` limits simulation to 2 FPS.

    You likely don't want this for interactive simulations that block on user
    input (Rogue-like), as this becomes lag.

-   `-H`: (Hold key) actions are taken if the player is holding at the end of a
    frame, a click is not needed.

    The simulation advances automatically if the player is holding any key at
    the end of the frame.

    If a click action (-i) was taken during the previous frame, it gets
    overridden if any key is held at the en of the frame.

    Holding SPACE makes the simulation advance with an empty action.

    Makes the game more interactive, and controls less precise.

    By hardware limitation, some combinations of keys may be impossible, while
    similar ones are possible, e.g. Right + Up + Space vs Left + Up + Space on
    Lenovo keyboards:
    <http://unix.stackexchange.com/questions/268850/leftupspace-keys-not-working-on-thinkpad-x201>

    Only the first human player is affected by this option, since it is
    impossible to give different controls to different players (the only two
    design choices are: affect only first or affect all equally, and we chose
    N.1).

-   `-i`: (toggle Immediate) mode. Default: on.

    If on, create action immediately whenever the user presses any key, without
    waiting for `SPACE` to be pressed.

    Makes game more interactive, and less precisely controllable.

    In particular, it becomes difficult to press multiple simultaneous keys
    consistently.

-   `-P <x> <y>`: initial Position of the game window in the screen at startup

-   `-S <string>`: (Scenario) choose a named pre-built world scenario. TODO way
    to show scenario list here. For now read source.

-   `-s <int>`, `-showFovId <int>`: Only show the Field Of Vision
    for the `int`-th object, i.e., what the i-th object can see.

    If not given, show a global world view. This means hat you are forced to use
    this opion if the world is so large that each tile would be less than one pixel wide.

    If `-showFovActorType` is given, then `int` refers to the `int`-th object that
    is controlled by an actor of the given type.

    You do not need to be controlling the observed player: in particular if
    there are 0 players to be controlled by keyboard, you can just watch the
    action unroll by itself. You can also control one player while observing
    another, but you will likely go nuts.

    This opion also determines which object will have its information shown on the HUD.
    If neither `-s` nor `-a` is given, the HUD shows data for the first `HumanActor` object.

    Default: 0

-   `-W <int>`: (Width) window width in pixels. Square windows only. Must be
    divisible by the width of the world. Default: 500.

## World state options

-   `-p`: now many human players should be added to the map. Default: 1.

    With 0 you watch computer bots fight.

-   `-r`: (Random) set a fixed random seed.

    This is the only source of randomness in the whole engine. Fixing it to a
    given value gives reproducible games.

-   `-t`: (Time limit):

    -   without `-d`, pause the simulation after this many steps. This shows the
        score on the screen. You can then continue the simulation with `P` if
        you want, just like any regular game pause.
    -   otherwise, quit immediately after this many steps

    `-1` means infinite. Default value: `-1`.

-   `-w <int>`: (Width) world width in tiles. Default: 20.

## Debug options

-   `-h`: (help) show this help

-   `-S`: Don't spawn any new objects during gameplay. Useful for debugging.

-   `-V`: (Verbose) Show debug and performance information.
