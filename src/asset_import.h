#ifndef SRE_ASSET_IMPORT_H
#define SRE_ASSET_IMPORT_H

#include "mesh.h"
#include "armature.h"
#include "material.h"

int SRE_Import_asset(sre_importer *importer, const char *filepath, bool always_loaded);

#endif