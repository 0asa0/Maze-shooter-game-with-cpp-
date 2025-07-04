// FOVBackground.h

#pragma once

#include "GameEngine.h" // GameEngine::GetEngine() i�in ve dolay�s�yla ekran boyutlar� i�in
#include "Sprite.h"

class FOVBackground
{
protected:
    Sprite* m_pPlayer;
    int m_iFOVDegrees;
    int m_iFOVDistance;
    double m_dDirection;
    POINT m_ptMouse;
    int m_iPlayerLightRadius; // YEN�: Oyuncu etraf�ndaki sabit ���k alan� yar��ap�

public:
    // YEN�: Kurucu metoda playerLightRadius eklendi
    FOVBackground(Sprite* pPlayer, int iFOVDegrees = 150, int iFOVDistance = 400, int iPlayerLightRadius = 50);
    virtual ~FOVBackground() {}

    virtual void Draw(HDC hDC);
    virtual void Draw(HDC hDC, int cameraX, int cameraY);

    virtual SPRITEACTION Update(int cameraX, int cameraY);
    void UpdateMousePos(int x, int y) { m_ptMouse.x = x; m_ptMouse.y = y; }
};