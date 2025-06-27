#include "math_.h"
#include <cstdint>
#include <Offsets/client_dll.hpp>



ViewMatrix GetGameViewMatrix(uintptr_t Client)
{
    VMMDLL_SCATTER_HANDLE handle = mem.CreateScatterHandle();
    ViewMatrix vm{};

    mem.AddScatterReadRequest(handle, Client + cs2_dumper::offsets::client_dll::dwViewMatrix, &vm);
    mem.ExecuteReadScatter(handle);
    mem.CloseScatterHandle(handle);

    return vm;
}



uintptr_t GetBaseEntity(ULONG64 Client, int index)
{
    auto entlistBase = mem.Read<uintptr_t>(Client + cs2_dumper::offsets::client_dll::dwEntityList);

    if (entlistBase == 0) {
        return 0;
    }

    auto entitylistBase = mem.Read<uintptr_t>(entlistBase + 0x8 * (index >> 9) + 16);

    if (entitylistBase == 0) {
        return 0;
    }

  
    return mem.Read<uintptr_t>(entitylistBase + (0x78 * (index & 0x1ff)));

}

uintptr_t GetPawnFromController(uintptr_t Client, int index)
{
  
    auto entityList = mem.Read<uintptr_t>(Client + cs2_dumper::offsets::client_dll::dwEntityList);
    if (entityList == 0)
        return 0;


    auto controllerPage = mem.Read<uintptr_t>(entityList + 8 * (index >> 9) + 0x10);
    if (controllerPage == 0)
        return 0;

    auto playerController = mem.Read<uintptr_t>(controllerPage + 0x78 * (index & 0x1FF));
    if (playerController == 0)
        return 0;

   
    uint32_t pawnHandle =  mem.Read<uint32_t>(playerController + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn);
    if ((pawnHandle & 0x7FFF) == 0x7FFF)
        return 0;

    int pawnIndex = pawnHandle & 0x7FFF;

   
    auto pawnPage = mem.Read<uintptr_t>(entityList + 8 * (pawnIndex >> 9) + 0x10);
    if (pawnPage == 0)
        return 0;

    auto pawn = mem.Read<uintptr_t>(pawnPage + 0x78 * (pawnIndex & 0x1FF));
    return pawn;
}

bool WorldToScreen(const Vector3& worldPosition, Vector3& screenPosition, const ViewMatrix& viewMatrix, int screenWidth, int screenHeight)
{
    float w = viewMatrix.matrix[3][0] * worldPosition.x +
        viewMatrix.matrix[3][1] * worldPosition.y +
        viewMatrix.matrix[3][2] * worldPosition.z +
        viewMatrix.matrix[3][3];

    if (w < 0.1f)
        return false;

    float x = viewMatrix.matrix[0][0] * worldPosition.x +
        viewMatrix.matrix[0][1] * worldPosition.y +
        viewMatrix.matrix[0][2] * worldPosition.z +
        viewMatrix.matrix[0][3];

    float y = viewMatrix.matrix[1][0] * worldPosition.x +
        viewMatrix.matrix[1][1] * worldPosition.y +
        viewMatrix.matrix[1][2] * worldPosition.z +
        viewMatrix.matrix[1][3];

    screenPosition.x = (screenWidth / 2.0f) * (1.0f + x / w);
    screenPosition.y = (screenHeight / 2.0f) * (1.0f - y / w);
    screenPosition.z = w;

    return true;
}

PlayerPosition GetPlayerPosition(uintptr_t Client, uintptr_t playerController)
{
    PlayerPosition pos = { {0,0,0}, {0,0,0} };

    uint32_t pawnHandle = mem.Read<uint32_t>(playerController + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn);
    if (!pawnHandle) return pos;

    auto pawnAddress = GetBaseEntity(Client, pawnHandle & 0x7FFF);
    if (!pawnAddress) return pos;

    VMMDLL_SCATTER_HANDLE handle = mem.CreateScatterHandle();

    Vector3 origin{}, viewOffset{};
    mem.AddScatterReadRequest(handle, pawnAddress + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin, &origin);
    mem.AddScatterReadRequest(handle, pawnAddress + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset, &viewOffset);
    mem.ExecuteReadScatter(handle);
    mem.CloseScatterHandle(handle);

    pos.foot = origin;
    pos.head = {
        origin.x + viewOffset.x,
        origin.y + viewOffset.y,
        origin.z + viewOffset.z
    };

    return pos;
}

