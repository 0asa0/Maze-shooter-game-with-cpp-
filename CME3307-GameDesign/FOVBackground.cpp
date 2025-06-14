// FOVBackground.cpp

#include "FOVBackground.h"
#include "GameEngine.h"
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846
#endif

double LerpAngle(double a, double b, float t)
{
    double diff = b - a;
    if (diff > PI) diff -= 2 * PI;
    if (diff < -PI) diff += 2 * PI;
    return a + diff * t;
}

// YEN�: Kurucu metot g�ncellendi
FOVBackground::FOVBackground(Sprite* pPlayer, int iFOVDegrees, int iFOVDistance, int iPlayerLightRadius)
    : m_pPlayer(pPlayer),
    m_iFOVDegrees(iFOVDegrees),
    m_iFOVDistance(iFOVDistance),
    m_dDirection(0.0),
    m_iPlayerLightRadius(iPlayerLightRadius) // YEN�: �ye de�i�keni ba�lat
{
    m_ptMouse.x = 0;
    m_ptMouse.y = 0;
}

SPRITEACTION FOVBackground::Update(int cameraX, int cameraY)
{
    if (m_pPlayer)
    {
        RECT rcPos = m_pPlayer->GetPosition();
        int playerX = rcPos.left + (rcPos.right - rcPos.left) / 2;
        int playerY = rcPos.top + (rcPos.bottom - rcPos.top) / 2;

        int mouseWorldX = m_ptMouse.x + cameraX;
        int mouseWorldY = m_ptMouse.y + cameraY;

        double targetDirection = atan2((double)mouseWorldY - playerY, (double)mouseWorldX - playerX);
        m_dDirection = LerpAngle(m_dDirection, targetDirection, 0.25f);
    }
    return SA_NONE;
}

void FOVBackground::Draw(HDC hDC, int cameraX, int cameraY)
{
    if (m_pPlayer == NULL)
        return;

    GameEngine* pGame = GameEngine::GetEngine();
    if (!pGame) return; // pGame null ise ��k
    int screenWidth = pGame->GetWidth();
    int screenHeight = pGame->GetHeight();

    // 1. Ge�ici haf�za DC'si ve karanl�k katman� i�in bitmap olu�tur
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
    // Oyuncu sprite'�n�n geni�lik ve y�ksekli�ini almam�z gerek.
    // Sprite s�n�f�nda GetWidth() ve GetHeight() zaten var.
    int playerSpriteWidth = 0;
    int playerSpriteHeight = 0;
    if (m_pPlayer->GetBitmap()) { // Bitmap null de�ilse boyutlar� al
        playerSpriteWidth = m_pPlayer->GetWidth();
        playerSpriteHeight = m_pPlayer->GetHeight();
    }

    int playerScreenX = (rcPos.left + playerSpriteWidth / 2) - cameraX;
    int playerScreenY = (rcPos.top + playerSpriteHeight / 2) - cameraY;


    // 4. �effafl�k i�in kullan�lacak beyaz f�r�a (TransparentBlt bu rengi �effaf yapacak)
    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, whiteBrush); // Beyaz f�r�ay� se�
    HPEN   noPen = CreatePen(PS_NULL, 0, 0); // Kenar �izgisi olmas�n
    HPEN   oldPen = (HPEN)SelectObject(memDC, noPen);


    // 5. FENER KON�S� B�LGES�N� OLU�TUR VE BEYAZA BOYA (�EFFAF YAP)
    const int numConePoints = 20;
    POINT conePoints[numConePoints + 2]; // Polygon i�in nokta dizisi (merkez + kenar noktalar�)
    conePoints[0] = { playerScreenX, playerScreenY };

    double halfFOV = (m_iFOVDegrees * PI / 180.0) / 2.0;
    double startAngle = m_dDirection - halfFOV;
    // double endAngle = m_dDirection + halfFOV; // Bu kullan�lm�yor, angleStep i�in yeterli
    double angleStep = (2.0 * halfFOV) / (numConePoints - 1); // Toplam FOV a��s� / (nokta say�s� - 1)

    for (int i = 0; i < numConePoints; ++i)
    {
        double currentAngle = startAngle + i * angleStep;
        conePoints[i + 1].x = playerScreenX + (int)(cos(currentAngle) * m_iFOVDistance);
        conePoints[i + 1].y = playerScreenY + (int)(sin(currentAngle) * m_iFOVDistance);
    }

    // Fener konisi i�in bir Region olu�tur
    HRGN hConeRgn = CreatePolygonRgn(conePoints, numConePoints + 1, WINDING);

    // 6. YEN�: OYUNCU ETRAFINDAK� DA�RESEL I�IK ALANI ���N B�LGE OLU�TUR
    HRGN hCircleRgn = CreateEllipticRgn(
        playerScreenX - m_iPlayerLightRadius,
        playerScreenY - m_iPlayerLightRadius,
        playerScreenX + m_iPlayerLightRadius,
        playerScreenY + m_iPlayerLightRadius
    );

    // 7. YEN�: �ki b�lgeyi birle�tir (OR i�lemi ile)
    // �nce birle�ik b�lgeyi tutacak bo� bir RGN olu�turmak iyi bir pratiktir.
    HRGN hCombinedRgn = CreateRectRgn(0, 0, 1, 1); // Ge�ici k���k bir b�lge
    int combineResult = CombineRgn(hCombinedRgn, hConeRgn, hCircleRgn, RGN_OR);

    // 8. B�RLE��K B�LGEY� BEYAZA BOYA (�EFFAF YAP)
    if (combineResult != ERROR && combineResult != NULLREGION) {
        FillRgn(memDC, hCombinedRgn, whiteBrush);
    }
    else { // E�er combine ba�ar�s�z olursa veya bo� b�lge d�nerse, ayr� ayr� boya
        FillRgn(memDC, hConeRgn, whiteBrush);
        FillRgn(memDC, hCircleRgn, whiteBrush); // Daireyi de beyazla doldur
    }

    // Olu�turulan GDI nesnelerini sil
    DeleteObject(hConeRgn);
    DeleteObject(hCircleRgn);
    DeleteObject(hCombinedRgn); // Birle�ik b�lgeyi de sil

    // 9. Karanl�k katman�, beyaz k�s�mlar� (fener ve oyuncu �����) �effaf olacak �ekilde
    // ana ekrana (hDC) �iz.
    TransparentBlt(hDC, 0, 0, screenWidth, screenHeight,
        memDC, 0, 0, screenWidth, screenHeight, RGB(255, 255, 255));

    // 10. Temizlik
    SelectObject(memDC, oldBitmap);
    SelectObject(memDC, oldBrush); // Eski f�r�ay� geri y�kle
    SelectObject(memDC, oldPen);   // Eski kalemi geri y�kle
    DeleteObject(whiteBrush);      // Olu�turulan beyaz f�r�ay� sil
    DeleteObject(noPen);           // Olu�turulan kalemi sil
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

// Bu Draw metodu genellikle kamera olmadan �a�r�lmaz ama yine de b�rak�yorum.
void FOVBackground::Draw(HDC hDC)
{
    Draw(hDC, 0, 0);
}