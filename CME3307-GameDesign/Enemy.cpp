// Enemy.cpp

#include "Enemy.h"
#include "Game.h"       // _pEnemyMissileBitmap ve game_engine i�in
#include "GameEngine.h" // game_engine i�in (Game.h zaten bunu i�eriyor olabilir ama do�rudan eklemek daha g�venli)
#include <cmath>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>

// A* Pathfinding (Bu k�s�m ayn� kal�yor)
namespace Pathfinder {
    struct Node { int x, y, gCost, hCost; Node* parent; Node(int _x, int _y) : x(_x), y(_y), gCost(0), hCost(0), parent(nullptr) {} int fCost() const { return gCost + hCost; } };
    struct CompareNode { bool operator()(const Node* a, const Node* b) const { return a->fCost() > b->fCost(); } };
    int calculateHCost(int x1, int y1, int x2, int y2) { return abs(x1 - x2) + abs(y1 - y2); }
    std::vector<POINT> reconstructPath(Node* endNode) { std::vector<POINT> p; Node* c = endNode; while (c != nullptr) { p.push_back({ c->x, c->y }); c = c->parent; } std::reverse(p.begin(), p.end()); return p; }
    std::vector<POINT> FindPath(MazeGenerator* maze, POINT start, POINT end) {
        if (!maze) return {}; // maze null ise bo� yol d�nd�r
        std::priority_queue<Node*, std::vector<Node*>, CompareNode> o; std::map<std::pair<int, int>, Node*> a;
        Node* s = new Node(start.x, start.y); s->hCost = calculateHCost(start.x, start.y, end.x, end.y); o.push(s); a[{start.x, start.y}] = s;
        int dx[] = { 0, 0, 1, -1 }, dy[] = { 1, -1, 0, 0 };
        while (!o.empty()) {
            Node* c = o.top(); o.pop();
            if (c->x == end.x && c->y == end.y) { std::vector<POINT> p = reconstructPath(c); for (auto i = a.begin(); i != a.end(); ++i) delete i->second; a.clear(); return p; }
            for (int i = 0; i < 4; ++i) {
                int nX = c->x + dx[i], nY = c->y + dy[i];
                // IsWall �a�r�s�ndan �nce maze null kontrol� zaten FindPath ba��nda yap�ld�.
                if (maze->IsWall(nX, nY)) continue;
                int nG = c->gCost + 1; auto it = a.find({ nX, nY });
                if (it == a.end() || nG < it->second->gCost) {
                    Node* nN; if (it == a.end()) { nN = new Node(nX, nY); a[{nX, nY}] = nN; }
                    else { nN = it->second; }
                    nN->parent = c; nN->gCost = nG; nN->hCost = calculateHCost(nX, nY, end.x, end.y); o.push(nN);
                }
            }
        }
        for (auto i = a.begin(); i != a.end(); ++i) delete i->second; a.clear(); return {};
    }
}


Enemy::Enemy(Bitmap* pBitmap, RECT& rcBounds, BOUNDSACTION baBoundsAction,
    MazeGenerator* pMaze, Sprite* pPlayer, EnemyType type)
    : Sprite(pBitmap, rcBounds, baBoundsAction, SPRITE_TYPE_ENEMY), m_pMaze(pMaze), m_pPlayer(pPlayer), m_type(type)
{
    m_state = AIState::IDLE;
    m_pathIndex = 0;
    m_attackCooldown = 0;
    m_pathfindingCooldown = 0;

    Sprite::SetNumFrames(4); // D��man animasyonu i�in frame say�s� (varsa)
    Sprite::SetFrameDelay(8); // Animasyon h�z�
}

SPRITEACTION Enemy::Update()
{
    UpdateAI();
    return Sprite::Update(); // Sprite'�n temel Update'ini �a��r (hareket, animasyon, s�n�r kontrol�)
}

