#pragma once

#include <filesystem>
#include <string>

#include "app_state.hpp"

std::string ReadFileContent(const std::filesystem::path &filePath,
                            size_t maxBytes = kMaxPreviewBytes);

void RenderPreviewWindow(AppState &state);
