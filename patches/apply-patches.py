"""
 Copyright (C) 2023  shezik
 
 espnut is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.  Note that the permission
 to redistribute or modify espnut under the terms of any later
 version of the General Public License is denied by the author
 of Nonpareil, Eric L. Smith, according to his notice.
 
 espnut is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program (in the file "COPYING" or "LICENSE"); if not,
 write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111, USA.
"""

from os.path import join, isfile, isdir, exists, split, realpath
Import("env")

# User options
patching_libs = [('GEM/src', 'GEM-1.5.1.patch')]

PROJECT_DIR = env.subst('$PROJECT_DIR')
PATCHES_DIR = join(PROJECT_DIR, 'patches')
# PATCHPY_PATH = join(split(realpath(__file__))[0], 'patch.py')
PATCHPY_PATH = join(PATCHES_DIR, 'patch.py')
LIBDEPS_DIR = env.subst('$PROJECT_LIBDEPS_DIR')
PIOENV = env.subst('$PIOENV')

def _touch(path):
        with open(path, "w") as fp:
            fp.write("")

show_patched_flag_hint = False

for patching_lib in patching_libs:
    lib_path = join(LIBDEPS_DIR, PIOENV, patching_lib[0])
    assert isdir(lib_path)

    patched_flag_path = join(lib_path, '.patched')
    if exists(patched_flag_path):
        print('Library already patched: {}'.format(lib_path))
        show_patched_flag_hint = True
        continue

    patch_path = join(PATCHES_DIR, patching_lib[1])
    assert isfile(patch_path)

    env.Execute('"$PYTHONEXE" "{}" -d "{}" "{}"'.format(PATCHPY_PATH, lib_path, patch_path))\

if show_patched_flag_hint:
    print('Remove the \'.patched\' file in the directories above to reapply patches.')

# Maybe you would like to apply multiple patches to one library.
# Flags are created AFTER patching everything.
for patching_lib in patching_libs:
    lib_path = join(LIBDEPS_DIR, PIOENV, patching_lib[0])
    patched_flag_path = join(lib_path, '.patched')

    if not exists(patched_flag_path):
        env.Execute(lambda *args, **kwargs: _touch(patched_flag_path))
