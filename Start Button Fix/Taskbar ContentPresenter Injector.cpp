// ==WindhawkMod==
// @id              taskbar-content-presenter-injector
// @name            Taskbar ContentPresenter Injector
// @description     Injects a ContentPresenter into Taskbar.TaskListLabeledButtonPanel and Taskbar.TaskListButtonPanel
// @version         1.3
// @author          Lockframe (Fix by Acercandr0)
// @github          https://github.com/Lockframe
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*...*/
// ==/WindhawkModReadme==

#include <windhawk_utils.h>

#undef GetCurrentTime

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/base.h>

#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <set>

using namespace winrt::Windows::UI::Xaml;

std::atomic<bool> g_taskbarViewDllLoaded = false;

struct TrackedPanelRef {
    winrt::weak_ref<Controls::Panel> ref;
};

std::vector<TrackedPanelRef> g_trackedPanels;
std::mutex g_panelMutex;

winrt::weak_ref<FrameworkElement> g_cachedTaskbarFrame;

const std::wstring c_TargetPanelLabeled  = L"Taskbar.TaskListLabeledButtonPanel";
const std::wstring c_TargetPanelButton   = L"Taskbar.TaskListButtonPanel";
const std::wstring c_RootFrameName       = L"Taskbar.TaskbarFrame";
const std::wstring c_InjectedControlName = L"CustomInjectedPresenter";

// -------------------------------------------------------------------------
// Original Function Pointers
// -------------------------------------------------------------------------
using TaskListButton_UpdateVisualStates_t = void(WINAPI*)(void*);
TaskListButton_UpdateVisualStates_t TaskListButton_UpdateVisualStates_Original;

using TaskListButton_UpdateButtonPadding_t = void(WINAPI*)(void*);
TaskListButton_UpdateButtonPadding_t TaskListButton_UpdateButtonPadding_Original;

using ExperienceToggleButton_UpdateVisualStates_t = void(WINAPI*)(void*);
ExperienceToggleButton_UpdateVisualStates_t ExperienceToggleButton_UpdateVisualStates_Original;

// -------------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------------

FrameworkElement GetFrameworkElementFromNative(void* pThis) {
    try {
        void* iUnknownPtr = (void**)pThis + 3;
        winrt::Windows::Foundation::IUnknown iUnknown;
        winrt::copy_from_abi(iUnknown, iUnknownPtr);
        return iUnknown.try_as<FrameworkElement>();
    } catch (...) {
        return nullptr;
    }
}

void RegisterPanelForCleanup(Controls::Panel const& panel) {
    if (!panel) return;
    void* pAbi = winrt::get_abi(panel);

    std::lock_guard<std::mutex> lock(g_panelMutex);
    auto it = g_trackedPanels.begin();
    while (it != g_trackedPanels.end()) {
        auto existing = it->ref.get();
        if (!existing) {
            it = g_trackedPanels.erase(it);
        } else {
            if (winrt::get_abi(existing) == pAbi) return;
            ++it;
        }
    }
    g_trackedPanels.push_back({ winrt::make_weak(panel) });
}

bool IsAlreadyInjected(Controls::Panel panel) {
    for (auto child : panel.Children()) {
        if (auto elem = child.try_as<FrameworkElement>()) {
            if (elem.Name() == c_InjectedControlName) return true;
        }
    }
    return false;
}

