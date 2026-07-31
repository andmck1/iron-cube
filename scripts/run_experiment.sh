#!/bin/bash
timestamp=$(date +"%y%m%d_%H%M%S")

shaderpipelines=(
    "asteroidInstancedVertex"
    "asteroidInstancedVertexCompute"
    "asteroidMesh"
    "asteroidMeshCompute"
    "asteroidMeshDXExample"
)

ninstances_power=(
    10
    100
    1000
    10000
    100000
    1000000
    10000000
)

for i in $(seq 0 4); do
    ngfx --activity "GPU Trace Profiler" \
        --exe "$(pwd)/build/bin/iron-cube --shader-pipeline $shaderpipeline " \
        --dir "$(pwd)" \
        --limit-to-frames 60 \
        --architecture "Ampere GA10x" \
        --metric-set-id 0 \
        --hes-enabled 1 \
        --per-line-active-threads-per-warp 1 \
        --multi-pass-metrics \
        --time-every-action \
        --auto-export \
        --output-dir "./data/$timestamp"
done
