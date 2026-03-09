#include "imgui.h"
#include <vector>
#include <cstdint>
#include <mutex>
#include <algorithm>

#include "mem.h"
#include "shared.h"

struct line_point {
    double x, y;
    float r, g, b, a;

    bool operator<(const line_point& other) const {
        if (x != other.x) return x < other.x;
        if (y != other.y) return y < other.y;
        return true;
    }
    
    bool operator==(const line_point& other) const {
        return x == other.x && y == other.y;
    }
};

struct node_path {
    uintptr_t prev;
    uintptr_t next;
    double x;
    double y;
    uint64_t extra1;
    uint64_t extra2;
};

static std::vector<line_point> grid_points;
static std::mutex g_LinesMutex;
static int g_GridSize = 48;
static float g_CellSize = 300.0f / g_GridSize;

const float map_min = -100.0f;
const float map_max = 1000.0f;
const float map_width = map_max - map_min;
const float map_height = map_max - map_min;

const float window_size = 300.0f;

extern "C" void add_grid_point(double x, double y, float r, float g, float b, float a = 1.0f) {
    std::lock_guard<std::mutex> lock(g_LinesMutex);
    grid_points.push_back({x, y, r, g, b, a});
}

extern "C" void clear_grid_points() {
    std::lock_guard<std::mutex> lock(g_LinesMutex);
    grid_points.clear();
}

void draw_grid_window() {
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(window_size + 40, window_size + 60), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("Pathing monitor", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(window_size, window_size);
    int safety_counter = 0;

    // draw background
    draw_list->AddRectFilled(canvas_pos, 
                             ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                             IM_COL32(20, 20, 30, 255));

    if (pathing_head_addr != 0x0) {
        
        uintptr_t head_ptr = *(uintptr_t*)pathing_head_addr;
        node_path node = *(node_path*)pathing_head_addr;

        uintptr_t node_ptr = node.next;

        while (node_ptr != head_ptr && node_ptr != 0x0 && node.next != 0x0 && safety_counter < 100) {

            if (!is_mapped(node_ptr)) {
                clear_grid_points();
                break;
            }

            node = *(node_path*)node_ptr;
            node_ptr = (uintptr_t)node.next;

            line_point p{node.x, node.y};

            if(!count(grid_points.begin(), grid_points.end(), p)) {
                add_grid_point(node.x, node.y, 0.490f, 0.215f, 0.770f, 1.0f);
            }
            safety_counter+=1;
        }
    }

    const float scale_factor_x = window_size / map_width;
    const float scale_factor_y = window_size / map_height;
    const float scale_factor = std::min(scale_factor_x, scale_factor_y);  // Ensure the grid fits within the window

    g_CellSize = window_size / g_GridSize;

    //draw cells
    for (int i = 0; i <= g_GridSize; i++) {
        float pos = canvas_pos.x + i * g_CellSize;
        draw_list->AddLine(ImVec2(pos, canvas_pos.y), 
                          ImVec2(pos, canvas_pos.y + canvas_size.y),
                          IM_COL32(50, 50, 60, 150), 1.0f);  // Vertical
        
        pos = canvas_pos.y + i * g_CellSize;
        draw_list->AddLine(ImVec2(canvas_pos.x, pos), 
                          ImVec2(canvas_pos.x + canvas_size.x, pos),
                          IM_COL32(50, 50, 60, 150), 1.0f);  // Horizontal
    }

    //draw path
    {
        std::lock_guard<std::mutex> lock(g_LinesMutex);
        for (size_t i = 1; i < grid_points.size(); ++i)
        {
            const line_point& p1_src = grid_points[i - 1];
            const line_point& p2_src = grid_points[i];

            float norm_x1 = (p1_src.x - map_min) / map_width * g_GridSize;
            float norm_y1 = (p1_src.y - map_min) / map_height * g_GridSize;
            float norm_x2 = (p2_src.x - map_min) / map_width * g_GridSize;
            float norm_y2 = (p2_src.y - map_min) / map_height * g_GridSize;

            ImVec2 p1(canvas_pos.x + norm_x1 * g_CellSize, 
                      canvas_pos.y + norm_y1 * g_CellSize);
            ImVec2 p2(canvas_pos.x + norm_x2 * g_CellSize, 
                      canvas_pos.y + norm_y2 * g_CellSize);

            ImVec4 color(p2_src.r, p2_src.g, p2_src.b, p2_src.a);
            draw_list->AddLine(p1, p2, ImColor(color), 2.0f);
        }
    }

    // draw border
    draw_list->AddRect(canvas_pos, 
                      ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                      IM_COL32(100, 100, 120, 255), 0.3f, 0, 2.0f);

    ImGui::End();
    clear_grid_points();
}