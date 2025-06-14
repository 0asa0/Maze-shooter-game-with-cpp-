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

#endif //GAME_H