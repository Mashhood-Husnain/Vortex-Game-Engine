#include "VortexEngine.hpp"

class EnemyTag : public VortexMonoBehaviour {};
class DestructibleTag : public VortexMonoBehaviour {};

VORTEX_REGISTER_SCRIPT(EnemyTag);
VORTEX_REGISTER_SCRIPT(DestructibleTag);
