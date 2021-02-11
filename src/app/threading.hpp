#pragma once

struct ThreadStatus {
    bool runDetector;
    bool runWebsocket;
    bool detectorClosed;
    bool websocketClosed;
};
