#!/bin/bash

# this makes sure errors are bubbled up and cause the script to fail
set -euxo pipefail

timestamp=$(date +"%y%m%d_%H%M%S")
shaderpipelines=(
    "asteroidInstancedVertex"
    "asteroidInstancedVertexCompute"
    "asteroidMesh"
    "asteroidMeshCompute"
    "asteroidMeshDXExample"
)
ninstances=(
    10
    100
    1000
    10000
    100000
    1000000
)

dro_instance_ratio=376.923

for shaderpipeline in "${shaderpipelines[@]}"; do
    for ninstances in "${ninstances[@]}"; do

        disc_outer_radius=$(awk -v r="$dro_instance_ratio" -v n="$ninstances" \
            'BEGIN { print sqrt(r * n) }')
        exe_command="$(pwd)/build/bin/iron-cube"
        exe_args="\
            --shader-pipeline $shaderpipeline \
            --disc-inner-radius 25 \
            --disc-outer-radius $disc_outer_radius \
            --disc-height 100 \
            --n-instances $ninstances"

        save_directory="$(pwd)/data/$timestamp/$shaderpipeline/i$ninstances"
        mkdir -p $save_directory

        /home/dtaunt/nvidia/NVIDIA-Nsight-Graphics-2026.2/host/linux-desktop-nomad-x64/ngfx \
            --activity "GPU Trace Profiler" \
            --exe "$exe_command" \
            --args "$exe_args" \
            --dir "$(pwd)" \
            --limit-to-frames 100 \
            --max-duration-ms 1000000 \
            --architecture "Ampere GA10x" \
            --metric-set-id 0 \
            --hes-enabled 1 \
            --per-line-active-threads-per-warp 1 \
            --multi-pass-metrics \
            --time-every-action \
            --auto-export \
            --output-dir "$save_directory"
    done
done