void Enemy::UpdateAI()
{
    if (m_attackCooldown > 0) m_attackCooldown--;
    if (m_pathfindingCooldown > 0) m_pathfindingCooldown--;

    if (!m_pPlayer || !game_engine) { // game_engine null kontrol� eklendi
        SetVelocity(0, 0);
        return;
    }

    // TILE_SIZE global de�i�kenini kullan�yoruz, null/s�f�r olmamal�.
    // Bu kontrol� Game.cpp'de TILE_SIZE'�n atand��� yerde yapmak daha mant�kl� olabilir.
    // �imdilik burada bir g�venlik �nlemi olarak b�rak�yorum.
    if (TILE_SIZE == 0) {
        SetVelocity(0, 0);
        return;
    }


    float playerDistance = sqrt(pow(static_cast<float>(m_pPlayer->GetPosition().left - m_rcPosition.left), 2.0f) +
        pow(static_cast<float>(m_pPlayer->GetPosition().top - m_rcPosition.top), 2.0f));

    // G�r�� mesafesini biraz daha art�ral�m (�rne�in 25 tile)
    if (playerDistance > TILE_SIZE * 25) { // �nceki 20 tile idi
        m_state = AIState::IDLE;
        SetVelocity(0, 0);
        return;
    }

    bool hasLOS = HasLineOfSightToPlayer();

    if (hasLOS) {
        m_path.clear(); // G�r�� hatt� varsa �nceki yolu temizle
        m_pathIndex = 0;
        if (m_type == EnemyType::CHASER) {
            m_state = AIState::CHASING; // Chaser direkt kovalas�n
        }
        else { // TURRET tipi
            m_state = AIState::ATTACKING; // Turret direkt ate� etsin
        }
    }
    else {
        // G�r�� hatt� yoksa her zaman CHASER gibi davran�p yol bulmaya �al��s�n
        m_state = AIState::CHASING;
    }

    switch (m_state)
    {
    case AIState::IDLE:
        SetVelocity(0, 0);
        break;

    case AIState::CHASING:
        if (!m_path.empty() && m_pathIndex < m_path.size()) {
            FollowPath();
        }
        else { // Yol yoksa veya bittiyse
            // Direkt oyuncuya do�ru basit bir y�nelim (e�er pathfinding ba�ar�s�z olursa veya cooldown'daysa)
            // Bu k�s�m asl�nda pathfinding ile daha iyi y�netilir, ama bir fallback olarak kalabilir.
            float dirX = static_cast<float>(m_pPlayer->GetPosition().left - m_rcPosition.left);
            float dirY = static_cast<float>(m_pPlayer->GetPosition().top - m_rcPosition.top);
            float len = sqrt(dirX * dirX + dirY * dirY);
            if (len > 0) { dirX /= len; dirY /= len; }

            // HAREKET HIZI ARTIRIMI (CHASING - DIRECT)
            // �nceki h�z: * 5 idi, �imdi * 8 yapal�m (yakla��k %60 art��)
            SetVelocity(static_cast<int>(dirX * 8), static_cast<int>(dirY * 8));

            if (m_pathfindingCooldown <= 0) {
                FindPath(); // Yeni yol bulmay� dene
                m_pathfindingCooldown = 30; // Pathfinding cooldown'unu biraz azaltal�m (�nceki 45 idi)
                // Daha s�k yol bulmaya �al���r.
            }
        }
        break;

    case AIState::ATTACKING:
        SetVelocity(0, 0); // Sald�r�rken sabit dur
        if (m_attackCooldown <= 0) {
            AttackPlayer();
            // ATE� ETME HIZI ARTIRIMI (COOLDOWN AZALTMA)
            // �nceki cooldown: 60 idi, �imdi 35 yapal�m (yakla��k %40 daha h�zl� ate�)
            // TURRET tipi i�in
            m_attackCooldown = (m_type == EnemyType::TURRET) ? 35 : 50; // Chaser biraz daha yava� ate� edebilir.
        }
        break;
    }
}

