// FOVBackground.cpp

#include "FOVBackground.h"
#include "GameEngine.h" // Ekran boyutlar�n� almak i�in eklendi
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// YEN�: A��lar� yumu�ak bir �ekilde birle�tirmek i�in yard�mc� fonksiyon (d�nme tak�lmas�n� ��zer)
// �ki a�� aras�ndaki en k�sa yolu bularak interpolasyon yapar.
double LerpAngle(double a, double b, float t)
{
    double diff = b - a;
    // -PI ve PI aral���n� a�an d�n��leri d�zeltir
    if (diff > PI) diff -= 2 * PI;
    if (diff < -PI) diff += 2 * PI;
    return a + diff * t;
}


FOVBackground::FOVBackground(Sprite* pPlayer, int iFOVDegrees, int iFOVDistance)
    : m_pPlayer(pPlayer), m_iFOVDegrees(iFOVDegrees), m_iFOVDistance(iFOVDistance), m_dDirection(0.0)
{
    m_ptMouse.x = 0;
    m_ptMouse.y = 0;
}

// DE����KL�K: Fare hareketlerini ve fener a��s�n� yumu�atmak i�in Update g�ncellendi
SPRITEACTION FOVBackground::Update(int cameraX, int cameraY)
{
    if (m_pPlayer)
    {
        // Oyuncunun d�nya koordinatlar�ndaki merkezi
        RECT rcPos = m_pPlayer->GetPosition();
        int playerX = rcPos.left + (rcPos.right - rcPos.left) / 2;
        int playerY = rcPos.top + (rcPos.bottom - rcPos.top) / 2;

        // Fare pozisyonunu d�nya koordinatlar�na �evir
        int mouseWorldX = m_ptMouse.x + cameraX;
        int mouseWorldY = m_ptMouse.y + cameraY;

        // Hedef a��y� hesapla
        double targetDirection = atan2((double)mouseWorldY - playerY, (double)mouseWorldX - playerX);

        // YEN�: A��y� an�nda de�i�tirmek yerine yumu�ak ge�i� yap (fare tak�lmas�n� �nler)
        m_dDirection = LerpAngle(m_dDirection, targetDirection, 0.25f);
    }
    return SA_NONE;
}

