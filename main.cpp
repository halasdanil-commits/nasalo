// Меню на C++ (Dear ImGui 1.90-1.91 + Win32 + DirectX 11)
// Отдельное окно-приложение, ни во что не внедряется.
// Сохрани файл в UTF-8 и собирай с флагом /utf-8 (Visual Studio: C/C++ -> Command Line -> /utf-8).
#include <d3d11.h>
#include <tchar.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

// ---------- DirectX11 ----------
static ID3D11Device* g_dev; static ID3D11DeviceContext* g_ctx; static IDXGISwapChain* g_sc; static ID3D11RenderTargetView* g_rtv;
static void MakeRTV() { ID3D11Texture2D* b; g_sc->GetBuffer(0, IID_PPV_ARGS(&b)); g_dev->CreateRenderTargetView(b, nullptr, &g_rtv); b->Release(); }
static bool MakeDevice(HWND h) {
    DXGI_SWAP_CHAIN_DESC d = {}; d.BufferCount = 2; d.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; d.OutputWindow = h; d.SampleDesc.Count = 1; d.Windowed = TRUE;
    d.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; D3D_FEATURE_LEVEL fl;
    const D3D_FEATURE_LEVEL arr[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, arr, 2, D3D11_SDK_VERSION, &d, &g_sc, &g_dev, &fl, &g_ctx) != S_OK) return false;
    MakeRTV(); return true;
}
static LRESULT WINAPI WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (ImGui_ImplWin32_WndProcHandler(h, m, w, l)) return true;
    if (m == WM_SIZE && g_dev && w != SIZE_MINIMIZED) {
        if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
        g_sc->ResizeBuffers(0, LOWORD(l), HIWORD(l), DXGI_FORMAT_UNKNOWN, 0); MakeRTV(); return 0;
    }
    if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(h, m, w, l);
}

// ---------- данные меню ----------
struct Item { std::string n; bool slider; bool on; float v, mn, mx; float* ext; };
struct Col  { std::string t; std::vector<Item> it; };
struct Tab  { std::string t; std::vector<Col> c; };
struct Page { std::string t; std::vector<Tab> tabs; };
static float g_sizePct = 100.f;                       // размер меню, %
static Item T(const char* n, bool on = false) { return { n, false, on, 0, 0, 0, nullptr }; }
static Item S(const char* n, float mn, float mx, float v, float* ext = nullptr) { return { n, true, false, v, mn, mx, ext }; }
static std::vector<Page> D;
static void BuildData() {
    D = {
     {"Основное", {{"Общее", {{"Главное", {T("Автозапуск"), S("Плавность",0,20,5), T("Тихий режим",1), T("Автосохранение",1), T("Уведомления",1), T("Анимации"), S("Масштаб интерфейса",50,150,100), S("Задержка отклика",0,500,120)}},
                               {"Быстрые действия", {T("Горячие клавиши"), T("Подсказки",1), S("Яркость",0,100,80)}}}},
                  {"Дополнительно", {{"Расширенное", {T("Журнал событий"), T("Энергосбережение"), S("Лимит кадров",30,240,120)}}}}}},
     {"Визуал", {{"Оформление", {{"Тема", {T("Тёмная тема",1), S("Акцентный цвет",0,360,320), S("Прозрачность окна",0,100,90), S("Скругление углов",0,32,20), S("Размер меню",50,150,100,&g_sizePct)}},
                                 {"Шрифт", {T("Крупный текст"), T("Жирный текст")}}}},
                 {"Эффекты", {{"Анимации", {T("Размытие фона",1), T("Свечение",1), T("Частицы"), S("Скорость анимации",0,200,100)}}}}}},
     {"Файлы", {{"Профили", {{"Конфиги", {T("Автозагрузка профиля",1), T("Резервные копии",1), S("Хранить версий",1,20,5), T("Сжатие")}}}},
                {"Импорт", {{"Импорт и экспорт", {T("Проверять файлы",1), T("Перезаписывать"), S("Размер кэша, МБ",0,1024,256)}}}}}},
     {"Система", {{"Производительность", {{"Ресурсы", {T("Аппаратное ускорение",1), T("Верт. синхронизация"), S("Потоки обработки",1,16,4), S("Лимит памяти, МБ",256,4096,1024)}}}},
                  {"Сеть", {{"Подключение", {T("Автопереподключение",1), T("Показывать пинг",1), S("Таймаут, сек",1,60,15)}}}}}},
     {"Звук", {{"Громкость", {{"Уровни", {S("Общая громкость",0,100,70), S("Музыка",0,100,50), T("Звуки интерфейса",1), T("Звук переключателей",1)}}}},
               {"Эффекты", {{"Обработка", {T("Эхо"), S("Басы",0,10,3), T("Шумоподавление",1)}}}}}},
     {"Информация", {{"Журнал", {{"Логи", {T("Сохранять логи"), T("Подробный лог"), S("Хранить дней",1,30,7)}}}},
                     {"О меню", {{"Обновления", {T("Проверять обновления",1), T("Показывать версию",1), T("Отправлять отчёты")}}}}}}
    };
}

