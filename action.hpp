#ifndef ACTION_HPP
#define ACTION_HPP

class Action {
    public:
        enum class MoveX {
            NONE,
            LEFT,
            RIGHT
        };
        enum class MoveY {
            NONE,
            DOWN,
            UP
        };
        Action();
        Action(Action::MoveX moveX, Action::MoveY moveY);
        MoveX getMoveX() const;
        MoveY getMoveY() const;
        void reset();
        void setMoveX(Action::MoveX x);
        void setMoveY(Action::MoveY y);
    private:
        MoveX moveX;
        MoveY moveY;
};

#endif
