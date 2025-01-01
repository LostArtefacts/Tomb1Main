#!/usr/bin/env python3
from pathlib import Path

from shared.cli.game_docker_entrypoint import run_script

run_script(
    version=2,
    platform="linux",
    compile_args=[],
    release_zip_files=[
        (Path("/app/build/tr2/linux/TR2X"), "TR2X"),
    ],
    compressable_exes=[
        Path("/app/build/tr2/linux/TR2X"),
    ],
)