// ---------- состояние ----------
static bool  g_open = true, g_keyOpen = false, g_active = false;
static int   g_page = 0, g_tab = 0;
static float g_shake = 0.f;
static char  g_keyBuf[64] = "", g_keyMsg[48] = "";
static const char* KEY = "837261179228";
static float S_ = 1.f;                                 // текущий масштаб
static ImVec2 O;                                       // левый верхний угол окна меню

// ---------- вспомогательное ----------
static ImU32 C(int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); }
static ImVec2 P(float x, float y) { return ImVec2(O.x + x * S_, O.y + y * S_); }
static ImU32 Mix(ImU32 a, ImU32 b, float t) {
    ImVec4 x = ImGui::ColorConvertU32ToFloat4(a), y = ImGui::ColorConvertU32ToFloat4(b);
    return ImGui::ColorConvertFloat4ToU32(ImVec4(x.x + (y.x - x.x) * t, x.y + (y.y - x.y) * t, x.z + (y.z - x.z) * t, x.w + (y.w - x.w) * t));
}
// плавная анимация к цели (хранится по ID)
static float Anim(ImGuiID id, float target, float speed = 14.f) {
    float* v = ImGui::GetStateStorage()->GetFloatRef(id, target);
    *v += (target - *v) * std::min(1.f, ImGui::GetIO().DeltaTime * speed);
    return *v;
}
static const ImU32 ACC = IM_COL32(185, 133, 255, 255), PINK = IM_COL32(255, 95, 168, 255), MUTE = IM_COL32(123, 122, 135, 255), TXT = IM_COL32(233, 232, 239, 255);

