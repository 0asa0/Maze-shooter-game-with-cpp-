// Missile.h
#pragma once
#include "Sprite.h"

class Missile : public Sprite
{
public:
    // Constructor art�k h�z i�in POINT yerine float vekt�r alacak
    Missile(Bitmap* pBitmap, RECT& rcBounds, POINT ptPosition, float fVelocityX, float fVelocityY);

    // Update fonksiyonunu override ederek kendi ak�c� hareket mant���m�z� ekleyece�iz
    virtual SPRITEACTION Update();

private:
    // Ak�c� hareket i�in float konum ve h�z de�i�kenleri
    float m_fPositionX;
    float m_fPositionY;
    float m_fVelocityX;
    float m_fVelocityY;
};