// DE����KL�K: Draw metodu tamamen yeniden yaz�ld�. Art�k beyaz bir ��gen �izmek yerine,
// fenerin d���nda kalan alan� karartan bir katman �iziyor.
void FOVBackground::Draw(HDC hDC, int cameraX, int cameraY)
{
    if (m_pPlayer == NULL)
        return;

    GameEngine* pGame = GameEngine::GetEngine();
    int screenWidth = pGame->GetWidth();
    int screenHeight = pGame->GetHeight();

    // 1. Ge�ici bir haf�za DC'si ve karanl�k katman� i�in bitmap olu�tur
    HDC memDC = CreateCompatibleDC(hDC);
    HBITMAP memBitmap = CreateCompatibleBitmap(hDC, screenWidth, screenHeight);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // 2. Bu ge�ici katman� tamamen siyahla doldur
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT rect = { 0, 0, screenWidth, screenHeight };
    FillRect(memDC, &rect, blackBrush);
    DeleteObject(blackBrush);

    // 3. Oyuncunun ekrandaki pozisyonunu hesapla
    RECT rcPos = m_pPlayer->GetPosition();
    int playerScreenX = (rcPos.left + m_pPlayer->GetWidth() / 2) - cameraX;
    int playerScreenY = (rcPos.top + m_pPlayer->GetHeight() / 2) - cameraY;

    // 4. Fener konisinin silinece�i bir "�effaf" f�r�a olu�tur
    // Bu f�r�a, siyah katman �zerinde "delik a�mak" i�in kullan�lacak.
    HBRUSH transparentBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // Veya �zel bir desenli f�r�a
    HPEN   noPen = CreatePen(PS_NULL, 0, 0); // Kenar �izgisi olmas�n
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, transparentBrush);
    HPEN   oldPen = (HPEN)SelectObject(memDC, noPen);

    // 5. Daha ger�ek�i bir fener konisi i�in poligon noktalar�n� hesapla
    // YAKINDA DAR, UZAKTA GEN�� B�R Koni
    const int numPoints = 20; // Koninin kenarlar�n� daha p�r�zs�z yapmak i�in nokta say�s�
    POINT points[numPoints + 2];
    points[0] = { playerScreenX, playerScreenY }; // �lk nokta her zaman oyuncunun merkezi

    double halfFOV = (m_iFOVDegrees * PI / 180.0) / 2.0; // Radyan cinsinden
    double startAngle = m_dDirection - halfFOV;
    double endAngle = m_dDirection + halfFOV;
    double angleStep = (endAngle - startAngle) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i)
    {
        double currentAngle = startAngle + i * angleStep;
        points[i + 1].x = playerScreenX + (int)(cos(currentAngle) * m_iFOVDistance);
        points[i + 1].y = playerScreenY + (int)(sin(currentAngle) * m_iFOVDistance);
    }

    // 6. Bu poligonu kullanarak siyah katman �zerinde bir "b�lge" olu�turup silelim.
    // GDI'da transparanl�k karma��k oldu�undan, en kolay yol, bu alan� "beyaz" ile doldurup
    // sonra t�m katman� �zel bir ROP kodu ile birle�tirmektir.
    // VEYA DAHA BAS�T�: Bu alan� ba�ka bir renkle boyay�p TransparentBlt kullanmak.
    // ��MD�L�K EN BAS�T Y�NTEM: AlphaBlend.

    // Polygon'u "�effaf" (siyah) olmayan bir renkle (�rn: beyaz) �izelim.
    // Ard�ndan, bu katman� ekrana AlphaBlend ile �izerken,
    // kaynak ve hedef pikselleri aras�nda �zel bir i�lem yapaca��z.
    // Bu, GDI'nin en zorlu k�s�mlar�ndan biridir.
    // EN TEM�Z ��Z�M:
    // a) Siyah katman� �iz.
    // b) Fener poligonunu bu katmandan "sil". Bunun i�in bir RGN (b�lge) kullanaca��z.

    HRGN hRgn = CreatePolygonRgn(points, numPoints + 1, WINDING);
    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // Ge�ici olarak beyaz f�r�a
    FillRgn(memDC, hRgn, whiteBrush); // B�lgeyi beyaza boyayarak "delik" a��yoruz.
    DeleteObject(whiteBrush);
    DeleteObject(hRgn);


    // 7. Karanl�k katman�, beyaz k�s�mlar� (yani fenerin oldu�u yer) �effaf olacak �ekilde
    // ana ekrana (hDC) �iz.
    // TransparentBlt, beyaz rengi �effaf yapar ve kalan siyah� ekrana �izer.
    // Bu, siyah bir maske �zerine delik a�ma efekti verir.
    TransparentBlt(hDC, 0, 0, screenWidth, screenHeight,
        memDC, 0, 0, screenWidth, screenHeight, RGB(255, 255, 255));

    // Alternatif ve daha iyi g�r�nen y�ntem: AlphaBlend
    // Bu y�ntem, siyah katman� yar� saydam olarak �izer ve fener efekti verir.
    // BLENDFUNCTION blend = { AC_SRC_OVER, 0, 192, 0 }; // 192 = %75 opakl�k
    // AlphaBlend(hDC, 0, 0, screenWidth, screenHeight, memDC, 0, 0, screenWidth, screenHeight, blend);

    // 8. Temizlik
    SelectObject(memDC, oldBitmap);
    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(noPen);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}


void FOVBackground::Draw(HDC hDC)
{
    Draw(hDC, 0, 0);
}