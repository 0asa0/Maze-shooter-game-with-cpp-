// Missile.cpp
#include "Missile.h"

// Constructor: Ba�lang�� pozisyonunu ve float h�z vekt�r�n� al�r
Missile::Missile(Bitmap* pBitmap, RECT& rcBounds, POINT ptPosition, float fVelocityX, float fVelocityY)
    : Sprite(pBitmap, ptPosition, { 0, 0 }, 0, rcBounds, BA_DIE, SPRITE_TYPE_PLAYER_MISSILE)
{
    // Sprite'�n kendi h�z�n� s�f�rl�yoruz, ��nk� kendi h�z mant���m�z� kullanaca��z.
    SetVelocity(0, 0);

    // Float konum ve h�z de�i�kenlerini ba�lat
    m_fPositionX = (float)ptPosition.x;
    m_fPositionY = (float)ptPosition.y;
    m_fVelocityX = fVelocityX;
    m_fVelocityY = fVelocityY;
}

// Kendi ak�c� hareket mant���m�z� i�eren Update metodu
SPRITEACTION Missile::Update()
{
    // Delta time'� varsayal�m (1.0f / 60.0f).
    // Bu de�erin oyun motorundan gelmesi en idealidir.
    float fDeltaTime = 1.0f / 60.0f;

    // Yeni pozisyonu hesapla: Pozisyon += H�z * Zaman
    m_fPositionX += m_fVelocityX * fDeltaTime;
    m_fPositionY += m_fVelocityY * fDeltaTime;

    // Sprite'�n tamsay� pozisyonunu g�ncelle (�izim ve �arp��ma i�in)
    SetPosition((int)m_fPositionX, (int)m_fPositionY);

    // Sprite'�n normal update'ini �a��r (s�n�r kontrol� ve �lme durumu i�in)
    // Ama kendi pozisyon g�ncellememizi yapt���m�z i�in, base class'�nkini
    // direkt �a��rmak yerine sadece s�n�r kontrol�n� yapabiliriz.
    // �imdilik, Sprite::Update() pozisyonu tekrar de�i�tirece�i i�in bunu �a��rmayal�m
    // ve s�n�r kontrol�n� manuel yapal�m.

    // S�n�r kontrol� (BA_DIE i�in)
    if ((m_rcPosition.right < m_rcBounds.left ||
        m_rcPosition.left > m_rcBounds.right ||
        m_rcPosition.bottom < m_rcBounds.top ||
        m_rcPosition.top > m_rcBounds.bottom))
    {
        return SA_KILL;
    }

    // Animasyon ve �lme durumunu kontrol et
    UpdateFrame();
    if (m_bDying)
        return SA_KILL;

    return SA_NONE;
}