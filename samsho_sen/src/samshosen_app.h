// samshosen - ReXGlue Recompiled Project
//
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <filesystem>
#include <fstream>
#include <rex/rex_app.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(_WIN32)
extern "C" void InitDiagnostics();
#endif

class SamshoSenApp : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<SamshoSenApp>(new SamshoSenApp(ctx, "samshosen",
        PPCImageConfig));
  }

  void OnConfigureLogging(rex::LogConfig& config) override {
    config.log_file = "samshosen_run.log";
    config.default_level = spdlog::level::debug;
  }

  void OnConfigurePaths(rex::PathConfig& paths) override {
    if (paths.game_data_root.empty()) {
      std::filesystem::path p1 = "C:/Users/azure/Downloads/samsho-recomp/extracted";
      std::filesystem::path p2 = std::filesystem::current_path() / "extracted";
      std::filesystem::path p3 = std::filesystem::current_path().parent_path().parent_path().parent_path().parent_path() / "extracted";
      if (std::filesystem::exists(p1)) {
        paths.game_data_root = p1;
      } else if (std::filesystem::exists(p2)) {
        paths.game_data_root = p2;
      } else if (std::filesystem::exists(p3)) {
        paths.game_data_root = p3;
      }
    }
  }

  void OnPreSetup(rex::RuntimeConfig& config) override {
    config.gpu_plugin = "xenos";
  }

  void OnPostSetup() override {
#if defined(_WIN32)
    InitDiagnostics();

    uint8_t* vbase = runtime()->memory()->virtual_membase();
    if (vbase) {
      VirtualAlloc(vbase, 0x40000000, MEM_COMMIT, PAGE_READWRITE);
    }
#endif
  }

  void OnPreLaunchModule() override {
#if defined(_WIN32)
    if (window()) {
      HWND hwnd = static_cast<HWND>(window()->GetNativeWindowHandle());
      if (hwnd) {
        ShowWindow(hwnd, SW_SHOWNORMAL);
        SetForegroundWindow(hwnd);
        SetActiveWindow(hwnd);
      }
    }
#endif
  }
};