// логотип (синяя волна): путь из SVG, толстая линия из кругов
static void Logo(ImDrawList* dl, ImVec2 pos, float width) {
    const float k = width / 560.f; auto pt = [&](float x, float y) { return ImVec2(pos.x + (x - 240) * k, pos.y + (y - 340) * k); };
    std::vector<ImVec2> pts; auto line = [&](ImVec2 a, ImVec2 b) { int n = (int)(std::hypot(b.x - a.x, b.y - a.y) / 5) + 1; for (int i = 0; i <= n; i++) pts.push_back({ a.x + (b.x - a.x) * i / n, a.y + (b.y - a.y) * i / n }); };
    auto quad = [&](ImVec2 a, ImVec2 c, ImVec2 b) { for (int i = 0; i <= 14; i++) { float t = i / 14.f, u = 1 - t; pts.push_back({ u * u * a.x + 2 * u * t * c.x + t * t * b.x, u * u * a.y + 2 * u * t * c.y + t * t * b.y }); } };
    line(pt(312, 664), pt(410, 440)); quad(pt(410, 440), pt(435, 395), pt(462, 440)); line(pt(462, 440), pt(575, 680));
    quad(pt(575, 680), pt(600, 725), pt(625, 680)); line(pt(625, 680), pt(722, 450));
    for (auto& p : pts) { float t = std::clamp((p.y - pos.y) / (440 * k), 0.f, 1.f); dl->AddCircleFilled(p, 52 * k, Mix(C(91, 82, 238), C(20, 185, 236), t), 24); }
}
static void Icon(ImDrawList* dl, int k, ImVec2 c, ImU32 col, float s) {
    float th = 1.7f * s; const float PI = 3.14159f;
    switch (k) {
    case 0: dl->AddCircle(c, 13 * s, col, 32, th); dl->AddCircle(c, 8 * s, col, 24, th); dl->AddCircleFilled(c, 2.5f * s, col); break;
    case 1: dl->PathArcTo({ c.x, c.y + 8 * s }, 14 * s, PI * 1.15f, PI * 1.85f, 20); dl->PathStroke(col, 0, th);
            dl->PathArcTo({ c.x, c.y + 8 * s }, 7 * s, PI * 1.2f, PI * 1.8f, 16); dl->PathStroke(col, 0, th); dl->AddCircleFilled({ c.x, c.y + 8 * s }, 1.6f * s, col); break;
    case 2: dl->AddRect({ c.x - 13 * s, c.y - 9 * s }, { c.x + 13 * s, c.y + 11 * s }, col, 4 * s, 0, th); dl->AddLine({ c.x - 13 * s, c.y - 3 * s }, { c.x + 13 * s, c.y - 3 * s }, col, th); break;
    case 3: dl->AddCircle(c, 5 * s, col, 20, th);
            for (int i = 0; i < 8; i++) { float a = i * PI / 4; dl->AddLine({ c.x + cosf(a) * 9 * s, c.y + sinf(a) * 9 * s }, { c.x + cosf(a) * 13 * s, c.y + sinf(a) * 13 * s }, col, th * 1.6f); }
            dl->AddCircle(c, 9 * s, col, 24, th); break;
    case 4: for (int i = 0; i < 5; i++) { float h = (i % 2 ? 7 : 12) * s; float x = c.x + (i - 2) * 6 * s; dl->AddLine({ x, c.y - h }, { x, c.y + h }, col, th * 1.5f); } break;
    case 5: dl->AddCircle(c, 13 * s, col, 32, th); dl->AddCircleFilled({ c.x, c.y - 5 * s }, 1.6f * s, col); dl->AddLine({ c.x, c.y - 1 * s }, { c.x, c.y + 7 * s }, col, th * 1.5f); break;
    }
}
// кнопка с текстом
static bool Btn(const char* id, const char* txt, ImVec2 p, ImVec2 sz, bool accent) {
    ImDrawList* dl = ImGui::GetWindowDrawList(); ImGui::SetCursorScreenPos(p); bool c = ImGui::InvisibleButton(id, sz);
    float h = Anim(ImGui::GetID(id), ImGui::IsItemHovered() ? 1.f : 0.f, 16);
    dl->AddRectFilled(p, { p.x + sz.x, p.y + sz.y }, accent ? Mix(ACC, C(210, 170, 255), h) : Mix(C(27, 27, 33), C(40, 40, 50), h), 14 * S_);
    ImVec2 ts = ImGui::CalcTextSize(txt);
    dl->AddText({ p.x + (sz.x - ts.x) / 2, p.y + (sz.y - ts.y) / 2 }, accent ? C(26, 18, 38) : TXT, txt); return c;
}
// анимированный тумблер
static void Toggle(const char* id, bool* v, ImVec2 p) {
    ImDrawList* dl = ImGui::GetWindowDrawList(); float w = 45 * S_, h = 28 * S_;
    ImGui::SetCursorScreenPos(p); if (ImGui::InvisibleButton(id, { w, h })) *v = !*v;
    float t = Anim(ImGui::GetID(id), *v ? 1.f : 0.f, 14);
    dl->AddRectFilled(p, { p.x + w, p.y + h }, Mix(C(42, 42, 52), ACC, t), h / 2);
    ImVec2 c = { p.x + (14 + 17 * t) * S_, p.y + h / 2 };
    dl->AddCircleFilled(c, 10 * S_, Mix(C(14, 14, 18), C(255, 255, 255), t), 24);
    if (t > 0.05f) { ImU32 k = C(26, 18, 38, (int)(255 * t)); dl->AddLine({ c.x - 4 * S_, c.y }, { c.x - 1 * S_, c.y + 3 * S_ }, k, 2.2f * S_); dl->AddLine({ c.x - 1 * S_, c.y + 3 * S_ }, { c.x + 4.5f * S_, c.y - 3 * S_ }, k, 2.2f * S_); }
}
// слайдер
static void Slider(const char* id, float* v, float mn, float mx, ImVec2 p) {
    ImDrawList* dl = ImGui::GetWindowDrawList(); float w = 165 * S_;
    ImGui::SetCursorScreenPos({ p.x, p.y - 6 * S_ }); ImGui::InvisibleButton(id, { w, 22 * S_ });
    if (ImGui::IsItemActive()) *v = mn + std::clamp((ImGui::GetIO().MousePos.x - p.x) / w, 0.f, 1.f) * (mx - mn);
    float t = (*v - mn) / (mx - mn);
    dl->AddRectFilled(p, { p.x + w, p.y + 10 * S_ }, C(38, 38, 46), 5 * S_);
    dl->AddRectFilled(p, { p.x + w * t, p.y + 10 * S_ }, ACC, 5 * S_);
    float tx = p.x + w * t; dl->AddRectFilled({ tx - 9 * S_, p.y - 6 * S_ }, { tx + 9 * S_, p.y + 16 * S_ }, TXT, 5 * S_);
}

