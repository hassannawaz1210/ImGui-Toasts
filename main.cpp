#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <cmath>
#include <map>

#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Toast structure
struct Toast {
    std::string type;
    std::string title;
    std::string text;
    std::chrono::steady_clock::time_point creation_time;
    float timeout; // in seconds
    float animation_progress;
    float height; // Store the height of the toast

    Toast(const std::string& type, const std::string& title, const std::string& text, float timeout)
        : type(type), title(title), text(text), timeout(timeout), animation_progress(0.0f), height(0.0f) {
        creation_time = std::chrono::steady_clock::now();
    }
};

std::vector<Toast> toasts;

// Notification class
class Notification {
public:
    void Info(const std::string& title, const std::string& text) {
        AddToast("info", title, text);
    }

    void Warning(const std::string& title, const std::string& text) {
        AddToast("warning", title, text);
    }

    void Error(const std::string& title, const std::string& text) {
        AddToast("error", title, text);
    }

    void Success(const std::string& title, const std::string& text) {
        AddToast("success", title, text);
    }

private:
    void AddToast(const std::string& type, const std::string& title, const std::string& text, float timeout = 5.0f) {
        toasts.emplace_back(type, title, text, timeout);
    }
};


template <typename T>
const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

float EaseOut(float t) {
    return 1.0f - std::pow(1.0f - t, 3.0f);
}

void RenderToasts(const std::map<std::string, GLuint>& iconTextures, ImFont* boldFont, ImFont* regularFont) {
    auto now = std::chrono::steady_clock::now();
    int display_w, display_h;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &display_w, &display_h);
    float previousToastHeight = 0.0f; // Initialize previous toast height

    for (auto it = toasts.begin(); it != toasts.end();) {
        float elapsed_time = std::chrono::duration<float>(now - it->creation_time).count();
        if (elapsed_time > it->timeout) {
            it = toasts.erase(it);
        }
        else {
            if (it->animation_progress < 1.0f) {
                it->animation_progress += 0.005f; //animation speed
                if (it->animation_progress > 1.0f) {
                    it->animation_progress = 1.0f;
                }
            }
            float eased_progress = EaseOut(it->animation_progress);

            // Calculate position and size based on animation progress
            float x_offset = (1.0f - eased_progress) * 300.0f;
            ImVec2 window_pos = ImVec2(display_w - 420.0f + x_offset, 30.0f + previousToastHeight + 10.0f * std::distance(toasts.begin(), it));

            ImGui::PushFont(regularFont);
            ImVec2 textSize = ImGui::CalcTextSize(it->text.c_str(), nullptr, false, 380.0f); // 380.0f is the width of the text area
            ImGui::PopFont();

            float windowHeight = textSize.y + 50.0f;
            it->height = windowHeight; // Store the height of the current toast

            ImGui::SetNextWindowPos(window_pos);
            ImGui::SetNextWindowSize(ImVec2(400, windowHeight)); // Fixed width, dynamic height

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // Adjust padding as needed

            ImGui::Begin(it->type.c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);

            // Save the current cursor position
            ImVec2 cursorPos = ImGui::GetCursorPos();

            // Calculate vertical center position for the icon
            float iconSize = 20.0f; //icon size
            float iconY = (windowHeight - iconSize) / 2.0f;

            ImGui::Dummy(ImVec2(5.0f, 0.0f)); //spacer
            ImGui::SameLine();

            // Set cursor position for the icon
            ImGui::SetCursorPosY(iconY);

            //show icon based on type 
            if (it->type == "success") {
                ImGui::Image((void*)(intptr_t)iconTextures.at("success"), ImVec2(iconSize, iconSize));
            }
            else if (it->type == "error") {
                ImGui::Image((void*)(intptr_t)iconTextures.at("error"), ImVec2(iconSize, iconSize));
            }
            else if (it->type == "warning") {
                ImGui::Image((void*)(intptr_t)iconTextures.at("warning"), ImVec2(iconSize, iconSize));
            }
            else if (it->type == "info") {
                ImGui::Image((void*)(intptr_t)iconTextures.at("info"), ImVec2(iconSize, iconSize));
            }

            ImGui::SameLine();

            ImGui::Dummy(ImVec2(10.0f, 0.0f)); //spacer
            ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::PushFont(boldFont);
            ImGui::Text("%s", it->title.c_str());
            ImGui::PopFont();
            ImGui::PushFont(regularFont);
            ImGui::PushTextWrapPos(380.0f); // Set wrap position to the width of the text area
            ImGui::TextWrapped("%s", it->text.c_str());
            ImGui::PopTextWrapPos();
            ImGui::PopFont();
            ImGui::EndGroup();

            // Calculate progress for the progress bar
            float progress = 1.0f - (elapsed_time / it->timeout);

            // Draw the progress bar as a filled rectangle at the bottom
            ImVec2 p = ImGui::GetWindowPos();
            ImVec2 progressBarStart = ImVec2(p.x, p.y + windowHeight - 5.0f); // Adjust the height as needed
            ImVec2 progressBarEnd = ImVec2(p.x + (ImGui::GetWindowWidth() * progress), p.y + windowHeight);
            ImGui::GetWindowDrawList()->AddRectFilled(progressBarStart, progressBarEnd, IM_COL32(255, 255, 255, 255)); // White color

            ImGui::End();

            // Reset padding
            ImGui::PopStyleVar();

            // Update previous toast height
            previousToastHeight += windowHeight + 10.0f; // Add some spacing between toasts

            ++it;
        }
    }
}

GLuint LoadTextureFromFile(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) {
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    return texture;
}

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Notification System", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize OpenGL loader
    //if (gl3wInit()) {
    //    return -1;
    //}

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load icons
    std::map<std::string, GLuint> iconTextures;
    iconTextures["success"] = LoadTextureFromFile("icons/tick.png");
    if(iconTextures["success"] == 0) {
		std::cerr << "Failed to load icon" << std::endl;
		return -1;
	}
    iconTextures["error"] = LoadTextureFromFile("icons/error.png");
    iconTextures["warning"] = LoadTextureFromFile("icons/warning.png");
    iconTextures["info"] = LoadTextureFromFile("icons/info.png");

    // Fonts loading
    ImFont* boldFont = io.Fonts->AddFontFromFileTTF("fonts/bold.otf", 18.0f);
    ImFont* regularFont = io.Fonts->AddFontFromFileTTF("fonts/regular.otf", 18.0f);

    Notification g_Notification;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Notification Demo");

        if (ImGui::Button("Add Info Toast")) {
            g_Notification.Info("Info Title", "This is an info message");
        }
        if (ImGui::Button("Add Warning Toast")) {
            g_Notification.Warning("Warning Title", "This is a warning message");
        }
        if (ImGui::Button("Add Error Toast")) {
            g_Notification.Error("Error Title", "This is an error message");
        }
        if (ImGui::Button("Add Success Toast")) {
            g_Notification.Success("Success Title", "This is a success message");
        }

        ImGui::End();

        // Render toasts
        RenderToasts(iconTextures, boldFont, regularFont);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
