#pragma once
#include "Soldier.h"
#include "ProjectileManager.h"

class PlayerSoldier;

class Enemy : public Soldier {
protected:
    int aiState;
    float detectionRange;
    float attackRange;
    float attackCooldown;
    Clock attackTimer;
    ProjectileManager* pm;
    bool activated;
    float patrolCenter;
    float patrolRadius;
    int enemyType;

    Animation walkAnim;
    Animation shootAnim;
    Animation deathAnim;
    Animation* currentAnim;

    int frameW;
    int frameH;
    int walkFrames;
    int shootFrames;
    int deathFrames;
    bool dying;

    int baseFrameW;
    int baseFrameH;
    Clock deathTimer;
    float deathDuration;

    float deathSpriteScale;
    bool faceRight;

    void switchAnim(Animation* newAnim);
    void applyDirectionFlip();

public:
    Enemy(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Enemy();

    void setProjectileManager(ProjectileManager* manager);
    void setPatrol(float centerX, float radius);
    void setEnemyType(int type);

    bool isActivated() const;
    bool isDying() const;
    float distanceTo(PlayerSoldier* player) const;
    bool playerInRange(PlayerSoldier* player, float range) const;

    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void onDeath();
    void updateBoundingBox();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void handleCollision(Level* lvl);

    int getAIState() const;
    int getEnemyType() const;
    void takeDamage(int amount);
};

class RebelSoldier : public Enemy {
public:
    RebelSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~RebelSoldier();
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
};

class BazookaSoldier : public Enemy {
public:
    BazookaSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~BazookaSoldier();
    virtual void performAttack(PlayerSoldier* player);
};

class ShieldedSoldier : public Enemy {
private:
    bool hasShield;
    int  shieldHP;

public:
    ShieldedSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~ShieldedSoldier();
    virtual void performAttack(PlayerSoldier* player);
    virtual void takeDamageFrom(int amount, int bulletDir);
};

class GrenadeSoldier : public Enemy {
public:
    GrenadeSoldier(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~GrenadeSoldier();
    virtual void performAttack(PlayerSoldier* player);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};

class Martian : public Enemy {
private:
    bool inPodPhase;
    int  podHP;

public:
    Martian(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Martian();
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};

class Paratrooper : public Enemy {
private:
    int paraState;       
    float landY;         
    float fallSpeed;     
    Animation flyAnim;   
    float swayTimer;     
    bool startDescent;   
    float triggerX;      

protected:
    virtual void applyGravity();

public:
    Paratrooper(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Paratrooper();
    void setLandY(float y);
    void setTriggerX(float x);
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void onDeath();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void handleCollision(Level* lvl);
};








class Boss : public Enemy {
protected:
    int   bossPhase;          
    float phase2Threshold;    
    const char* bossName;     
    bool  entranceDone;       
    float entranceTimer;      

    Animation idleAnim;       
    Animation chargeAnim;     
    Animation specialAnim;    

    float specialCooldown;    
    Clock specialTimer;       
    float chargeCooldown;     
    Clock chargeTimer;        
    float chargeSpeed;        
    bool  isCharging;         
    float chargeDuration;     
    float chargeElapsed;      

public:
    Boss(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Boss();

    const char* getBossName() const;
    float getHealthFraction() const;  
    bool  isEntranceDone() const;

    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void onDeath();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};






class Ironokava : public Boss {
public:
    Ironokava(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Ironokava();
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
};








class Hairbuster : public Boss {
private:
    float flyCenterX;        
    float flyCenterY;        
    float flyRadiusX;        
    float flyRadiusY;        
    float flyAngle;          
    float flySpeed;          
    float diveTargetX;       
    float diveTargetY;       
    bool  isDiving;          
    float diveSpeed;         
    float diveTimer;         
    float diveDuration;      
    float bombCooldown;      
    Clock bombTimer;         

protected:
    virtual void applyGravity();  

public:
    Hairbuster(TextureManager* texMgr, AudioManager* audMgr);
    virtual ~Hairbuster();
    void  setFlyCenter(float cx, float cy);
    virtual void updateAI(PlayerSoldier* player, Level* lvl);
    virtual void performAttack(PlayerSoldier* player);
    virtual void onDeath();
    virtual void draw(RenderWindow& window, float scrollX, float scrollY);
    virtual void handleCollision(Level* lvl);
};