// ---------- ключ ----------
static void SaveKey() { std::ofstream("key.dat") << "1"; }
static void LoadKey() { std::ifstream f("key.dat"); char c = 0; if (f >> c) g_active = (c == '1'); }
static void KeyPanel() {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(O, P(1116, 697), C(15, 15, 20, 250), 30 * S_);
    float sx = std::sin(g_shake * 40.f) * 8 * S_ * g_shake; if (g_shake > 0) g_shake = std::max(0.f, g_shake - ImGui::GetIO().DeltaTime * 3);
    float cx = 558 - 190, y = 200; auto text = [&](const char* s, float sz, float yy, ImU32 col) { ImVec2 ts = ImGui::GetFont()->CalcTextSizeA(sz * S_, FLT_MAX, 0, s); dl->AddText(ImGui::GetFont(), sz * S_, { P(558, 0).x - ts.x / 2 + sx, P(0, yy).y }, col, s); };
    if (!g_active) {
        text("Активация", 30, y, TXT); text("Введите ключ, чтобы открыть меню", 16, y + 46, MUTE);
        ImGui::SetCursorScreenPos({ P(cx, y + 90).x + sx, P(0, y + 90).y }); ImGui::PushItemWidth(380 * S_);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, C(27, 27, 33)); ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14 * S_); ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 14 * S_, 14 * S_ });
        bool enter = ImGui::InputTextWithHint("##key", "Ключ", g_keyBuf, sizeof(g_keyBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopStyleVar(2); ImGui::PopStyleColor(); ImGui::PopItemWidth();
        bool go = Btn("##act", "Активировать", { P(cx, y + 150).x + sx, P(0, y + 150).y }, { 380 * S_, 50 * S_ }, true) || enter;
        if (go) {
            std::string k; for (char c : std::string(g_keyBuf)) if (c != ' ' && c != '-') k += c;
            if (k == KEY) { g_active = true; g_keyOpen = false; g_keyMsg[0] = 0; g_keyBuf[0] = 0; SaveKey(); }
            else { strcpy_s(g_keyMsg, k.empty() ? "Введите ключ" : "Неверный ключ"); g_shake = 1.f; }
        }
        text(g_keyMsg, 16, y + 215, PINK); text("Бот: не подключён", 14, y + 250, MUTE);
    } else {
        text("Ключ активен", 30, y, TXT); text("Тип: навсегда", 16, y + 46, MUTE);
        if (Btn("##back", "Назад", P(cx, y + 100), { 380 * S_, 50 * S_ }, true)) g_keyOpen = false;
        if (Btn("##reset", "Сбросить ключ", P(cx, y + 162), { 380 * S_, 50 * S_ }, false)) { g_active = false; g_keyOpen = false; std::remove("key.dat"); }
        text("Бот: не подключён", 14, y + 235, MUTE);
    }
}

