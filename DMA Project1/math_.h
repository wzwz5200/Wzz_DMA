#pragma once
#include <libs/vmmdll.h>
#include <Memory/Memory.h>
#include "Offsets/offsets.hpp"

struct EntityHandle {
    uintptr_t entity; // 真实 pawn 地址
    // 可能还有其它成员，比如序号，具体看CS2反汇编结构
};

struct EntityPointer {
    uintptr_t ptr;
};

constexpr int kMaxEntities = 32;
constexpr int kEntitiesPerPage = 8;
constexpr int kPageCount = kMaxEntities / kEntitiesPerPage;



struct Vector3 {
    float x, y, z;

    Vector3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    // 向量加向量
    Vector3 operator+(const Vector3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
};

struct PlayerPosition {
    Vector3 head;    // 头部位置
    Vector3 foot;    // 脚部位置
};

struct ViewMatrix {
    float matrix[4][4];
};

struct BoneJointData
{
    Vector3 Pos;
    char pad[0x14];
};

struct BoneJointPos
{
    Vector3 Pos;
    Vector3 ScreenPos;
    bool IsVisible = false;
};

bool ReadAllEntities(uintptr_t clientBase, EntityPointer* outEntities);

ViewMatrix GetGameViewMatrix(uintptr_t Client);
uintptr_t GetBaseEntity(uintptr_t Client, int index);
uintptr_t GetPawnFromController(uintptr_t Client, int index);

bool WorldToScreen(const Vector3& worldPosition, Vector3& screenPosition, const ViewMatrix& viewMatrix, int screenWidth, int screenHeight);


PlayerPosition GetPlayerPosition(uintptr_t Client, uintptr_t playerController);

struct ControllerInfo {
    uintptr_t controllerPtr;
    uint32_t pawnHandle;
    uintptr_t pawnPtr;
};


bool ReadControllersAndPawns(uintptr_t clientBase, ControllerInfo* outControllers, int maxCount);