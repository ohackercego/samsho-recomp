#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <deque>
#include <mutex>

#include <rex/hook.h>
#include <rex/types.h>
#include <rex/input/input.h>

using namespace rex;

namespace rex::kernel::xam {
u32 XamInputGetState_entry(u32 user_index, u32 flags, ppc_ptr_t<rex::input::X_INPUT_STATE> state_ptr);
u32 XamInputGetCapabilities_entry(u32 user_index, u32 flags, ppc_ptr_t<rex::input::X_INPUT_CAPABILITIES> caps);
u32 XamInputGetCapabilitiesEx_entry(u32 unk, u32 user_index, u32 flags, ppc_ptr_t<rex::input::X_INPUT_CAPABILITIES> caps);
}

namespace {

static uint32_t s_packet_number_p1 = 1;
static uint32_t s_packet_number_p2 = 1;

u32 Custom_XamInputGetState_entry(u32 user_index, u32 flags, ppc_ptr_t<rex::input::X_INPUT_STATE> state_ptr) {
  if (!state_ptr) return 0x00000057;

  if (user_index > 1) {
    return 1167; // ERROR_DEVICE_NOT_CONNECTED
  }

  u32 res = rex::kernel::xam::XamInputGetState_entry(user_index, flags, state_ptr);

  if (res != 0) {
    std::memset(state_ptr, 0, sizeof(rex::input::X_INPUT_STATE));
    res = 0;
  }

  if (user_index == 0) {
    state_ptr->packet_number = ++s_packet_number_p1;
  } else {
    state_ptr->packet_number = ++s_packet_number_p2;
  }

  HWND fg = GetForegroundWindow();
  DWORD fg_pid = 0;
  if (fg) GetWindowThreadProcessId(fg, &fg_pid);
  if (!fg || fg_pid == GetCurrentProcessId()) {
    auto is_down = [](int vk) -> bool {
      return (GetAsyncKeyState(vk) & 0x8000) != 0;
    };

    uint16_t b = state_ptr->gamepad.buttons;
    int16_t lx = state_ptr->gamepad.thumb_lx;
    int16_t ly = state_ptr->gamepad.thumb_ly;

    if (user_index == 0) {
      // Player 1
      // Movimento: Setas (D-Pad + Analógico Esquerdo)
      if (is_down(VK_UP)) {
        b |= rex::input::X_INPUT_GAMEPAD_DPAD_UP;
        ly = 32767;
      }
      if (is_down(VK_DOWN)) {
        b |= rex::input::X_INPUT_GAMEPAD_DPAD_DOWN;
        ly = -32768;
      }
      if (is_down(VK_LEFT)) {
        b |= rex::input::X_INPUT_GAMEPAD_DPAD_LEFT;
        lx = -32768;
      }
      if (is_down(VK_RIGHT)) {
        b |= rex::input::X_INPUT_GAMEPAD_DPAD_RIGHT;
        lx = 32767;
      }

      // Golpes / Ações: ASDF
      // A -> Xbox A
      // S -> Xbox B
      // D -> Xbox X
      // F -> Xbox Y
      if (is_down('A')) b |= rex::input::X_INPUT_GAMEPAD_A;
      if (is_down('S')) b |= rex::input::X_INPUT_GAMEPAD_B;
      if (is_down('D')) b |= rex::input::X_INPUT_GAMEPAD_X;
      if (is_down('F')) b |= rex::input::X_INPUT_GAMEPAD_Y;

      // Ombros e Gatilhos: QWERT
      if (is_down('Q')) b |= rex::input::X_INPUT_GAMEPAD_LEFT_SHOULDER;
      if (is_down('W')) b |= rex::input::X_INPUT_GAMEPAD_RIGHT_SHOULDER;
      if (is_down('E')) state_ptr->gamepad.left_trigger = 255;
      if (is_down('R')) state_ptr->gamepad.right_trigger = 255;
      if (is_down('T')) b |= rex::input::X_INPUT_GAMEPAD_LEFT_THUMB;

      // Start: Enter ou Espaco
      if (is_down(VK_RETURN) || is_down(VK_SPACE)) b |= rex::input::X_INPUT_GAMEPAD_START;

      // Select / Back: Ctrl ou Esc
      if (is_down(VK_CONTROL) || is_down(VK_ESCAPE)) b |= rex::input::X_INPUT_GAMEPAD_BACK;
    } else if (user_index == 1) {
      // Player 2 - Teclas numéricas (Numpad e números superiores 0-9)
      // Movimento: 8 (Cima), 2 ou 5 (Baixo), 4 (Esquerda), 6 (Direita)
      if (is_down(VK_NUMPAD8) || is_down('8')) {
        b |= rex::input::X_INPUT_GAMEPAD_DPAD_UP;
        ly = 32767;
      }
      if (is_down(VK_NUMPAD2) || is_down(VK_NUMPAD5) || is_down('2') || is_down('5')) {
        b |= rex::input::X_INPUT_GAMEPAD_DPAD_DOWN;
        ly = -32768;
      }
      if (is_down(VK_NUMPAD4) || is_down('4')) {
        b |= rex::input::X_INPUT_GAMEPAD_DPAD_LEFT;
        lx = -32768;
      }
      if (is_down(VK_NUMPAD6) || is_down('6')) {
        b |= rex::input::X_INPUT_GAMEPAD_DPAD_RIGHT;
        lx = 32767;
      }

      // Golpes / Ações: 1, 3, 7, 9
      // 1 -> Xbox A
      // 3 -> Xbox B
      // 7 -> Xbox X
      // 9 -> Xbox Y
      if (is_down(VK_NUMPAD1) || is_down('1')) b |= rex::input::X_INPUT_GAMEPAD_A;
      if (is_down(VK_NUMPAD3) || is_down('3')) b |= rex::input::X_INPUT_GAMEPAD_B;
      if (is_down(VK_NUMPAD7) || is_down('7')) b |= rex::input::X_INPUT_GAMEPAD_X;
      if (is_down(VK_NUMPAD9) || is_down('9')) b |= rex::input::X_INPUT_GAMEPAD_Y;

      // Ombros e Gatilhos: 0, ., /, *
      // 0 -> LB (Left Shoulder)
      // . -> RB (Right Shoulder)
      // / -> LT (Left Trigger)
      // * -> RT (Right Trigger)
      if (is_down(VK_NUMPAD0) || is_down('0')) b |= rex::input::X_INPUT_GAMEPAD_LEFT_SHOULDER;
      if (is_down(VK_DECIMAL) || is_down(VK_OEM_PERIOD)) b |= rex::input::X_INPUT_GAMEPAD_RIGHT_SHOULDER;
      if (is_down(VK_DIVIDE) || is_down(VK_OEM_2)) state_ptr->gamepad.left_trigger = 255;
      if (is_down(VK_MULTIPLY)) state_ptr->gamepad.right_trigger = 255;

      // Start: + (Numpad Add ou tecla +)
      if (is_down(VK_ADD) || is_down(VK_OEM_PLUS)) b |= rex::input::X_INPUT_GAMEPAD_START;

      // Select / Back: - (Numpad Subtract ou tecla -)
      if (is_down(VK_SUBTRACT) || is_down(VK_OEM_MINUS)) b |= rex::input::X_INPUT_GAMEPAD_BACK;
    }

    state_ptr->gamepad.buttons = b;
    state_ptr->gamepad.thumb_lx = lx;
    state_ptr->gamepad.thumb_ly = ly;
  }

  return res;
}

u32 Custom_XamInputGetCapabilities_entry(u32 user_index, u32 flags, ppc_ptr_t<rex::input::X_INPUT_CAPABILITIES> caps) {
  u32 res = rex::kernel::xam::XamInputGetCapabilities_entry(user_index, flags, caps);
  if ((user_index == 0 || user_index == 1) && caps) {
    std::memset(caps, 0, sizeof(rex::input::X_INPUT_CAPABILITIES));
    caps->type = 1;
    caps->sub_type = 1;
    caps->flags = 0;
    caps->gamepad.buttons = 0xFFFF;
    caps->gamepad.left_trigger = 0xFF;
    caps->gamepad.right_trigger = 0xFF;
    caps->gamepad.thumb_lx = (int16_t)0x7FFF;
    caps->gamepad.thumb_ly = (int16_t)0x7FFF;
    caps->gamepad.thumb_rx = (int16_t)0x7FFF;
    caps->gamepad.thumb_ry = (int16_t)0x7FFF;
    return 0;
  }
  return res;
}

u32 Custom_XamInputGetCapabilitiesEx_entry(u32 unk, u32 user_index, u32 flags, ppc_ptr_t<rex::input::X_INPUT_CAPABILITIES> caps) {
  u32 res = rex::kernel::xam::XamInputGetCapabilitiesEx_entry(unk, user_index, flags, caps);
  if ((user_index == 0 || user_index == 1) && caps) {
    std::memset(caps, 0, sizeof(rex::input::X_INPUT_CAPABILITIES));
    caps->type = 1;
    caps->sub_type = 1;
    caps->flags = 0;
    caps->gamepad.buttons = 0xFFFF;
    caps->gamepad.left_trigger = 0xFF;
    caps->gamepad.right_trigger = 0xFF;
    caps->gamepad.thumb_lx = (int16_t)0x7FFF;
    caps->gamepad.thumb_ly = (int16_t)0x7FFF;
    caps->gamepad.thumb_rx = (int16_t)0x7FFF;
    caps->gamepad.thumb_ry = (int16_t)0x7FFF;
    return 0;
  }
  return res;
}

} // namespace