// -------------------------------------------------------------------------
// Inyección diferida: espera a LayoutUpdated para garantizar que el
// panel esté completamente construido antes de tocar su árbol visual.
// -------------------------------------------------------------------------
void InjectContentPresenterIntoPanel(FrameworkElement targetPanel) {
    if (!targetPanel) return;

    auto panel = targetPanel.try_as<Controls::Panel>();
    if (!panel) return;

    RegisterPanelForCleanup(panel);

    if (IsAlreadyInjected(panel)) return;

    auto weakPanel  = winrt::make_weak(panel);
    auto tokenHolder = std::make_shared<winrt::event_token>();

    *tokenHolder = targetPanel.LayoutUpdated(
        [weakPanel, tokenHolder](winrt::Windows::Foundation::IInspectable const&,
                                  winrt::Windows::Foundation::IInspectable const&) {

            auto p = weakPanel.get();
            if (!p) return;

            p.try_as<FrameworkElement>().LayoutUpdated(*tokenHolder);

            if (IsAlreadyInjected(p)) return;

            auto fe = p.try_as<FrameworkElement>();
            if (fe && (fe.ActualWidth() <= 0 || fe.ActualHeight() <= 0)) {
                auto tokenHolder2 = std::make_shared<winrt::event_token>();
                *tokenHolder2 = fe.LayoutUpdated(
                    [weakPanel, tokenHolder2](winrt::Windows::Foundation::IInspectable const&,
                                              winrt::Windows::Foundation::IInspectable const&) {
                        auto p2 = weakPanel.get();
                        if (!p2) return;
                        p2.try_as<FrameworkElement>().LayoutUpdated(*tokenHolder2);
                        if (IsAlreadyInjected(p2)) return;

                        Controls::ContentPresenter presenter;
                        presenter.Name(c_InjectedControlName);
                        presenter.HorizontalAlignment(HorizontalAlignment::Stretch);
                        presenter.VerticalAlignment(VerticalAlignment::Stretch);
                        p2.Children().Append(presenter);
                    });
                return;
            }

            Controls::ContentPresenter presenter;
            presenter.Name(c_InjectedControlName);
            presenter.HorizontalAlignment(HorizontalAlignment::Stretch);
            presenter.VerticalAlignment(VerticalAlignment::Stretch);
            p.Children().Append(presenter);
        });
}

void ScanAndInjectRecursive(FrameworkElement element) {
    if (!element) return;

    auto className = winrt::get_class_name(element);
    if (className == c_TargetPanelLabeled || className == c_TargetPanelButton) {
        InjectContentPresenterIntoPanel(element);
        return;
    }

    int count = Media::VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = Media::VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child) ScanAndInjectRecursive(child);
    }
}

void ScheduleScanAsync(FrameworkElement startNode) {
    if (!startNode) return;
    auto weak = winrt::make_weak(startNode);

    try {
        startNode.Dispatcher().RunAsync(
            winrt::Windows::UI::Core::CoreDispatcherPriority::Low,
            [weak]() {
                if (auto node = weak.get()) {
                    FrameworkElement current = node;
                    while (current) {
                        if (winrt::get_class_name(current) == c_RootFrameName) {
                            g_cachedTaskbarFrame = winrt::make_weak(current);
                            ScanAndInjectRecursive(current);
                            return;
                        }
                        auto parent = Media::VisualTreeHelper::GetParent(current);
                        current = parent ? parent.try_as<FrameworkElement>() : nullptr;
                    }
                    ScanAndInjectRecursive(node);
                }
            });
    } catch (...) {}
}

// -------------------------------------------------------------------------
// Cleanup
// -------------------------------------------------------------------------
void RemoveInjectedFromPanel(Controls::Panel panel) {
    if (!panel) return;
    try {
        auto children = panel.Children();
        for (int i = (int)children.Size() - 1; i >= 0; i--) {
            if (auto fe = children.GetAt(i).try_as<FrameworkElement>()) {
                if (fe.Name() == c_InjectedControlName) children.RemoveAt(i);
            }
        }
    } catch (...) {}
}

// -------------------------------------------------------------------------
// Hooks
// -------------------------------------------------------------------------
void InjectForElement(void* pThis) {
    try {
        if (auto elem = GetFrameworkElementFromNative(pThis)) {
            if (auto frame = g_cachedTaskbarFrame.get()) {
                auto weakFrame = winrt::make_weak(frame);
                frame.Dispatcher().RunAsync(
                    winrt::Windows::UI::Core::CoreDispatcherPriority::Low,
                    [weakFrame]() {
                        if (auto f = weakFrame.get()) ScanAndInjectRecursive(f);
                    });
            } else {
                ScheduleScanAsync(elem);
            }
        }
    } catch (...) {}
}

