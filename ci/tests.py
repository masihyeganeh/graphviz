#!/usr/bin/env python3

"""
test cases that are only relevant to run in CI
"""

import os
import platform
import shutil
import sys
import pytest

sys.path.append(os.path.join(os.path.dirname(__file__), "../rtest"))
from gvtest import dot #pylint: disable=C0413

def is_asan_instrumented() -> bool:
  """
  was the Graphviz under test built with Address Sanitizer?
  """
  return os.environ["CI_JOB_NAME"] in (
    "ubuntu21.04-cmake-ASan-test-including-ctest",
    "ubuntu21.10-cmake-ASan-test-including-ctest",
  )

def is_cmake() -> bool:
  """
  is the current CI environment a CMake job?
  """
  return os.getenv("build_system") == "cmake"

@pytest.mark.parametrize("binary", [
  "acyclic",
  "bcomps",
  "ccomps",
  "circo",
  "cluster",
  "diffimg",
  "dijkstra",
  "dot",
  "dot2gxl",
  "dot_builtins",
  "dotty",
  "edgepaint",
  "fdp",
  "gc",
  "gml2gv",
  "graphml2gv",
  "gv2gml",
  "gv2gxl",
  "gvcolor",
  "gvedit",
  "gvgen",
  "gvmap",
  "gvmap.sh",
  "gvpack",
  "gvpr",
  "gxl2dot",
  "gxl2gv",
  "lneato",
  "mingle",
  "mm2gv",
  "neato",
  "nop",
  "osage",
  "patchwork",
  "prune",
  "sccmap",
  "sfdp",
  "smyrna",
  "tred",
  "twopi",
  "unflatten",
  "vimdot",
])
def test_existence(binary: str):
  """
  check that a given binary was built and is on $PATH
  """

  tools_not_built_with_cmake = [
    "cluster",
    "diffimg",
    "dot_builtins",
    "dotty",
    "edgepaint",
    "gv2gxl",
    "gvedit",
    "gvmap",
    "gvmap.sh",
    "gxl2dot",
    "lneato",
    "mingle",
    "prune",
    "smyrna",
    "vimdot",
  ]

  tools_not_built_with_msbuild = [
    "cluster",
    "dot2gxl",
    "dot_builtins",
    "gv2gxl",
    "gvedit",
    "gvmap.sh",
    "gxl2dot",
    "vimdot",
  ]

  tools_not_built_with_autotools_on_macos = [
    "mingle",
    "smyrna",
  ]

  os_id = os.getenv("OS_ID")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1835 is fixed
  if os_id == "ubuntu" and binary == "mingle":
    check_that_tool_does_not_exist(binary, os_id)
    pytest.skip(f"mingle is not built for {os_id} (#1835)")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1834 is fixed
  if os_id == "centos" and binary == "smyrna":
    check_that_tool_does_not_exist(binary, os_id)
    pytest.skip("smyrna is not built for Centos (#1834)")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1753 and
  # https://gitlab.com/graphviz/graphviz/-/issues/1836 is fixed
  if os.getenv("build_system") == "cmake":
    if binary in tools_not_built_with_cmake:
      check_that_tool_does_not_exist(binary, os_id)
      pytest.skip(f"{binary} is not built with CMake (#1753 & #1836)")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1837 is fixed
  if os.getenv("build_system") == "msbuild":
    if binary in tools_not_built_with_msbuild:
      check_that_tool_does_not_exist(binary, os_id)
      pytest.skip(f"{binary} is not built with MSBuild (#1837)")

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1854 is fixed
  if os.getenv("build_system") == "autotools":
    if platform.system() == "Darwin":
      if binary in tools_not_built_with_autotools_on_macos:
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip(f"{binary} is not built with autotools on macOS (#1854)")

  assert shutil.which(binary) is not None

def check_that_tool_does_not_exist(tool, os_id):
  """
  validate that the given tool does *not* exist
  """
  assert shutil.which(tool) is None, f"{tool} has been resurrected in the " \
    f'{os.getenv("build_system")} build on {os_id}. Please remove skip.'

@pytest.mark.xfail(is_cmake() and platform.system() != "Linux"
                   and not is_asan_instrumented(),
                   reason="png:gd unavailable when built with CMake",
                   strict=True) # FIXME
def test_1786():
  """
  png:gd format should be supported
  https://gitlab.com/graphviz/graphviz/-/issues/1786
  """

  # run a trivial graph through Graphviz
  dot("png:gd", source="digraph { a -> b; }")
