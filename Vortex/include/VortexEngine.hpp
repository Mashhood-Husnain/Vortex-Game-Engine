#pragma once

#include "vortex_behaviour.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"
#include "vortex_physics.hpp"
#include "vortex_audio.hpp"
#include "vortex_uimanager.hpp"
#include "vortex_application.hpp"
#include "vortex_camera.hpp"
#include "vortex_debugrenderer.hpp"

#include "vortex_global_vars.hpp"
#include "vortex_script_registry.hpp"
#include "vortex_game_api.hpp"
#include "vortex_game_reloader.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cstdlib>
#include <cmath>
#include <string>
#include <algorithm>

using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
using Quaternion = glm::quat;

using glm::angleAxis;
using glm::rotation;
using glm::distance;
using glm::normalize;
using glm::length;
