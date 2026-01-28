#pragma once

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <tgbot/tgbot.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <string>
#include <nlohmann/json.hpp>
#include "logging/Logger.hpp"