REX_HOOK(__imp__XamInputGetState, Custom_XamInputGetState_entry)
REX_HOOK(__imp__XamInputGetCapabilities, Custom_XamInputGetCapabilities_entry)
REX_HOOK(__imp__XamInputGetCapabilitiesEx, Custom_XamInputGetCapabilitiesEx_entry)

#include "generated/default/samshosen_init.h"

namespace {

constexpr uint16_t VK_PAD_A = 0x5800;
constexpr uint16_t VK_PAD_B = 0x5801;
constexpr uint16_t VK_PAD_X = 0x5802;
constexpr uint16_t VK_PAD_Y = 0x5803;
constexpr uint16_t VK_PAD_RSHOULDER = 0x5804;
constexpr uint16_t VK_PAD_LSHOULDER = 0x5805;
constexpr uint16_t VK_PAD_LTRIGGER = 0x5806;
constexpr uint16_t VK_PAD_RTRIGGER = 0x5807;
constexpr uint16_t VK_PAD_DPAD_UP = 0x5810;
constexpr uint16_t VK_PAD_DPAD_DOWN = 0x5811;
constexpr uint16_t VK_PAD_DPAD_LEFT = 0x5812;
constexpr uint16_t VK_PAD_DPAD_RIGHT = 0x5813;
constexpr uint16_t VK_PAD_START = 0x5814;
constexpr uint16_t VK_PAD_BACK = 0x5815;
constexpr uint16_t VK_PAD_LTHUMB_PRESS = 0x5816;
constexpr uint16_t VK_PAD_RTHUMB_PRESS = 0x5817;

struct KeystrokeEvent {
  uint16_t vk;
  uint16_t flags;
  uint8_t user_index;
};

struct KeyBind {
  int vk_win;
  uint16_t vk_pad;
  uint8_t user_index;
};

static const KeyBind kKeyBinds[] = {
  // Player 1
  { VK_UP, VK_PAD_DPAD_UP, 0 },
  { VK_DOWN, VK_PAD_DPAD_DOWN, 0 },
  { VK_LEFT, VK_PAD_DPAD_LEFT, 0 },
  { VK_RIGHT, VK_PAD_DPAD_RIGHT, 0 },
  { 'A', VK_PAD_A, 0 },
  { 'S', VK_PAD_B, 0 },
  { 'D', VK_PAD_X, 0 },
  { 'F', VK_PAD_Y, 0 },
  { 'Q', VK_PAD_LSHOULDER, 0 },
  { 'W', VK_PAD_RSHOULDER, 0 },
  { 'E', VK_PAD_LTRIGGER, 0 },
  { 'R', VK_PAD_RTRIGGER, 0 },
  { 'T', VK_PAD_LTHUMB_PRESS, 0 },
  { VK_RETURN, VK_PAD_START, 0 },
  { VK_SPACE, VK_PAD_A, 0 },
  { VK_CONTROL, VK_PAD_BACK, 0 },
  { VK_ESCAPE, VK_PAD_BACK, 0 },

  // Player 2 (Numpad + números superiores 0-9)
  { VK_NUMPAD8, VK_PAD_DPAD_UP, 1 },
  { '8', VK_PAD_DPAD_UP, 1 },
  { VK_NUMPAD2, VK_PAD_DPAD_DOWN, 1 },
  { VK_NUMPAD5, VK_PAD_DPAD_DOWN, 1 },
  { '2', VK_PAD_DPAD_DOWN, 1 },
  { '5', VK_PAD_DPAD_DOWN, 1 },
  { VK_NUMPAD4, VK_PAD_DPAD_LEFT, 1 },
  { '4', VK_PAD_DPAD_LEFT, 1 },
  { VK_NUMPAD6, VK_PAD_DPAD_RIGHT, 1 },
  { '6', VK_PAD_DPAD_RIGHT, 1 },
  { VK_NUMPAD1, VK_PAD_A, 1 },
  { '1', VK_PAD_A, 1 },
  { VK_NUMPAD3, VK_PAD_B, 1 },
  { '3', VK_PAD_B, 1 },
  { VK_NUMPAD7, VK_PAD_X, 1 },
  { '7', VK_PAD_X, 1 },
  { VK_NUMPAD9, VK_PAD_Y, 1 },
  { '9', VK_PAD_Y, 1 },
  { VK_NUMPAD0, VK_PAD_LSHOULDER, 1 },
  { '0', VK_PAD_LSHOULDER, 1 },
  { VK_DECIMAL, VK_PAD_RSHOULDER, 1 },
  { VK_OEM_PERIOD, VK_PAD_RSHOULDER, 1 },
  { VK_DIVIDE, VK_PAD_LTRIGGER, 1 },
  { VK_OEM_2, VK_PAD_LTRIGGER, 1 },
  { VK_MULTIPLY, VK_PAD_RTRIGGER, 1 },
  { VK_ADD, VK_PAD_START, 1 },
  { VK_OEM_PLUS, VK_PAD_START, 1 },
  { VK_SUBTRACT, VK_PAD_BACK, 1 },
  { VK_OEM_MINUS, VK_PAD_BACK, 1 },
};

constexpr size_t kNumKeyBinds = sizeof(kKeyBinds) / sizeof(kKeyBinds[0]);

struct KeyBindState {
  bool is_down = false;
  std::chrono::steady_clock::time_point next_repeat;
};

static KeyBindState s_bind_states[kNumKeyBinds];
static std::deque<KeystrokeEvent> s_keystroke_queue;
static std::mutex s_input_mutex;

static void PollKeystrokes() {
  HWND fg = GetForegroundWindow();
  DWORD fg_pid = 0;
  if (fg) GetWindowThreadProcessId(fg, &fg_pid);
  bool focused = (!fg || fg_pid == GetCurrentProcessId());

  auto now = std::chrono::steady_clock::now();

  for (size_t i = 0; i < kNumKeyBinds; ++i) {
    bool down = focused && ((GetAsyncKeyState(kKeyBinds[i].vk_win) & 0x8000) != 0);

    if (down && !s_bind_states[i].is_down) {
      s_bind_states[i].is_down = true;
      s_bind_states[i].next_repeat = now + std::chrono::milliseconds(280);
      s_keystroke_queue.push_back({ kKeyBinds[i].vk_pad, 0x0001, kKeyBinds[i].user_index });
    } else if (down && s_bind_states[i].is_down) {
      if (now >= s_bind_states[i].next_repeat) {
        s_bind_states[i].next_repeat = now + std::chrono::milliseconds(80);
        s_keystroke_queue.push_back({ kKeyBinds[i].vk_pad, 0x0005, kKeyBinds[i].user_index });
      }
    } else if (!down && s_bind_states[i].is_down) {
      s_bind_states[i].is_down = false;
      s_keystroke_queue.push_back({ kKeyBinds[i].vk_pad, 0x0002, kKeyBinds[i].user_index });
    }
  }

  while (s_keystroke_queue.size() > 64) {
    s_keystroke_queue.pop_front();
  }
}

static uint32_t FetchKeystroke(uint32_t user_index, uint32_t flags, uint32_t pKeystroke, uint8_t* base, uint32_t* out_user) {
  std::lock_guard<std::mutex> lock(s_input_mutex);
  PollKeystrokes();

  for (auto it = s_keystroke_queue.begin(); it != s_keystroke_queue.end(); ++it) {
    if (user_index == 255 || it->user_index == user_index) {
      KeystrokeEvent ev = *it;
      s_keystroke_queue.erase(it);

      if (out_user) *out_user = ev.user_index;

      if (pKeystroke && base) {
        REX_STORE_U16(pKeystroke + 0, ev.vk);
        REX_STORE_U16(pKeystroke + 2, 0);
        REX_STORE_U16(pKeystroke + 4, ev.flags);
        REX_STORE_U8(pKeystroke + 6, ev.user_index);
        REX_STORE_U8(pKeystroke + 7, 0);
      }
      return 0; // ERROR_SUCCESS
    }
  }

  return 4306; // ERROR_EMPTY
}

} // namespace

