#include "pi/runtime/capability.h"

#include <string.h>

/* =---- Path Validation ---------------------------------------= */

PI_Api(bool) piIsPathAllowed(const PiCapabilityContext *const ctx, const char *const path)
{
    size_t i, prefixLen;

    if (!ctx || !path)
        return false;

    /* If FS_ESCAPE is set, all paths are allowed */
    if (piHasCapability(ctx, PI_CAP_FS_ESCAPE))
        return true;

    /* If no allowed paths configured, all paths are allowed */
    if (!ctx->allowedPaths || ctx->allowedPathCount == 0)
        return true;

    /* Check if path starts with any of the allowed prefixes */
    for (i = 0; i < ctx->allowedPathCount; i++)
    {
        if (!ctx->allowedPaths[i])
            continue;

        prefixLen = strlen(ctx->allowedPaths[i]);

        if (strncmp(path, ctx->allowedPaths[i], prefixLen) == 0)
            return true;
    }

    return false;
}

/* =------------------------------------------------------------= */
