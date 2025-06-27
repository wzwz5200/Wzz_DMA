#include <cstdint>
#include <math_.h>
#pragma once



class CheatInit
{
public:

	CheatInit(uintptr_t Client);

	void Cache(uintptr_t Client);



private:
    uintptr_t   LocalPlayer;

     int screenWidth;
    int screenHeight;

	ViewMatrix viewMatrix;

	uintptr_t entity;

	uintptr_t pawn;
	
	int team;
	int localTeam;
	uintptr_t scene;

	uintptr_t boneMatrix;
	BoneJointData BoneArray[30]{};


	Vector3 screenPos;

	PlayerPosition pos;
	Vector3 headScreen, footScreen;


};



