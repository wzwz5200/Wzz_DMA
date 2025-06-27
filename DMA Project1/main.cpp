#include "pch.h"
#include "framework.h"
#include <iostream>
#include <Memory/Memory.h>
#include <Hack.h>
#include <SimpleOV.hpp>




static bool showWindow = true; // 默认显示窗口
bool ESP = false;

int main() {

    std::cout << "HELLO\n";

    mem.Init("cs2.exe", true, false);
    uintptr_t Client =	mem.GetBaseDaddy("client.dll");




    overlay::SetupWindow();
    if (!(overlay::CreateDeviceD3D(overlay::Window)))
        return 1;

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsLight;

    while (!overlay::ShouldQuit)
    {
        if (GetAsyncKeyState(VK_INSERT) & 1)  // &1 确保只触发一次
        {
            showWindow = !showWindow;  // 切换显示状态
        }
        overlay::Render();

        ImGuiIO& io = ImGui::GetIO();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 10.0f;    // 窗口圆角


        // 暗色主题配色
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f); // 更深背景
        colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);

        colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.4f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.5f, 1.0f);

        colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.2f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.4f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.4f, 0.5f, 1.0f);

        colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.25f, 0.35f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.35f, 0.45f, 1.00f);

        colors[ImGuiCol_CheckMark] = ImVec4(0.49f, 0.49f, 0.49f, 0.78f); // 高亮蓝色勾
        colors[ImGuiCol_SliderGrab] = ImVec4(0.49f, 0.49f, 0.49f, 0.78f);
        //  colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.5f, 0.9f, 1.0f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 0.78f);

        colors[ImGuiCol_Separator] = ImVec4(0.2f, 0.2f, 0.3f, 1.0f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3f, 0.3f, 0.4f, 1.0f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.4f, 0.4f, 0.5f, 1.0f);

        //ImGui::ShowDemoWindow();

        float fps = 1.0f / io.DeltaTime;

        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Once);
        if (ESP) {
            CheatInit Init(Client);
            Init.Cache(Client);
          


        }
        if (showWindow) {

            ImGui::Begin("WZ辅助", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::Text("FPS: %.2f", fps);
            ImGui::Checkbox("透视", &ESP);


            ImGui::End();



        }




        overlay::EndRender();
    }

    overlay::CloseOverlay();


    getchar();

}
