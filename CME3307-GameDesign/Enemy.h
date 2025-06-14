#pragma once
#include "Sprite.h"
#include "MazeGenerator.h"
#include <vector>
#include <windows.h>

// Forward declaration
class GameEngine;
extern int TILE_SIZE;

enum class EnemyType { CHASER, TURRET };
enum class AIState { IDLE, CHASING, ATTACKING };

class Enemy : public Sprite
{
public:
    // HAREKET HATASI D�ZELT�LD�: Kurucu metoda harita s�n�rlar� eklendi.
    Enemy(Bitmap* pBitmap, RECT& rcBounds, BOUNDSACTION baBoundsAction,
        MazeGenerator* pMaze, Sprite* pPlayer, EnemyType type);
    EnemyType GetEnemyType() const { return m_type; };
    virtual SPRITEACTION Update();

private:
    void UpdateAI();
    bool FindPath();
    void FollowPath();
    bool HasLineOfSightToPlayer();
    void AttackPlayer();

    MazeGenerator* m_pMaze;
    Sprite* m_pPlayer;
    EnemyType m_type;
    AIState m_state;

    std::vector<POINT> m_path;
    int m_pathIndex;
    int m_attackCooldown;

    // PERFORMANS OPT�M�ZASYONU: Yol bulma i�lemini s�n�rlamak i�in bir saya�
    int m_pathfindingCooldown;
};