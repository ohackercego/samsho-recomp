// crash_diag.cpp -- last-chance access-violation logger (debug aid, not for shipping).
//
// When a recompiled title boots but then dies with a bare 0xC0000005 and no stack
// (the SDK's VEH only fixes up guest memory faults; anything it doesn't handle
// falls through unlogged), drop this into your project to find out WHERE. It
// installs a low-priority VEH (runs AFTER the SDK's, so it only sees genuinely
// unhandled AVs) and writes the fault address, operation, and a symbolized stack
// walk to crash_diag.log.
//
// Build RelWithDebInfo so the recompiled sub_XXXX / rex:: frames resolve to names:
//
//     target_sources(mygame PRIVATE src/crash_diag.cpp)
//     target_link_libraries(mygame PRIVATE dbghelp)
//     cmake --preset win-amd64-relwithdebinfo && cmake --build out/build/win-amd64-relwithdebinfo
//
// Reading the result: a fault address of base+0 (host 0x100000000) is a guest
// NULL dereference -- try the `--protect_zero=false` cvar for reads and
// dispatch_tolerance.cpp for calls. The top guest sub_XXXX frame is the function
// to investigate; map it back via your generated code / a disassembler.

#include <windows.h>
#include <tlhelp32.h>
#include <dbghelp.h>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <chrono>
#pragma comment(lib, "dbghelp.lib")

namespace {

void symline(FILE* f, HANDLE proc, DWORD64 addr, int idx) {
  char buf[sizeof(SYMBOL_INFO) + 512];
  auto* si = reinterpret_cast<SYMBOL_INFO*>(buf);
  si->SizeOfStruct = sizeof(SYMBOL_INFO);
  si->MaxNameLen = 511;
  IMAGEHLP_MODULE64 mod;
  mod.SizeOfStruct = sizeof(mod);
  const char* modname = SymGetModuleInfo64(proc, addr, &mod) ? mod.ModuleName : "?";
  DWORD64 disp = 0;
  if (SymFromAddr(proc, addr, &disp, si))
    fprintf(f, "  [%2d] %s!%s +0x%llX  (0x%llX)\n", idx, modname, si->Name,
            (unsigned long long)disp, (unsigned long long)addr);
  else
    fprintf(f, "  [%2d] %s!0x%llX\n", idx, modname, (unsigned long long)addr);
  // Source file:line (RelWithDebInfo) -- maps straight to the generated sub_XXXX cpp.
  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(line);
  DWORD line_disp = 0;
  if (SymGetLineFromAddr64(proc, addr, &line_disp, &line))
    fprintf(f, "         %s:%lu\n", line.FileName, line.LineNumber);
}

LONG CALLBACK Veh(EXCEPTION_POINTERS* ep) {
  auto* er = ep->ExceptionRecord;
  if (er->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;

  FILE* f = fopen("crash_diag.log", "a");
  if (!f) return EXCEPTION_CONTINUE_SEARCH;

  HANDLE proc = GetCurrentProcess();
  static bool inited = false;
  if (!inited) {
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(proc, nullptr, TRUE);
    inited = true;
  }

  const char* op = er->ExceptionInformation[0] == 1   ? "write"
                   : er->ExceptionInformation[0] == 0 ? "read"
                                                      : "exec";
  CONTEXT* ctx = ep->ContextRecord;
  fprintf(f, "\n==== ACCESS VIOLATION (%s 0x%llX) thread %lu ====\n", op,
          (unsigned long long)er->ExceptionInformation[1], GetCurrentThreadId());
  fprintf(f, "faulting instruction:\n");
  symline(f, proc, ctx->Rip, 0);

  fprintf(f, "stack:\n");
  STACKFRAME64 sf{};
  sf.AddrPC.Offset = ctx->Rip;    sf.AddrPC.Mode = AddrModeFlat;
  sf.AddrFrame.Offset = ctx->Rbp; sf.AddrFrame.Mode = AddrModeFlat;
  sf.AddrStack.Offset = ctx->Rsp; sf.AddrStack.Mode = AddrModeFlat;
  CONTEXT walk = *ctx;
  for (int i = 0; i < 48; ++i) {
    if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, proc, GetCurrentThread(), &sf, &walk, nullptr,
                     SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
      break;
    if (!sf.AddrPC.Offset) break;
    symline(f, proc, sf.AddrPC.Offset, i);
  }
  fflush(f);
  fclose(f);
  return EXCEPTION_CONTINUE_SEARCH;  // let it crash as before
}

void DumpAllThreads() {
  FILE* f = fopen("thread_diag.log", "w");
  if (!f) return;
  fprintf(f, "==== THREAD DIAGNOSTIC DUMP ====\n");

  HANDLE proc = GetCurrentProcess();
  SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
  SymInitialize(proc, nullptr, TRUE);

  DWORD myPid = GetCurrentProcessId();
  DWORD myTid = GetCurrentThreadId();

  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (snap != INVALID_HANDLE_VALUE) {
    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(snap, &te)) {
      do {
        if (te.th32OwnerProcessID == myPid && te.th32ThreadID != myTid) {
          HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
          if (hThread) {
            SuspendThread(hThread);
            CONTEXT ctx{};
            ctx.ContextFlags = CONTEXT_FULL;
            if (GetThreadContext(hThread, &ctx)) {
              fprintf(f, "\n--- Thread ID %lu (RIP: 0x%llX) ---\n", te.th32ThreadID, (unsigned long long)ctx.Rip);
              symline(f, proc, ctx.Rip, 0);
              STACKFRAME64 sf{};
              sf.AddrPC.Offset = ctx.Rip; sf.AddrPC.Mode = AddrModeFlat;
              sf.AddrFrame.Offset = ctx.Rbp; sf.AddrFrame.Mode = AddrModeFlat;
              sf.AddrStack.Offset = ctx.Rsp; sf.AddrStack.Mode = AddrModeFlat;
              CONTEXT walk = ctx;
              for (int i = 1; i < 32; ++i) {
                if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, proc, hThread, &sf, &walk, nullptr,
                                 SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
                  break;
                if (!sf.AddrPC.Offset) break;
                symline(f, proc, sf.AddrPC.Offset, i);
              }
            }
            ResumeThread(hThread);
            CloseHandle(hThread);
          }
        }
      } while (Thread32Next(snap, &te));
    }
    CloseHandle(snap);
  }
  fprintf(f, "\n==== END OF DUMP ====\n");
  fflush(f);
  fclose(f);
}

void WatchdogWorker() {
  std::this_thread::sleep_for(std::chrono::seconds(5));
  DumpAllThreads();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  DumpAllThreads();
}

}  // namespace

extern "C" void InitDiagnostics() {
  static bool done = false;
  if (!done) {
    done = true;
    AddVectoredExceptionHandler(0, Veh);
    std::thread(WatchdogWorker).detach();
  }
}