// ---------- меню ----------
static void DrawMenu() {
    ImGuiIO& io = ImGui::GetIO(); S_ = g_sizePct / 100.f; io.FontGlobalScale = S_;
    ImGui::SetNextWindowSize({ 1116 * S_, 697 * S_ }, ImGuiCond_Always); ImGui::SetNextWindowPos({ 300, 120 }, ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 30 * S_); ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 }); ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, C(22, 22, 27));
    ImGui::Begin("menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
    O = ImGui::GetWindowPos(); ImDrawList* dl = ImGui::GetWindowDrawList();

    if (!g_active || g_keyOpen) { KeyPanel(); }
    else {
        // боковая панель
        dl->AddRectFilled(P(14, 16), P(118, 682), C(27, 27, 33), 22 * S_);
        Logo(dl, P(22, 28), 60 * S_);
        dl->AddLine(P(48, 116), P(84, 116), C(255, 255, 255, 16));
        const float navTop[6] = { 155, 239, 323, 405, 489, 576 };
        float iy = Anim(ImGui::GetID("navy"), navTop[g_page], 12);
        dl->AddRectFilled(P(36, iy), P(96, iy + 60), C(52, 25, 43), 16 * S_);
        for (int i = 0; i < 6; i++) {
            ImGui::SetCursorScreenPos(P(36, navTop[i]));
            char id[16]; snprintf(id, sizeof id, "##nav%d", i);
            if (ImGui::InvisibleButton(id, { 60 * S_, 60 * S_ })) { g_page = i; g_tab = 0; }
            float a = Anim(ImGui::GetID(id), (g_page == i || ImGui::IsItemHovered()) ? 1.f : 0.f, 12);
            Icon(dl, i, P(66, navTop[i] + 30), Mix(C(138, 137, 150), PINK, a), S_ * (1.f + 0.1f * (g_page == i ? 1.f : 0.f)));
        }
        // верхняя панель
        dl->AddRectFilled(P(135, 17), P(183, 65), C(27, 27, 33), 14 * S_);
        for (int i = 0; i < 3; i++) dl->AddLine(P(147, 31 + i * 7), P(171 - i * 6, 31 + i * 7), MUTE, 1.8f * S_);
        dl->AddRectFilled(P(198, 17), P(576, 63), C(27, 27, 33), 14 * S_);
        dl->AddCircle(P(220, 39), 7 * S_, MUTE, 20, 1.8f * S_); dl->AddLine(P(225, 44), P(230, 49), MUTE, 1.8f * S_);
        ImGui::SetCursorScreenPos(P(246, 26)); ImGui::PushItemWidth(310 * S_);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, C(0, 0, 0, 0)); static char sb[64] = ""; ImGui::InputTextWithHint("##s", "Поиск", sb, sizeof sb); ImGui::PopStyleColor(); ImGui::PopItemWidth();
        ImGui::SetCursorScreenPos(P(940, 17)); if (ImGui::InvisibleButton("##gear", { 48 * S_, 48 * S_ })) g_keyOpen = true;
        Icon(dl, 3, P(964, 41), ImGui::IsItemHovered() ? TXT : MUTE, S_ * 0.7f);
        if (Btn("##close", "Закрыть", P(1006, 17), { 94 * S_, 54 * S_ }, false)) g_open = false;

        // вкладки
        auto& tabs = D[g_page].tabs; float tx = 135;
        for (int i = 0; i < (int)tabs.size(); i++) {
            ImVec2 ts = ImGui::CalcTextSize(tabs[i].t.c_str()); float w = ts.x / S_ + 58; char id[16]; snprintf(id, sizeof id, "##tab%d", i);
            ImGui::SetCursorScreenPos(P(tx, 83)); if (ImGui::InvisibleButton(id, { w * S_, 40 * S_ })) g_tab = i;
            float a = Anim(ImGui::GetID(id), g_tab == i ? 1.f : 0.f, 14);
            dl->AddRectFilled(P(tx, 83), P(tx + w, 123), C(27, 27, 33, (int)(255 * a)), 12 * S_);
            dl->AddText({ P(tx + 29, 0).x, P(0, 103).y - ts.y / 2 }, Mix(MUTE, TXT, a), tabs[i].t.c_str());
            dl->AddCircleFilled(P(tx + w / 2, 126), 3 * S_, C(185, 133, 255, (int)(255 * a)));
            tx += w + 12;
        }

        // колонки
        auto& cols = tabs[g_tab].c; float cx = 135;
        for (auto& col : cols) {
            dl->AddText(P(cx, 150), C(207, 207, 216), col.t.c_str());
            float top = 175, rh = 61; dl->AddRectFilled(P(cx, top), P(cx + 456, top + rh * col.it.size()), C(27, 27, 33), 16 * S_);
            for (int r = 0; r < (int)col.it.size(); r++) {
                Item& it = col.it[r]; float y = top + r * rh; char id[32]; snprintf(id, sizeof id, "##i%p", (void*)&it);
                if (r) dl->AddLine(P(cx + 15, y), P(cx + 441, y), C(255, 255, 255, 10));
                float lit = it.slider ? 0.f : Anim(ImGui::GetID(id) + 1, it.on ? 1.f : 0.f, 14);
                ImVec2 ts = ImGui::CalcTextSize(it.n.c_str());
                dl->AddText({ P(cx + 15, 0).x, P(0, y + rh / 2).y - ts.y / 2 }, it.slider ? C(160, 159, 172) : Mix(MUTE, TXT, lit), it.n.c_str());
                if (it.slider) {
                    float& val = it.ext ? *it.ext : it.v; char buf[24]; snprintf(buf, sizeof buf, "%.2f", val);
                    dl->AddRectFilled(P(cx + 189, y + 15), P(cx + 256, y + 45), C(21, 21, 26), 10 * S_);
                    ImVec2 vs = ImGui::CalcTextSize(buf); dl->AddText({ P(cx + 222.5f, 0).x - vs.x / 2, P(0, y + 30).y - vs.y / 2 }, C(214, 213, 222), buf);
                    Slider(id, &val, it.mn, it.mx, P(cx + 276, y + 25));
                } else Toggle(id, &it.on, P(cx + 396, y + 16.5f));
            }
            cx += 473;
        }
        dl->AddRectFilled(P(1068, 135), P(1098, 499), C(185, 133, 255), 15 * S_);

        // уголок изменения размера
        ImGui::SetCursorScreenPos(P(1066, 647)); ImGui::InvisibleButton("##rs", { 44 * S_, 44 * S_ });
        if (ImGui::IsItemActive()) g_sizePct = std::clamp(g_sizePct + io.MouseDelta.x / (1116 * S_) * 100.f, 50.f, 150.f);
        dl->AddLine(P(1090, 686), P(1104, 672), MUTE, 1.5f * S_); dl->AddLine(P(1098, 686), P(1104, 680), MUTE, 1.5f * S_);
    }
    ImGui::End(); ImGui::PopStyleColor(); ImGui::PopStyleVar(3);
}

