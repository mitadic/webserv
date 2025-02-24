#include "ServerEngine.hpp"


volatile std::sig_atomic_t g_signal = 0;  // declared in ServerEngine.hpp


int main(void) {

    ServerEngine engine;

    engine.run();
}