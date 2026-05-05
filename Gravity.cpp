#include "Gravity.h"

Gravity::Gravity()
    : gravAcc(1.f)
    , termVel(20.f)
    , jumpStr(-15.f)
    , grounded(false)
{}

Gravity::Gravity(float gravAcc, float termVel, float jumpStr)
    : gravAcc(gravAcc)
    , termVel(termVel)
    , jumpStr(jumpStr)
    , grounded(false)
{}

bool Gravity::checkFloor(char** lvl, int lvlH, int lvlW, int cellSize,
                          float x, float offsetY, int entityW, int entityH)
{
    // WHY: 3-point check same as tumblepop — left edge, center, right edge
    int row = (int)(offsetY + entityH) / cellSize;
    int lc  = (int)(x)                / cellSize;
    int mc  = (int)(x + entityW / 2)  / cellSize;
    int rc  = (int)(x + entityW)      / cellSize;

    // WHY: clamp indices — never read outside array bounds
    if (row < 0)     row = 0;
    if (row >= lvlH) row = lvlH - 1;
    if (lc  < 0)     lc  = 0;
    if (lc  >= lvlW) lc  = lvlW - 1;
    if (mc  < 0)     mc  = 0;
    if (mc  >= lvlW) mc  = lvlW - 1;
    if (rc  < 0)     rc  = 0;
    if (rc  >= lvlW) rc  = lvlW - 1;

    return (lvl[row][lc] == 'g' || lvl[row][mc] == 'g' || lvl[row][rc] == 'g');
}

void Gravity::update(float& x, float& y, float& velY, bool& onGround,
                     char** lvl, int lvlH, int lvlW, int cellSize,
                     int entityW, int entityH)
{
    if (lvl == nullptr) return;

    if (y < 0) {
        y        = 0;
        velY     = 0;
        onGround = false;
        this->grounded = false;
        return;
    }

    float offsetY = y + velY;

    // WHY: the fix — include velY==0 case so player standing still stays grounded
    // Without this, every frame: land→velY=0→movingDown=false→onGround=false→
    // gravity adds 1→falls 1px→snaps back. This makes onGround flicker every
    // frame, causing Space to randomly fail to jump.
    bool movingDownOrStill = (velY >= 0);

    bool floor = this->checkFloor(lvl, lvlH, lvlW, cellSize,
                                  x, offsetY, entityW, entityH);

    if (floor && movingDownOrStill) {
        // WHY: snap entity to exact block top — prevents sinking
        int row     = (int)(offsetY + entityH) / cellSize;
        if (row >= lvlH) row = lvlH - 1;
        y           = (float)(row * cellSize - entityH);
        velY        = 0;
        onGround    = true;
        this->grounded = true;
    } else {
        y           = offsetY;
        onGround    = false;
        this->grounded = false;
    }

    // WHY: accumulate gravity every airborne frame
    if (!onGround) {
        velY += this->gravAcc;
        if (velY > this->termVel) velY = this->termVel;
    }
}

void Gravity::jump(float& velY, bool& onGround) {
    if (onGround) {
        velY           = this->jumpStr;
        onGround       = false;
        this->grounded = false;
    }
}