void WINAPI TaskListButton_UpdateVisualStates_Hook(void* pThis) {
    TaskListButton_UpdateVisualStates_Original(pThis);
    InjectForElement(pThis);
}

void WINAPI TaskListButton_UpdateButtonPadding_Hook(void* pThis) {
    TaskListButton_UpdateButtonPadding_Original(pThis);
    InjectForElement(pThis);
}

void WINAPI ExperienceToggleButton_UpdateVisualStates_Hook(void* pThis) {
    ExperienceToggleButton_UpdateVisualStates_Original(pThis);
    InjectForElement(pThis);
}

// -------------------------------------------------------------------------
// Init / Uninit
// -------------------------------------------------------------------------
bool HookTaskbarViewDllSymbols(HMODULE module) {
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))"},
            &TaskListButton_UpdateVisualStates_Original,
            TaskListButton_UpdateVisualStates_Hook,
        },
        {
            {LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateButtonPadding(void))"},
            &TaskListButton_UpdateButtonPadding_Original,
            TaskListButton_UpdateButtonPadding_Hook,
        },
        {
            {LR"(private: void __cdecl winrt::Taskbar::implementation::ExperienceToggleButton::UpdateVisualStates(void))"},
            &ExperienceToggleButton_UpdateVisualStates_Original,
            ExperienceToggleButton_UpdateVisualStates_Hook,
            true
        }
    };

    if (!HookSymbols(module, hooks, ARRAYSIZE(hooks))) {
        Wh_Log(L"Failed to hook Taskbar.View.dll symbols");
        return false;
    }
    return true;
}

HMODULE GetTaskbarViewModuleHandle() {
    HMODULE m = GetModuleHandle(L"Taskbar.View.dll");
    return m ? m : GetModuleHandle(L"ExplorerExtensions.dll");
}

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE module = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (module && !g_taskbarViewDllLoaded &&
        GetTaskbarViewModuleHandle() == module &&
        !g_taskbarViewDllLoaded.exchange(true)) {

        Wh_Log(L"Taskbar View DLL loaded: %s", lpLibFileName);
        if (HookTaskbarViewDllSymbols(module)) Wh_ApplyHookOperations();
    }
    return module;
}

BOOL Wh_ModInit() {
    Wh_Log(L"Initializing Taskbar Injector Mod");

    if (HMODULE m = GetTaskbarViewModuleHandle()) {
        g_taskbarViewDllLoaded = true;
        if (!HookTaskbarViewDllSymbols(m)) return FALSE;
    } else {
        HMODULE kb = GetModuleHandle(L"kernelbase.dll");
        auto pLoadLibraryExW = (decltype(&LoadLibraryExW))GetProcAddress(kb, "LoadLibraryExW");
        WindhawkUtils::Wh_SetFunctionHookT(pLoadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    }
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L"Uninitializing Taskbar Injector Mod");

    std::vector<TrackedPanelRef> local;
    {
        std::lock_guard<std::mutex> lock(g_panelMutex);
        local = std::move(g_trackedPanels);
    }

    for (auto& tracked : local) {
        if (auto panel = tracked.ref.get()) {
            auto dispatcher = panel.Dispatcher();
            auto cleanupFn = [panel]() { RemoveInjectedFromPanel(panel); };

            if (dispatcher.HasThreadAccess()) {
                cleanupFn();
            } else {
                try {
                    dispatcher.RunAsync(
                        winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                        cleanupFn).get();
                } catch (...) {}
            }
        }
    }

    g_cachedTaskbarFrame = nullptr;
}