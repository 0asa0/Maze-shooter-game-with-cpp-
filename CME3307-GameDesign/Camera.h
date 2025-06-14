// Camera.h

#pragma once
#include <windows.h>
#include "Sprite.h" // Sprite'� takip edebilmesi i�in eklendi

class Camera
{
public:
    int x, y;           // Kameran�n d�nya koordinatlar�ndaki sol �st k��esi
    int width, height;  // Kameran�n boyutu (g�r�� alan�)

    // DE����KL�K: Kurucu metod art�k takip edilecek bir hedef al�yor.
    Camera(Sprite* target, int w = 640, int h = 480);

    // DE����KL�K: Kameran�n yumu�ak hareketini sa�layacak Update fonksiyonu
    void Update();

    // Orijinal fonksiyonlar yerinde duruyor
    void Move(int dx, int dy) { x += dx; y += dy; }
    void SetPosition(int newX, int newY) { x = newX; y = newY; }
    RECT GetRect() const { return { x, y, x + width, y + height }; }

private:
    Sprite* m_pTarget;          // Takip edilecek sprite (oyuncu)
    float   m_fLerpFactor;      // Yumu�atma fakt�r� (0.0 ile 1.0 aras�)
    float   m_fCurrentX, m_fCurrentY; // Ak�c� hareket i�in float pozisyonlar
};