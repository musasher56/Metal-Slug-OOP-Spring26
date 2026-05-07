#pragma once



class Gravity {
public:
    Gravity();
    Gravity(float gravAcc, float termVel, float jumpStr);

    
    void update(float& x, float& y, float& velY, bool& onGround,
                char** lvl, int lvlH, int lvlW, int cellSize,
                int entityW, int entityH);

    void jump(float& velY, bool& onGround);

    bool isOnGround() const { return this->grounded; }

private:
    float gravAcc;   
    float termVel;   
    float jumpStr;   
    bool  grounded;  

    
    bool checkFloor(char** lvl, int lvlH, int lvlW, int cellSize,
                    float x, float offsetY, int entityW, int entityH);
};