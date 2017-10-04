#include <stdio.h>
#include <system_error>
#include "kernel.hpp"
#include <Interaction/OpenGL.hpp>
#include <thread>

int main() {
    getEnvironment().init();
    try {
        StaticMesh model;
        model.load("Res/human.obj");
        FrameBufferCPU FB;
        GLWindow window;
        Pipeline pipeline;
        glm::mat4 V = lookAt({ 6.0f,2.0f,0.0f }, vec3{ 0.0f,2.0f,0.0f }, { 0.0f,1.0f,0.0f });
        glm::mat4 M;
        M = scale(M, vec3(1.0f, 1.0f, 1.0f)*0.02f);
        float t = glfwGetTime();
        while (window.update()) {
            auto size = window.size();
            if (size.x == 0 || size.y == 0)
                std::this_thread::yield();
            FB.resize(size.x, size.y);
            float w = size.x, h = size.y;
            glm::mat4 P = perspectiveFov(radians(45.0f), w, h, 0.1f, 20.0f);
            float now = glfwGetTime();
            M = rotate(M, (now - t)*2.0f, { 0.0f,1.0f,0.0f });
            t = now;
            Uniform uni{ P*V*M,model.mTex->toSampler() };
            auto uniform = share(std::vector<Uniform>({ uni }));
            kernel(model.mVert, model.mIndex, uniform, FB, pipeline);
            window.present(pipeline, *FB.colorBuffer);
        }
    }
    catch (const std::exception& e) {
        puts("Catched an error:");
        puts(e.what());
        system("pause");
    }
    return 0;
}