extern "C" REX_FUNC(sub_825A8B18) {
  uint32_t user_index = ctx.r3.u32;
  uint32_t flags = ctx.r4.u32;
  uint32_t pKeystroke = ctx.r5.u32;

  uint32_t out_user = user_index;
  uint32_t res = FetchKeystroke(user_index, flags, pKeystroke, base, &out_user);
  ctx.r3.u64 = res;
}

extern "C" REX_FUNC(__imp__XamInputGetKeystrokeEx) {
  uint32_t pUserIndex = ctx.r3.u32;
  uint32_t flags = ctx.r4.u32;
  uint32_t pKeystroke = ctx.r5.u32;

  uint32_t req_user = 255;
  if (pUserIndex && !(flags & 0x40000000)) {
    req_user = REX_LOAD_U32(pUserIndex);
  }

  uint32_t out_user = 0;
  uint32_t res = FetchKeystroke(req_user, flags, pKeystroke, base, &out_user);
  if (res == 0 && pUserIndex) {
    REX_STORE_U32(pUserIndex, out_user);
  }
  ctx.r3.u64 = res;
}

extern "C" REX_FUNC(sub_821C4730) {
  uint32_t obj = ctx.r3.u32;
  if (obj) {
    REX_STORE_U32(obj + 0, 0x28000002);
    REX_STORE_U32(obj + 12, 1280);
    REX_STORE_U32(obj + 16, 720);
    REX_STORE_U32(obj + 20, 1);
    REX_STORE_U32(obj + 48, 1280);
    REX_STORE_U32(obj + 52, 0);
  }
  ctx.r3.u64 = 0;
}

