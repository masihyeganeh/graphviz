#!/usr/bin/env python3

"""
test cases that are only relevant to run in CI
"""

import os
import platform
import subprocess
import sys

import pytest

sys.path.append(os.path.join(os.path.dirname(__file__), "../tests"))
from gvtest import (  # pylint: disable=wrong-import-position
    dot,
    freedesktop_os_release,
    is_cmake,
    is_macos,
    is_mingw,
    which,
)


def is_win64() -> bool:
    """
    is the Graphviz under test targeting the x64 Windows API?
    """
    if platform.system() != "Windows":
        return False
    if os.getenv("project_platform") != "x64":
        return False
    return True


@pytest.mark.parametrize(
    "binary",
    [
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
    ],
)
def test_existence(binary: str):
    """
    check that a given binary was built and is on $PATH
    """

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

    tools_not_built_with_cmake = [
        "dot_builtins",
    ]

    os_id = freedesktop_os_release().get("ID")

    # FIXME: Remove skip when
    # https://gitlab.com/graphviz/graphviz/-/issues/1834 is fixed
    if os_id in ("centos", "rocky") and binary == "smyrna":
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip("smyrna is not built for Centos (#1834)")

    if os_id == "rocky" and binary == "mingle":
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip("mingle is not built for Rocky due to lacking libANN")

    if is_cmake() and binary in tools_not_built_with_cmake:
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip(f"{binary} is not built with CMake")

    # FIXME: Remove skip when
    # https://gitlab.com/graphviz/graphviz/-/issues/1837 is fixed
    if os.getenv("build_system") == "msbuild":
        if binary in tools_not_built_with_msbuild:
            check_that_tool_does_not_exist(binary, os_id)
            pytest.skip(f"{binary} is not built with MSBuild (#1837)")

    if binary == "mingle" and is_cmake() and (is_win64() or is_mingw()):
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip(f"{binary} is not built on some Windows due to lacking libANN")

    if binary == "diffimg" and is_win64():
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip(f"{binary} is not built on 64-bit Windows due to lacking libgd")

    if binary == "gvedit" and platform.system() == "Windows" and not is_mingw():
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip(f"{binary} is not built on Windows due to lacking Qt")

    # FIXME: Smyrna dependencies are not avaiable in other jobs
    if binary == "smyrna" and is_cmake() and platform.system() != "Linux":
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip("smyrna is not built on non-Linux due to lacking dependencies")

    if binary == "gvmap.sh" and is_mingw():
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip(f"{binary} is not installed on MinGW")

    if binary == "vimdot" and platform.system() == "Windows":
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip(f"{binary} is not installed on Windows")

    if binary == "smyrna" and not is_cmake() and is_macos():
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip("https://gitlab.com/graphviz/graphviz/-/issues/2422")

    if binary == "vimdot" and not is_cmake() and is_macos():
        check_that_tool_does_not_exist(binary, os_id)
        pytest.skip("https://gitlab.com/graphviz/graphviz/-/issues/2423")

    assert which(binary) is not None


def check_that_tool_does_not_exist(tool, os_id):
    """
    validate that the given tool does *not* exist
    """
    assert which(tool) is None, (
        f"{tool} has been resurrected in the "
        f'{os.getenv("build_system")} build on {os_id}. Please remove skip.'
    )


@pytest.mark.xfail(
    is_cmake() and not is_mingw() and is_win64(),
    reason="png:gd unavailable when built with CMake",
    strict=True,
)  # FIXME
def test_1786():
    """
    png:gd format should be supported
    https://gitlab.com/graphviz/graphviz/-/issues/1786
    """

    # run a trivial graph through Graphviz
    dot("png:gd", source="digraph { a -> b; }")


def test_2469():
    """
    svgz format should be supported
    https://gitlab.com/graphviz/graphviz/-/issues/2469
    """

    # run a trivial graph through Graphviz
    dot("svgz", source="digraph { a -> b; }")


def test_installation():
    """
    check that Graphviz reports the expected version number
    """

    expected_version = os.environ["GV_VERSION"]

    actual_version_string = subprocess.check_output(
        [
            "dot",
            "-V",
        ],
        universal_newlines=True,
        stderr=subprocess.STDOUT,
    )
    try:
        actual_version = actual_version_string.split()[4]
    except IndexError:
        pytest.fail(f"Malformed version string: {actual_version_string}")
    assert actual_version == expected_version
