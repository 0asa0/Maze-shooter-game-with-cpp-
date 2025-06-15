// Game.h
#ifndef GAME_H
#define GAME_H
#pragma once
#include <windows.h>
#include "Resource.h"
#include "GameEngine.h"
#include "Bitmap.h"
#include "Sprite.h"
#include "Background.h"
#include  "MazeGenerator.h"
#include "Camera.h"
#include "Player.h"
#include "Enemy.h"
#include "FOVBackground.h"
#include <vector>

//Structures to be used
struct Tile {
    int x, y;
    Bitmap* bitmap;
};
// YEN�: Bir skoru ve zaman damgas�n� tutmak i�in struct yap�s�
struct HighScoreEntry {
    int score;
    std::string timestamp;

    // Skor'a g�re (azalan) s�ralama i�in kar��la�t�rma operat�r�
    bool operator<(const HighScoreEntry& other) const {
        return score < other.score;
    }
};

//--------------------------------------------------
//Global Variables (Declarations)
//--------------------------------------------------

extern HINSTANCE   instance;
extern GameEngine* game_engine;
extern Player* charSprite;
extern MazeGenerator* mazeGenerator;
extern Bitmap* _pEnemyMissileBitmap;
extern int TILE_SIZE;
extern FOVBackground* fovEffect;
extern Camera* camera;
extern std::vector<Tile> nonCollidableTiles;
extern Bitmap* healthPWBitmap;
extern Bitmap* ammoPWBitmap;
extern Bitmap* armorPWBitmap;
extern Bitmap* pointPWBitmap;
extern Bitmap* wallBitmap;
extern Bitmap* floorBitmap;
extern Bitmap* keyBitmap;
extern Bitmap* endPointBitmap;
extern Bitmap* secondWeaponBitmap;
extern Bitmap* _pPlayerMissileBitmap;

// D��man spawn zamanlay�c�lar� i�in de�i�kenler
extern DWORD g_dwLastSpawnTime;
extern DWORD g_dwLastClosestEnemySpawnTime; // YEN�: En yak�n d��mandan spawn i�in zamanlay�c�
extern bool isLevelFinished;

extern int  currentLevel;

extern int window_X, window_Y;

void GenerateLevel(int level);
void GenerateMaze(Bitmap* tileBit);
void AddNonCollidableTile(int x, int y, Bitmap* bitmap);
void CleanupLevel();
void LoadBitmaps(HDC hDC);
void OnLevelComplete();

// D��man spawn fonksiyonlar�
void SpawnEnemyNearPlayer();
void SpawnEnemyNearClosest(); // YEN� FONKS�YON B�LD�R�M�


void DrawUI(HDC hDC);
void LoadHighScores();
void SaveHighScores();
void CheckAndSaveScore(int finalScore);
void RestartGame();
std::string GetCurrentTimestamp(); // YEN�

#endif //GAME_H