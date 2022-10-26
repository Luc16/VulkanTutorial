//
// Created by luc on 26/10/22.
//

#ifndef VULKANTUTORIAL_WINDOW_H
#define VULKANTUTORIAL_WINDOW_H

#include <GLFW/glfw3.h>

namespace vtt {
    class Window {
    public:
        Window(int width, int height, const std::string& title);
        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;
        Window(Window &&) = delete;
        Window &operator=(Window &&) = delete;

        ~Window();

        [[nodiscard]] GLFWwindow* window() const { return m_window; }
    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

        bool m_frameBufferResized = false;
        int m_width;
        int m_height;

        GLFWwindow* m_window{};
    };
}



#endif //VULKANTUTORIAL_WINDOW_H
