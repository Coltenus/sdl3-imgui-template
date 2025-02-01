#include "scene.h"

namespace ui {

Scene::Scene(bool fixed, ImVec2 size, ImVec4 clear_color) : clear_color(clear_color), size(size), fixed_size(fixed) {
    frame_buffer = new utils::FrameBuffer();
}

Scene::~Scene() {
    delete frame_buffer;
}

void Scene::bind() {
    frame_buffer->bind();
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Scene::unbind() {
    frame_buffer->unbind();
}

void Scene::draw() {
    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene", NULL, fixed_size ? ImGuiWindowFlags_NoResize : 0);
    const auto sw_size = ImGui::GetContentRegionAvail();
    frame_buffer->rescale(glm::vec2(sw_size.x, sw_size.y));
    glViewport(0, 0, (int)sw_size.x, (int)sw_size.y);
    ImGui::Image((ImTextureID)frame_buffer->get_texture(), sw_size, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

};
