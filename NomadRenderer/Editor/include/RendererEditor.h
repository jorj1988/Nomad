#pragma once
#include "shift/sglobal.h"

#if defined(NOMADCORE_BUILD)
#  define NOMAD_RENDERER_EDITOR_EXPORT X_DECL_EXPORT
#else
#  define NOMAD_RENDERER_EDITOR_EXPORT X_DECL_IMPORT
#endif

S_MODULE(NOMAD_RENDERER_EDITOR_EXPORT, NomadRendererEditor)
