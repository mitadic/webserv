#pragma once

#include <csignal> // For signal handling

extern volatile sig_atomic_t g_signal;  // declaration, 'extern' avoids multiple defs. Init in main.cpp