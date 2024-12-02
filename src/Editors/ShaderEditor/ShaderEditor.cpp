// ShaderEditor.cpp : Определяет точку входа для приложения.
//
#include "stdafx.h"
#include "../../xrEngine/xr_input.h"
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    if (!IsDebuggerPresent()) Debug._initialize(false);
    const char* FSName = "fs.ltx";
    Core._initialize("Shader", ELogCallback, 1, FSName);

    Tools = new CShaderTool();
    STools = (CShaderTool*)Tools;
    UI = new CShaderMain();
    UI->RegisterCommands();

    UIMainForm* MainForm = new UIMainForm();
    ::MainForm = MainForm;
    UI->Push(MainForm, false);

    bool NeedExit = false;
    while (!NeedExit)
    {
        SDL_Event Event;
        while (SDL_PollEvent(&Event))
        {
            switch (Event.type)
            {
            case SDL_QUIT:
                EPrefs->SaveConfig();
                NeedExit = true;
                break;

            case SDL_WINDOWEVENT:
                switch (Event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    if (UI && REDevice)
                    {
                        UI->Resize(Event.window.data1, Event.window.data2, true);
                        EPrefs->SaveConfig();
                    }
                    break;
                case SDL_WINDOWEVENT_SHOWN:
                case SDL_WINDOWEVENT_ENTER:
                    Device.b_is_Active = true;
                    break;

                case SDL_WINDOWEVENT_HIDDEN:
                case SDL_WINDOWEVENT_LEAVE:
                    Device.b_is_Active = false;
                    break;
                }
                break;

            case SDL_KEYDOWN:
                if (UI) UI->KeyDown(Event.key.keysym.scancode, UI->GetShiftState());
                break;
            case SDL_KEYUP:
                if (UI) UI->KeyUp(Event.key.keysym.scancode, UI->GetShiftState());
                break;

            case SDL_MOUSEMOTION:
                pInput->MouseMotion(Event.motion.xrel, Event.motion.yrel);
                break;
            case SDL_MOUSEWHEEL:
                pInput->MouseScroll(Event.wheel.y);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                int mouse_button = 0;
                if (Event.button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
                if (Event.button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
                if (Event.button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
                if (Event.type == SDL_MOUSEBUTTONDOWN) {
                    pInput->MousePressed(mouse_button);
                }
                else {
                    pInput->MouseReleased(mouse_button);
                }
            }
            break;
            }

            if (!UI->ProcessEvent(&Event))
                break;
        }
        MainForm->Frame();
    }

    xr_delete(MainForm);
    Core._destroy();
    return 0;
}