bool Enemy::FindPath()
{
    if (!m_pPlayer || !m_pMaze || TILE_SIZE == 0) return false;

    // Hedef tile oyuncunun tam ortas� de�il, en yak�n grid h�cresi olmal�
    POINT startTile = { m_rcPosition.left / TILE_SIZE, m_rcPosition.top / TILE_SIZE };
    POINT endTile = { m_pPlayer->GetPosition().left / TILE_SIZE, m_pPlayer->GetPosition().top / TILE_SIZE };

    // Biti� noktas� bir duvar ise yol bulmaya �al��ma.
    // Ancak bazen oyuncu �ok h�zl� hareket ederken anl�k olarak duvar i�inde g�r�nebilir.
    // Bu durumda, oyuncunun hemen yan�ndaki ge�erli bir tile'� hedeflemek daha iyi olabilir.
    // �imdilik basit tutal�m:
    if (m_pMaze->IsWall(endTile.x, endTile.y)) {
        // Hedef duvar ise, etraf�ndaki bo� bir tile'� dene (basit bir deneme)
        int dx[] = { 0, 0, 1, -1, 1, 1, -1, -1 };
        int dy[] = { 1, -1, 0, 0, 1, -1, 1, -1 };
        bool foundAlternative = false;
        for (int i = 0; i < 8; ++i) {
            int altX = endTile.x + dx[i];
            int altY = endTile.y + dy[i];
            if (!m_pMaze->IsWall(altX, altY)) {
                endTile = { altX, altY };
                foundAlternative = true;
                break;
            }
        }
        if (!foundAlternative) return false; // Alternatif de bulunamazsa ��k
    }


    m_path = Pathfinder::FindPath(m_pMaze, startTile, endTile);
    m_pathIndex = 0;
    return !m_path.empty();
}

void Enemy::FollowPath()
{
    if (m_path.empty() || m_pathIndex >= m_path.size() || TILE_SIZE == 0) {
        SetVelocity(0, 0);
        m_path.clear(); // Yol bittiyse veya ge�ersizse temizle
        return;
    }

    POINT targetTile = m_path[m_pathIndex];
    // Hedef konumu tile'�n merkezi yap
    float targetX = static_cast<float>(targetTile.x * TILE_SIZE) + (TILE_SIZE / 2.0f);
    float targetY = static_cast<float>(targetTile.y * TILE_SIZE) + (TILE_SIZE / 2.0f);

    // Mevcut konum d��man�n merkezi
    float currentX = static_cast<float>(m_rcPosition.left + GetWidth() / 2.0f);
    float currentY = static_cast<float>(m_rcPosition.top + GetHeight() / 2.0f);

    float distanceToTargetTileCenter = sqrt(pow(targetX - currentX, 2.0f) + pow(targetY - currentY, 2.0f));

    // Hedef tile'a yeterince yakla��ld�ysa bir sonraki tile'a ge�
    if (distanceToTargetTileCenter < TILE_SIZE / 1.5f) { // Biraz daha toleransl� olabilir (�nceki TILE_SIZE / 2.0f idi)
        m_pathIndex++;
        if (m_pathIndex >= m_path.size()) {
            m_path.clear(); // Yolun sonuna gelindi
            SetVelocity(0, 0);
            return;
        }
        // Bir sonraki hedef tile'� hemen alal�m, b�ylece y�nelim daha erken g�ncellenir.
        targetTile = m_path[m_pathIndex];
        targetX = static_cast<float>(targetTile.x * TILE_SIZE) + (TILE_SIZE / 2.0f);
        targetY = static_cast<float>(targetTile.y * TILE_SIZE) + (TILE_SIZE / 2.0f);
    }

    float dirX = targetX - currentX;
    float dirY = targetY - currentY;
    float len = sqrt(dirX * dirX + dirY * dirY);
    if (len > 0) { dirX /= len; dirY /= len; }

    // HAREKET HIZI ARTIRIMI (FOLLOW_PATH)
    // �nceki h�z: * 7 idi, �imdi * 10 yapal�m (yakla��k %40 art��)
    SetVelocity(static_cast<int>(dirX * 10), static_cast<int>(dirY * 10));
}

bool Enemy::HasLineOfSightToPlayer()
{
    if (!m_pPlayer || !m_pMaze || TILE_SIZE == 0) return false;

    // Bresenham �izgi algoritmas� ile g�r�� hatt� kontrol�
    int x0 = m_rcPosition.left / TILE_SIZE;
    int y0 = m_rcPosition.top / TILE_SIZE;
    int x1 = m_pPlayer->GetPosition().left / TILE_SIZE;
    int y1 = m_pPlayer->GetPosition().top / TILE_SIZE;

    // �ok uzaksa direkt false d�n (performans i�in)
    // Bu mesafe, UpdateAI i�indeki genel detect mesafesinden k���k olmal�
    if (abs(x0 - x1) > 15 || abs(y0 - y1) > 15) return false; // �nceki 20 idi

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        // Ba�lang�� noktas�ndaki duvar kontrol�n� atla (d��man kendi i�indeyse tak�lmas�n)
        if (!(x0 == (m_rcPosition.left / TILE_SIZE) && y0 == (m_rcPosition.top / TILE_SIZE))) {
            if (m_pMaze->IsWall(x0, y0)) return false; // Arada duvar var
        }
        if (x0 == x1 && y0 == y1) break; // Hedefe ula��ld�
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    return true; // Engel yok, g�r�� hatt� var
}

