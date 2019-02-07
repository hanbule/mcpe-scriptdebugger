#include "statichook.h"
#include "inspector/InspectorServer.h"
#include "inspector/InspectorManager.h"

extern "C" void* mcpelauncher_hook(void* symbol, void* hook, void** original) { return nullptr; }

bool ON_SERVER_THREAD();

namespace ScriptApi {
    struct V8CoreInterface {
        char filler[0x48];
        v8::Isolate *isolate; // 0x4C
        v8::Persistent<v8::Context> context; // 0x50
        char filler2[0x7C - 0x50];
        InspectorServer::InspectorManager inspectorManager;
    };
    struct ScriptFramework {
        int filler;
        V8CoreInterface* core;
    };
}
struct ScriptEngine : public ScriptApi::ScriptFramework {
};

InspectorServer::InspectorServer inspectorServer;


TInstanceHook(void, _ZN9ScriptApi15V8CoreInterfaceC2Ev, ScriptApi::V8CoreInterface) {
    original(this);
    new (&inspectorManager)InspectorServer::InspectorManager;
}
TInstanceHook(void, _ZN9ScriptApi15V8CoreInterfaceD2Ev, ScriptApi::V8CoreInterface) {
    if (ON_SERVER_THREAD())
        inspectorServer.removeInspector("server");
    else
        inspectorServer.removeInspector("client");
    inspectorManager.~InspectorManager();
    original(this);
}
TInstanceHook(void, _ZN9ScriptApi15V8CoreInterface10initializeERNS_12ScriptReportE, ScriptApi::V8CoreInterface, void* report) {
    original(this, report);
    v8::HandleScope handleScope (isolate);
    inspectorManager.init(isolate, context.Get(isolate));
    if (ON_SERVER_THREAD())
        inspectorServer.addInspector("server", &inspectorManager);
    else
        inspectorServer.addInspector("client", &inspectorManager);
}
TInstanceHook(void, _ZN12ScriptEngine6updateEv, ScriptEngine) {
    original(this);
    core->inspectorManager.update();
}


extern "C" void mod_init() {
    void* sym = dlsym(MinecraftHandle(), "_ZN9ScriptApi15ScriptFrameworkC2Ev");
    const char* s = &((const char*) sym)[0x22 + 3];
    if (*(int*) s != 0x7C)
        throw std::runtime_error("V8CoreInterface size changed");
    *((int*) s) += sizeof(InspectorServer::InspectorManager);

    inspectorServer.start(4242);
}
