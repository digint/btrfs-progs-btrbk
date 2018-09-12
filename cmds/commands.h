/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#ifndef __BTRFS_COMMANDS_H__
#define __BTRFS_COMMANDS_H__

enum {
	CMD_HIDDEN = (1 << 0),	/* should not be in help listings */
	CMD_ALIAS = (1 << 1),	/* alias of next command in cmd_group */
	CMD_FORMAT_TEXT = (1 << 2),	/* output as plain text */
	CMD_FORMAT_JSON = (1 << 3),	/* output in json */
};

#define CMD_FORMAT_MASK		(CMD_FORMAT_TEXT | CMD_FORMAT_JSON)

struct cmd_struct {
	const char *token;
	int (*fn)(const struct cmd_struct *cmd, int argc, char **argv);

	/*
	 * Usage strings
	 *
	 * A NULL-terminated array of the following format:
	 *
	 *   usagestr[0] - one-line synopsis (required)
	 *   usagestr[1] - one-line short description (required)
	 *   usagestr[2..m] - a long (possibly multi-line) description
	 *                    (optional)
	 *   usagestr[m + 1] - an empty line separator (required if at least one
	 *                     option string is given, not needed otherwise)
	 *   usagestr[m + 2..n] - option strings, one option per line
	 *                        (optional)
	 *   usagestr[n + 1] - NULL terminator
	 *
	 * Options (if present) should always (even if there is no long
	 * description) be prepended with an empty line.  Supplied strings are
	 * indented but otherwise printed as-is, no automatic wrapping is done.
	 *
	 * Grep for cmd_*_usage[] for examples.
	 */
	const char * const *usagestr;

	/* should be NULL if token is not a subgroup */
	const struct cmd_group *next;

	/* CMD_* flags above */
	int flags;
};

/*
 * These macros will create cmd_struct structures with a standard name:
 * cmd_struct_<name>.
 */
#define __CMD_NAME(name)	cmd_struct_ ##name
#ifdef BTRFS_SEPARATED_BUILD
#define DECLARE_COMMAND(name)
#else
#define DECLARE_COMMAND(name)						\
	extern const struct cmd_struct __CMD_NAME(name)
#endif

/* Define a command with all members specified */
#define DEFINE_COMMAND(name, _token, _fn, _usagestr, _group, _flags)	\
	const struct cmd_struct __CMD_NAME(name) =			\
		{							\
			.token = (_token),				\
			.fn = (_fn),					\
			.usagestr = (_usagestr),			\
			.next = (_group),				\
			.flags = CMD_FORMAT_TEXT | (_flags),		\
		}

/*
 * Define a command for the common case - just a name and string.
 * It's assumed that the callback is called cmd_<name> and the usage
 * array is named cmd_<name>_usage.
 */
#define DEFINE_SIMPLE_COMMAND(name, token)				\
	DEFINE_COMMAND(name, token, cmd_ ##name,			\
		       cmd_ ##name ##_usage, NULL, 0)

/*
 * Define a command with flags, eg. with the additional output formats.
 * See CMD_* .
 */
#define DEFINE_COMMAND_WITH_FLAGS(name, token, flags)			\
	DEFINE_COMMAND(name, token, cmd_ ##name,			\
		       cmd_ ##name ##_usage, NULL, (flags))

/*
 * Define a command group callback.
 * It's assumed that the callback is called cmd_<name> and the
 * struct cmd_group is called <name>_cmd_group.
 */
#ifdef BTRFS_SEPARATED_BUILD
#define DEFINE_GROUP_COMMAND(name, token)
#else
#define DEFINE_GROUP_COMMAND(name, token)				\
	DEFINE_COMMAND(name, token, handle_command_group,		\
		       NULL, &(name ## _cmd_group), 0)
#endif

/*
 * Define a command group callback when the name and the string are
 * the same.
 */
#define DEFINE_GROUP_COMMAND_TOKEN(name)				\
	DEFINE_GROUP_COMMAND(name, #name)

struct cmd_group {
	const char * const *usagestr;
	const char *infostr;

	const struct cmd_struct * const commands[];
};

static inline int cmd_execute(const struct cmd_struct *cmd,
			      int argc, char **argv)
{
	return cmd->fn(cmd, argc, argv);
}

#ifndef BTRFS_SEPARATED_BUILD
int handle_command_group(const struct cmd_struct *cmd, int argc, char **argv);
#endif

extern const char * const generic_cmd_help_usage[];

DECLARE_COMMAND(subvolume);
DECLARE_COMMAND(subvolume_list);
DECLARE_COMMAND(filesystem);
DECLARE_COMMAND(filesystem_du);
DECLARE_COMMAND(filesystem_usage);
DECLARE_COMMAND(balance);
DECLARE_COMMAND(device);
DECLARE_COMMAND(scrub);
DECLARE_COMMAND(check);
DECLARE_COMMAND(inspect);
DECLARE_COMMAND(inspect_dump_super);
DECLARE_COMMAND(inspect_dump_tree);
DECLARE_COMMAND(inspect_tree_stats);
DECLARE_COMMAND(property);
DECLARE_COMMAND(send);
DECLARE_COMMAND(receive);
DECLARE_COMMAND(reflink);
DECLARE_COMMAND(quota);
DECLARE_COMMAND(qgroup);
DECLARE_COMMAND(replace);
DECLARE_COMMAND(restore);
DECLARE_COMMAND(rescue);


#ifdef BTRFS_SEPARATED_BUILD

#ifndef BTRFS_SEPARATED_ENTRY
#error please define BTRFS_SEPARATED_ENTRY (see Makefile: "btrfs-%.separated.o" target)
#endif

#include "common/help.h"
#include "common/utils.h"
#include "kernel-shared/volumes.h"

/* Note: handle_command_group is defined in btrfs.c and cannot be
 * linked with separated subcommands because btrfs.o also contains a
 * "main" symbol. As a workaround, we simply return 1 (error) for
 * calls to handle_command_group() here (which is fine as this
 * functionality is not required for BTRFS_SEPARATED_BUILD commands).
 */
#define handle_command_group(cmd_group,argc,argv) 1

/* forward declaration of main entry point (non-static are already declared above) */
static int BTRFS_SEPARATED_ENTRY(const struct cmd_struct *cmd, int argc, char **argv);
static const char * const BTRFS_SEPARATED_USAGE [];

#ifdef BTRFS_SEPARATED_STATIC_CMD_STRUCT
static const struct cmd_struct BTRFS_SEPARATED_CMD_STRUCT;
#else
const struct cmd_struct BTRFS_SEPARATED_CMD_STRUCT;
#endif

int main(int argc, char **argv)
{
	int ret;
	int i;

	set_argv0(argv);
	btrfs_config_init();

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			usage_command(& BTRFS_SEPARATED_CMD_STRUCT, true, false);
			exit(0);
		}
		if (strcmp(argv[i], "--version") == 0) {
			printf("%s\n", PACKAGE_STRING);
			exit(0);
		}
	}

	ret = cmd_execute(& BTRFS_SEPARATED_CMD_STRUCT, argc, argv);
	btrfs_close_all_devices();
	exit(ret);
}

#endif  /* BTRFS_SEPARATED_BUILD */

#endif
