#!/usr/bin/env python

# On Windows, static libraries are using "foo.lib" naming.
# On Unix, they are in the form of "libfoo.a"
# On Windows, "foo.lib" can also the definition of symbols in a foo.dll.
# So you can link to "foo.lib", whatever you are doing static or dynamic linking and you are good.
# However, meson is always creating static library as "libfoo.a"
# 'to avoid a potential name clash with shared libraries which also generate import libraries with a lib suffix.' [1]
# On top of that, qmake is replacing all `-lfoo` in LIBS by `foo.lib` (on Windows).
# So at the end, we try to link with `foo.lib` but we have `libfoo.a`
# Solution could be :
# - Rename `libfoo.a` to `foo.lib`, but it would mean modify deps libraries on the FS
# - Don't use LIBS and directly set QMAKE_LFLAGS but we would have to handle different command line option format
#   between g++/clang and msvc
# - Update meson build system of each projet to explicitly set the library naming.
# - Replace `-lfoo` with absolute path to static library. This is what meson is doing internally and what
#   we are doing here
#
# Any `-lfoo` is replace with absolute path to static library (`libfoo.a`) if we found one.
# Else, it is keep unchanged.
#
# [1] https://mesonbuild.com/Reference-manual_functions.html#library_name_suffix

import sys, subprocess
from pathlib import Path


def forward_to_pkg_config():
    completeProcess = subprocess.run(
        ["pkg-config", *sys.argv[1:]], capture_output=True, check=True, text=True
    )
    return completeProcess.stdout


def search_static_lib(lib_name, search_paths):
    for path in search_paths:
        lib_path = path / f"lib{lib_name}.a"
        if lib_path.exists():
            return str(lib_path)
    return None


def replace_static_lib(pkg_output):
    search_paths = []
    for option in pkg_output.split():
        if option.startswith("-L"):
            search_paths.append(Path(option[2:]))
            yield option
        if option.startswith("-l"):
            static_lib = search_static_lib(option[2:], search_paths)
            if static_lib:
                yield static_lib
            else:
                yield option


if __name__ == "__main__":
    pkg_output = forward_to_pkg_config()
    print(" ".join(replace_static_lib(pkg_output)))
