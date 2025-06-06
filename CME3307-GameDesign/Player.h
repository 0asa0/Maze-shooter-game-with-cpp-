#pragma once
// Player.h
#pragma once
#include "Sprite.h"
#include "MazeGenerator.h"

class Player : public Sprite
{
public:
    Player(Bitmap* pBitmap, MazeGenerator* pMaze);

    virtual SPRITEACTION Update();
    void HandleInput(); // Klavye girdilerini burada i�leyece�iz

private:
    MazeGenerator* m_pMaze; // �arp��ma kontrol� i�in labirent referans�
    int m_iSpeed;
};