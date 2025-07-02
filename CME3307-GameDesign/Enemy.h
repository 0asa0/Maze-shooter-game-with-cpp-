#pragma once
#include "Sprite.h"
#include "MazeGenerator.h"
#include <vector>
#include <windows.h>

// Forward declaration
class GameEngine;
extern int TILE_SIZE;

// YEN�: Robot Turret d��man tipi eklendi
enum class EnemyType { CHASER, TURRET, ROBOT_TURRET, RANDOM_WALKER };
enum class AIState { IDLE, CHASING, ATTACKING };

class Enemy : public Sprite
{
public:
    Enemy(Bitmap* pBitmap, RECT& rcBounds, BOUNDSACTION baBoundsAction,
        MazeGenerator* pMaze, Sprite* pPlayer, EnemyType type);
    EnemyType GetEnemyType() const { return m_type; };
    virtual SPRITEACTION Update(); // Override edece�iz
    virtual Sprite* AddSprite(); // YEN�: �l�m sprite'� yaratmak i�in override ediyoruz.
    // YEN�: D��manlar�n can y�netimi i�in fonksiyonlar
    void TakeDamage(int amount);
    bool IsDead() const;
    int GetHealth() const { return m_iHealth; } // �ste�e ba�l�, debug veya UI i�in

private:
    void UpdateAI();
    bool FindPath();
    void FollowPath();
    bool HasLineOfSightToPlayer();
    void AttackPlayer();

    void ResolveWallCollisions(POINT& desiredVelocity);

    MazeGenerator* m_pMaze;
    Sprite* m_pPlayer;
    EnemyType m_type;
    AIState m_state;

    std::vector<POINT> m_path;
    int m_pathIndex;
    int m_attackCooldown;
    int m_pathfindingCooldown;

    int m_randomMoveTimer;
    POINT m_randomMoveDirection;

    // YEN�: D��man can�
    int m_iHealth;
};