#pragma once

// WHY: Gravity is its own class so Player, Enemy, Vehicle all reuse it
// Just give any entity a Gravity member and call update() each frame
class Gravity {
public:
    Gravity();
    Gravity(float gravAcc, float termVel, float jumpStr);

    // WHY: call this every frame — pass entity's x/y/width/height + level grid
    void update(float& x, float& y, float& velY, bool& onGround,
                char** lvl, int lvlH, int lvlW, int cellSize,
                int entityW, int entityH);

    void jump(float& velY, bool& onGround);

    bool isOnGround() const { return this->grounded; }

private:
    float gravAcc;   // acceleration per frame (like tumblepop's gravity=1)
    float termVel;   // max fall speed cap
    float jumpStr;   // initial upward velocity on jump (negative = up)
    bool  grounded;  // internal state mirror

    // WHY: check if any of the 3 bottom points land on a solid block
    bool checkFloor(char** lvl, int lvlH, int lvlW, int cellSize,
                    float x, float offsetY, int entityW, int entityH);
};