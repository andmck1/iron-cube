#!/bin/bash

# this makes sure errors are bubbled up and cause the script to fail
set -euxo pipefail

# timestamp=$(date +"%y%m%d_%H%M%S")
timestamp="260801_135508"
shaderpipelines=(
    # "asteroidInstancedVertex"
    # "asteroidInstancedVertexCompute"
    # "asteroidMesh"
    "asteroidMeshCompute"
    "asteroidMeshDXExample"
)
ninstances=(
    10
    # 100
    # 1000
    # 10000
    # 100000
    # 1000000
    # 10000000
)
frame_limit=60

dro_instance_ratio=376.923

for shaderpipeline in "${shaderpipelines[@]}"; do
    for ninstances in "${ninstances[@]}"; do

        save_directory="$(pwd)/data/$timestamp/$shaderpipeline/i$ninstances"
        mkdir -p $save_directory
        # mkdir "$save_directory/report"

        disc_outer_radius=$(awk -v r="$dro_instance_ratio" -v n="$ninstances" \
            'BEGIN { print sqrt(r * n) }')
        exe_command="$(pwd)/build/bin/iron-cube"
        exe_args="\
            --shader-pipeline $shaderpipeline \
            --disc-inner-radius 25 \
            --disc-outer-radius $disc_outer_radius \
            --disc-height 100 \
            --n-instances $ninstances \
            --enable-debug-write"
        app_metrics_filepath_arg="
            --output-debug-data-filepath $save_directory/app_metrics.csv"
        app_metrics_with_gpu_filepath_arg="
            --output-debug-data-filepath $save_directory/app_metrics_with_gpu.csv"

        # ngfx run, with app data collected
        /home/dtaunt/nvidia/NVIDIA-Nsight-Graphics-2026.2/host/linux-desktop-nomad-x64/ngfx \
            --activity "GPU Trace Profiler" \
            --exe "$exe_command" \
            --args "$exe_args $app_metrics_filepath_arg" \
            --dir "$(pwd)" \
            --limit-to-frames $frame_limit \
            --architecture "Ampere GA10x" \
            --no-timeout \
            --metric-set-id 0 \
            --hes-enabled 1 \
            --per-line-active-threads-per-warp 1 \
            --multi-pass-metrics \
            --time-every-action \
            --auto-export \
            --output-dir "$save_directory"

        # app run with vram data collected
        $exe_command $exe_args $app_metrics_with_gpu_filepath_arg --enable-gpu-debug-write --frame-limit $frame_limit

        # nsys run
        # nsys profile \
        #     --trace="vulkan,nvtx,cuda,vulkan-annotations" \
        #     --sample=cpu \
        #     --cpuctxsw=process-tree \
        #     --gpu-metrics-devices=all \
        #     --cuda-memory-usage=true \
        #     --gpu-metrics-frequency=10000 \
        #     --force-overwrite=true \
        #     --duration=10 \
        #     --output "$save_directory/report" \
        #     "$exe_command" $exe_args
        # nsys stats "$save_directory/report.sqlite"
    done
done