void Enemy::AttackPlayer()
{
    // _pEnemyMissileBitmap ve game_engine global de�i�kenlerine Game.h �zerinden eri�iliyor.
    // Bu de�i�kenlerin null olup olmad���n� kontrol etmek iyi bir pratiktir.
    if (!m_pPlayer || !game_engine || !_pEnemyMissileBitmap || TILE_SIZE == 0) return;

    // Oyuncunun ve d��man�n merkez noktalar�n� hesapla
    float playerCenterX = static_cast<float>(m_pPlayer->GetPosition().left + m_pPlayer->GetWidth() / 2.0f);
    float playerCenterY = static_cast<float>(m_pPlayer->GetPosition().top + m_pPlayer->GetHeight() / 2.0f);
    float enemyCenterX = static_cast<float>(m_rcPosition.left + GetWidth() / 2.0f);
    float enemyCenterY = static_cast<float>(m_rcPosition.top + GetHeight() / 2.0f);

    // Ate� y�n�n� hesapla
    float dirX = playerCenterX - enemyCenterX;
    float dirY = playerCenterY - enemyCenterY;
    float length = sqrt(dirX * dirX + dirY * dirY);
    if (length > 0) { dirX /= length; dirY /= length; }
    else { return; } // Hedef tam �st�ndeyse ate� etme (b�lme s�f�r hatas� �nlemi)

    // Mermi i�in s�n�rlar (labirentin tamam�)
    // globalBounds Game.cpp'den geliyor, burada do�rudan eri�imi yok.
    // Merminin hareket edece�i genel alan� tan�mlayan bir RECT gerekli.
    // �imdilik MazeGenerator'dan labirent boyutlar�n� alal�m.
    // Bu, merminin Sprite kurucusuna verilecek rcBounds.
    RECT rcMissileBounds;
    if (m_pMaze) {
        const auto& mazeData = m_pMaze->GetMaze();
        if (!mazeData.empty() && !mazeData[0].empty()) {
            rcMissileBounds = { 0, 0,
                                static_cast<long>(mazeData[0].size() * TILE_SIZE),
                                static_cast<long>(mazeData.size() * TILE_SIZE) };
        }
        else { // Labirent verisi yoksa varsay�lan b�y�k bir alan
            rcMissileBounds = { 0, 0, 4000, 4000 };
        }
    }
    else { // m_pMaze null ise
        rcMissileBounds = { 0, 0, 4000, 4000 };
    }


    // Mermi olu�tur
    // Yeni Missile s�n�f�n� kullan�yorsak:
    // Missile* pMissile = new Missile(_pEnemyMissileBitmap, rcMissileBounds,
    //                                {(int)enemyCenterX, (int)enemyCenterY},
    //                                dirX * missileSpeed, dirY * missileSpeed);

    // E�er hala eski Sprite tabanl� mermi kullan�l�yorsa:
    Sprite* pMissile = new Sprite(_pEnemyMissileBitmap, rcMissileBounds, BA_DIE, SPRITE_TYPE_ENEMY_MISSILE);
    pMissile->SetPosition(static_cast<int>(enemyCenterX - pMissile->GetWidth() / 2.0f),
        static_cast<int>(enemyCenterY - pMissile->GetHeight() / 2.0f));

    // MERM� HIZI ARTIRIMI
    // �nceki h�z: 8 idi, �imdi 12 yapal�m (yakla��k %50 art��)
    int missileSpeed = 12;
    pMissile->SetVelocity(static_cast<int>(dirX * missileSpeed), static_cast<int>(dirY * missileSpeed));

    game_engine->AddSprite(pMissile);
}