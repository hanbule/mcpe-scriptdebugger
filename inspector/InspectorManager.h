#pragma once

#include <v8-inspector.h>
#include <mutex>
#include <condition_variable>
#include <set>

namespace InspectorServer {

class InspectorWebSocketSession;

class InspectorManager : protected v8_inspector::V8InspectorClient {

private:
    std::unique_ptr<v8_inspector::V8Inspector> inspector;
    std::set<InspectorWebSocketSession*> channels;
    std::mutex channelsMutex;


    std::condition_variable runMessageLoopCv;
    std::mutex runMessageLoopMutex;
    bool runMessageLoopBool = true;


protected:
    void runMessageLoopOnPause(int contextGroupId) override;

    void quitMessageLoopOnPause() override;

public:
    static InspectorManager instance;

    void init(v8::Isolate* isolate, v8::Local<v8::Context> context);

    void update();


    void onConnectionOpened(InspectorWebSocketSession& channel);

    void onConnectionClosed(InspectorWebSocketSession& channel);

};

}