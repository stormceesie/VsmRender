#pragma once
// Precompiled header for headers which dont change often
// When one changes the whole PCH will recompile

#include <vulkan/vulkan.h>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <optional>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <algorithm>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>