std::string GetName(uintptr_t playerController)
{
    std::string name;
    uintptr_t temp = mem.Read<uintptr_t>(playerController + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);
    if (temp) {
        char buff[50]{};
        mem.Read(temp, &buff, sizeof(buff));  // sizeof(buff) == 50
        name = std::string(buff);
    }
    else {
        name = "unknown";
    }
    return name;
}

bool ReadAllPlayerNames(const ControllerInfo* controllers, int count, PlayerNameInfo* outNames)
{
    //批量读名字字符串地址
    auto handle1 = mem.CreateScatterHandle();
    for (int i = 0; i < count; ++i) {
        if (!controllers[i].controllerPtr) {
            outNames[i].strAddr = 0;
            continue;
        }
        uintptr_t addr = controllers[i].controllerPtr + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName;
        mem.AddScatterReadRequest(handle1, addr, &outNames[i].strAddr);
    }
    mem.ExecuteReadScatter(handle1);
    mem.CloseScatterHandle(handle1);

   //批量读名字内容
    auto handle2 = mem.CreateScatterHandle();
    for (int i = 0; i < count; ++i) {
        if (!outNames[i].strAddr) {
            memset(outNames[i].name, 0, sizeof(outNames[i].name));
            continue;
        }
        mem.AddScatterReadRequest(handle2, outNames[i].strAddr, outNames[i].name, sizeof(outNames[i].name));
    }
    mem.ExecuteReadScatter(handle2);
    mem.CloseScatterHandle(handle2);

    return true;
}


bool ReadControllersAndPawns(uintptr_t clientBase, ControllerInfo* outControllers, int maxCount)
{
    uintptr_t entityList = mem.Read<uintptr_t>(clientBase + cs2_dumper::offsets::client_dll::dwEntityList);
    if (!entityList) {
        LOG("[-] Failed to read entity list\n");
        return false;
    }

    // 缓存 page 指针
    uintptr_t controllerPages[4]{};
    uintptr_t pawnPages[4]{};

    for (int i = 0; i < maxCount; ++i) {
        int ctrlPageIdx = i >> 9;
        if (!controllerPages[ctrlPageIdx]) {
            controllerPages[ctrlPageIdx] = mem.Read<uintptr_t>(entityList + 0x10 + 8 * ctrlPageIdx);
        }
    }

    // 批量读 controller entry 地址
    auto handle1 = mem.CreateScatterHandle();
    for (int i = 0; i < maxCount; ++i) {
        int ctrlPageIdx = i >> 9;
        int ctrlSlot = i & 0x1FF;
        uintptr_t page = controllerPages[ctrlPageIdx];
        if (!page) {
            outControllers[i].controllerPtr = 0;
            continue;
        }
        uintptr_t addr = page + 0x78 * ctrlSlot;
        mem.AddScatterReadRequest(handle1, addr, &outControllers[i].controllerPtr);
    }
    mem.ExecuteReadScatter(handle1);
    mem.CloseScatterHandle(handle1);

    //批量读 m_hPlayerPawn
    auto handle2 = mem.CreateScatterHandle();
    for (int i = 0; i < maxCount; ++i) {
        if (!outControllers[i].controllerPtr) {
            outControllers[i].pawnHandle = 0xFFFFFFFF;
            continue;
        }
        uintptr_t addr = outControllers[i].controllerPtr + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn;
        mem.AddScatterReadRequest(handle2, addr, &outControllers[i].pawnHandle);
    }
    mem.ExecuteReadScatter(handle2);
    mem.CloseScatterHandle(handle2);

    //解 pawn 地址
    auto handle3 = mem.CreateScatterHandle();
    for (int i = 0; i < maxCount; ++i) {
        uint32_t handleVal = outControllers[i].pawnHandle;
        int pawnIndex = handleVal & 0x7FFF;
        if (pawnIndex >= 2048) {
            outControllers[i].pawnPtr = 0;
            continue;
        }
        int pawnPageIdx = pawnIndex >> 9;
        if (!pawnPages[pawnPageIdx]) {
            pawnPages[pawnPageIdx] = mem.Read<uintptr_t>(entityList + 0x10 + 8 * pawnPageIdx);
        }
        uintptr_t page = pawnPages[pawnPageIdx];
        if (!page) {
            outControllers[i].pawnPtr = 0;
            continue;
        }
        uintptr_t addr = page + 0x78 * (pawnIndex & 0x1FF);
        mem.AddScatterReadRequest(handle3, addr, &outControllers[i].pawnPtr);
    }
    mem.ExecuteReadScatter(handle3);
    mem.CloseScatterHandle(handle3);

    return true;
}