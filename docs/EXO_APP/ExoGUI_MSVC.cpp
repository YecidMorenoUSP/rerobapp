

#include <cstdio>
#include <iostream>
#include <armadillo>

#define SDL_MAIN_HANDLED

#include <imgui.h>
#include <imgui.cpp>
#include <imgui_internal.h>

#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>

#include <imgui_impl_sdl.h>
#include <imgui_impl_sdl.cpp>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_opengl2.cpp>

#include <utils.h>

#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h> 

#include <imgui_plot2.hpp>
#include <imgui_plot2.cpp>

#include <string.h>
#include <mutex>
#include <atomic>

#include <windows.h>

#include "mainGUI.cpp"


int main(int argc, const char* argv[]) {

    SDL_SetMainReady();

    if(mainGUI()<0) return -1; 

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );
	
    SDL_Window* window = SDL_CreateWindow("  ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1, 1, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

	
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }


    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    ImVec4 clear_color = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);


	static bool  sdemo ;
    // Main loop
    bool done = false;
    while (!done)
    {
    
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        static bool exit = false;
        loopGUI(&exit);
        if (exit) break;
		{
			
			static ImGuiWindowFlags maximized  ;
			static bool showPlot = true;
			static bool close = true;
			static float time = 0;
			
			static bool RUNNING = false;
			
			// ImGui::ShowDemoWindow();         

			ImGui::Begin("LOG",&showPlot,maximized);
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::Text(GUI.LOG.c_str());
			ImGui::End();

			// ImGui::Begin("LOG",&showPlot,maximized);

			// ImGui::End();

		
			time += ImGui::GetIO().DeltaTime;

			if(!close) break;

		}

     
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();


	

	return 0;
}
