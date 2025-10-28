/**
 * target.h - Handles building and rebuilding of Makefile targets.
 *
 * This file contains logic for determining whether a target or its
 * prerequisites need to be rebuilt, based on modification times and
 * user-specified build flags.
 *
 * Functions:
 *  - handle_target(): Handels recursive target cehcking and rebuild logic.
 *
 * @Author:		Rasmus Mikaelsson (et24rmn)
 * @Date:		2025-10-07
 * @Version:	1.0
 */

#ifndef TARGET_H
#define TARGET_H

#include "parser.h"

/**
 * Determines if a target or its prerequisites need rebuilding.
 *
 * @param target			The name of the target to handle.
 * @param mmakefile			Pointer to the parsed Makefile structure.
 * @param force_build		Force build flag. If true, always rebuilds the target.
 * @param silence_commands	Silence commands flag. If true, suppresses command output.
 * @param fp				File pointer for output (unused here but may be used elsewhere).
 *
 * @return				0 if successful, 1 if an error occurs or a rebuild fails.
 */
int handle_target(const char *target, makefile *mmakefile, int force_build, int silence_commands, FILE *fp);

#endif
