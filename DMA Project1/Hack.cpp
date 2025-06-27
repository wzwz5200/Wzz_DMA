#include "pch.h"
#include "Hack.h"
#include "Memory/Memory.h"
#include "Offsets/offsets.hpp"
#include <iostream>
#include <chrono>
#include <iostream>

#include <Offsets/client_dll.hpp>
#include <ImGui/imgui.h>
#include <mutex>
#include <array> 
#include <future>

ImVec4 espColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

CheatInit::CheatInit(uintptr_t Client)
{

	viewMatrix = GetGameViewMatrix(Client);

    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    static uintptr_t cachedLocalPlayer = 0;
    static uintptr_t cachedGameEntitySystem = 0;
    static int cachedMaxIndex = 0;
    static DWORD lastUpdate = 0;
  
    DWORD now = GetTickCount();
    if (now - lastUpdate > 500) {
        cachedLocalPlayer = mem.Read<uintptr_t>(Client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
     


        lastUpdate = now;
    }
    LocalPlayer = cachedLocalPlayer;



	

}

void CheatInit::Cache(uintptr_t Client)
{

    localTeam = mem.Read<int>(LocalPlayer + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum); // m_iTeamNum

    auto drawList = ImGui::GetBackgroundDrawList();

    ControllerInfo entities[kMaxEntities]{};

    if (ReadControllersAndPawns(Client, entities, kMaxEntities)) {
        for (int i = 0; i < kMaxEntities; ++i) {
            if (entities[i].controllerPtr == 0) continue;

            int entityTeam = mem.Read<int>(entities[i].controllerPtr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
            if (entityTeam == localTeam) continue;

            pos = GetPlayerPosition(Client, entities[i].controllerPtr);
            int Life = mem.Read<int>(entities[i].pawnPtr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_lifeState);
            if (Life != 256) continue;

            uintptr_t GameSceneNode = mem.Read<uintptr_t>(entities[i].pawnPtr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
            if (!GameSceneNode) continue;

            uintptr_t boneMatrix =  mem.Read<uintptr_t>(GameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80);
            if (!boneMatrix) continue;

            mem.Read(boneMatrix, &BoneArray, sizeof(BoneArray));

            std::vector<BoneJointPos> BonePosList;
            for (int i = 0; i < 30; i++) {
                Vector3 screenPos;
                bool isVisible = false;

                if (WorldToScreen(BoneArray[i].Pos, screenPos, viewMatrix, screenWidth, screenHeight))
                    isVisible = true;

                BonePosList.push_back({ BoneArray[i].Pos, screenPos, isVisible });
            }

            for (int i = 1; i < BonePosList.size(); ++i) {
                const auto& prev = BonePosList[i - 1];
                const auto& curr = BonePosList[i];
                if (prev.IsVisible && curr.IsVisible) {
                    drawList->AddLine(ImVec2(prev.ScreenPos.x, prev.ScreenPos.y),
                        ImVec2(curr.ScreenPos.x, curr.ScreenPos.y),
                        IM_COL32(0, 255, 0, 255), 1.5f);
                }
            }






            Vector3 headScreen, footScreen, g;
            WorldToScreen(pos.head, headScreen, viewMatrix, screenWidth, screenHeight);
            WorldToScreen(pos.foot, footScreen, viewMatrix, screenWidth, screenHeight);
            float height = footScreen.y - headScreen.y;
            float width = height * 0.5f;
            drawList->AddRect(ImVec2(headScreen.x - width / 2, headScreen.y),
                ImVec2(headScreen.x + width / 2, footScreen.y),
                ImGui::ColorConvertFloat4ToU32(espColor), 0.0f, 0, 1.5f);
       

            std::cout << pos.head.x << std::endl;





        }
    }
    else {
        LOG("[-] Failed to read controllers and pawns\n");
    }
   




}