// круглая кнопка-логотип: перетаскивается, тап открывает/закрывает меню
static void Fab() {
    ImGui::SetNextWindowPos({ 40, 140 }, ImGuiCond_FirstUseEver); ImGui::SetNextWindowSize({ 84, 84 });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 }); ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("fab", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
    ImVec2 p = ImGui::GetWindowPos(); ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGui::InvisibleButton("##f", { 84, 84 });
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0, 5.f)) ImGui::SetWindowPos({ p.x + ImGui::GetIO().MouseDelta.x, p.y + ImGui::GetIO().MouseDelta.y });
    if (ImGui::IsItemDeactivated() && ImGui::GetMouseDragDelta(0).x * ImGui::GetMouseDragDelta(0).x + ImGui::GetMouseDragDelta(0).y * ImGui::GetMouseDragDelta(0).y < 25.f) g_open = !g_open;
    dl->AddCircleFilled({ p.x + 42, p.y + 42 }, 40, C(21, 21, 26), 40); dl->AddCircle({ p.x + 42, p.y + 42 }, 34, C(37, 50, 77), 40, 2.f);
    Logo(dl, { p.x + 20, p.y + 28 }, 44);
    ImGui::End(); ImGui::PopStyleVar(2);
}

// плашка сверху (fps, время)
static void Hud() {
    ImDrawList* dl = ImGui::GetBackgroundDrawList(); float w = ImGui::GetIO().DisplaySize.x; ImVec2 a = { w / 2 - 172, 24 }, b = { w / 2 + 172, 68 };
    dl->AddRectFilled(a, b, C(20, 20, 26, 220), 14); char buf[32]; time_t t = time(0); tm lt; localtime_s(&lt, &t);
    strftime(buf, sizeof buf, "%H:%M", &lt); dl->AddText({ a.x + 22, a.y + 13 },
