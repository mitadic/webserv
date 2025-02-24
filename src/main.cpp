#include "ServerEngine.hpp"
#include "../incl/ServerBlock.hpp"
#include "../incl/Config.hpp"


volatile std::sig_atomic_t g_signal = 0;  // declared in ServerEngine.hpp


int main(int argc, char **argv) {

    ServerEngine engine;
    
    engine.run();
}