extern "C" REX_FUNC(sub_821C54A0) {
  uint32_t obj = ctx.r3.u32;
  if (obj) {
    REX_STORE_U32(obj + 0, 0x28000002);
    REX_STORE_U32(obj + 12, 1280);
    REX_STORE_U32(obj + 16, 720);
    REX_STORE_U32(obj + 20, 1);
    REX_STORE_U32(obj + 48, 1280);
    REX_STORE_U32(obj + 52, 0);
  }
  ctx.r3.u64 = 0;
}

extern "C" REX_FUNC(sub_821C3B78) {
  uint32_t obj = ctx.r3.u32;
  if (obj) {
    REX_STORE_U32(obj + 0, 0x28000002);
    REX_STORE_U32(obj + 12, 1280);
    REX_STORE_U32(obj + 16, 720);
    REX_STORE_U32(obj + 20, 1);
    REX_STORE_U32(obj + 48, 1280);
    REX_STORE_U32(obj + 52, 0);
  }
  ctx.r3.u64 = 0;
}

extern "C" REX_FUNC(sub_821C4AB0) {
  uint32_t obj = ctx.r3.u32;
  if (obj) {
    REX_STORE_U32(obj + 0, 0x28000002);
    REX_STORE_U32(obj + 12, 1280);
    REX_STORE_U32(obj + 16, 720);
    REX_STORE_U32(obj + 20, 1);
    REX_STORE_U32(obj + 48, 1280);
    REX_STORE_U32(obj + 52, 0);
  }
  ctx.r3.u64 = 0;
}

extern "C" REX_FUNC(sub_821C6220) {
  uint32_t obj = ctx.r3.u32;
  uint32_t out_meta = ctx.r6.u32;

  if (obj) {
    REX_STORE_U32(obj + 0, 0x28000002);
    REX_STORE_U32(obj + 12, 1280);
    REX_STORE_U32(obj + 16, 720);
    REX_STORE_U32(obj + 20, 1);
    REX_STORE_U32(obj + 48, 1280);
    REX_STORE_U32(obj + 52, 0);
    REX_STORE_U32(obj + 56, 0);
    REX_STORE_U32(obj + 60, 0);
    REX_STORE_U32(obj + 76, 0);
    REX_STORE_U32(obj + 80, 0);
  }

  if (out_meta) {
    REX_STORE_U32(out_meta + 0, 1280);
    REX_STORE_U32(out_meta + 4, 720);
    REX_STORE_U32(out_meta + 8, 1);
    REX_STORE_U32(out_meta + 12, 1);
    REX_STORE_U32(out_meta + 16, 0);
    REX_STORE_U32(out_meta + 20, 0);
    REX_STORE_U32(out_meta + 24, 0);
  }

  ctx.r3.u64 = 0;
}
