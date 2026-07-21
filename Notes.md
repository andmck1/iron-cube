# Useful commands

Format all files

```bash
find src -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i {} +
```

Find and replace across subdirectory

```bash
find src -type f -exec sed -i 's/Initializ/Initialis/g' {} +;
```

Neovim:

- In snacks picker go Tab -> C-a -> C-q to move items to quickfix list
- Tab is forward where Ctrl-O is back!

# gists

```cpp
uint nInstances = updateData_->instances->size();
float instanceDensity =
    nInstances /
    (PI * *updateData_->discHeight *
     ((*updateData_->discOuterRadius * *updateData_->discOuterRadius)
     -
      (*updateData_->discInnerRadius *
      *updateData_->discInnerRadius)));
long maximumPossibleWork = 0;
for (uint lodIndex = 0; lodIndex < MAX_LOD_COUNT - 1; lodIndex++)
{
    uint32_t nLODMeshlets =
        static_cast<uint32_t>(geometry_->meshletLODCounts[lodIndex]);

    float lodRadius =
        *updateData_->maxLODDistance *
        std::pow(
            (float) (lodIndex + 1) / (float) (MAX_LOD_COUNT - 1),
            1.0 / *updateData_->lodExponent
        );
    float lodVolume = 4.0 / 3.0 * PI * std::pow(lodRadius, 3.0);
    if (lodIndex > 0)
    {
        float previousLODRadius =
            *updateData_->maxLODDistance *
            std::pow(
                (float) (lodIndex - 1 + 1) / (float) (MAX_LOD_COUNT -
                1), 1.0 / *updateData_->lodExponent
            );
        float previousLODVolume =
            4.0 / 3.0 * PI * std::pow(previousLODRadius, 3.0);
        lodVolume -= previousLODVolume;
    }
    maximumPossibleWork += nLODMeshlets * lodVolume;
}
{
    uint lodIndex = MAX_LOD_COUNT - 1;
    uint32_t nLODMeshlets =
        static_cast<uint32_t>(geometry_->meshletLODCounts[lodIndex]);
    float lodRadius = updateData_->camera->getFarClip();
    float lodVolume = 4.0 / 3.0 * PI * std::pow(lodRadius, 3.0);
    if (lodIndex > 0)
    {
        float previousLODRadius =
            *updateData_->maxLODDistance *
            std::pow(
                (float) (lodIndex - 1 + 1) / (float) (MAX_LOD_COUNT -
                1), 1.0 / *updateData_->lodExponent
            );
        float previousLODVolume =
            4.0 / 3.0 * PI * std::pow(previousLODRadius, 3.0);
        lodVolume -= previousLODVolume;
    }
    maximumPossibleWork += nLODMeshlets * lodVolume;
}
maximumPossibleWork *= instanceDensity;
```

```cpp
uint32_t averageMeshletCount = 0;
long _maximumPossibleWork = meshletCount * instanceCount;
long maximumPossibleWork = _maximumPossibleWork;
std::array<float, MAX_LOD_COUNT> lodWeights;
if (*updateData_->isLODOn)
{
    // ------------------------------
    uint nInstances = updateData_->instances->size();
    long maximumPossibleWork = 0;

    uint lodIndex = 0;
    float prevLODRadius = 0.f;
    for (lodIndex = 0; lodIndex < MAX_LOD_COUNT; lodIndex++)
    {
        uint32_t nLODMeshlets =
            static_cast<uint32_t>(geometry_->meshletLODCounts[lodIndex]);
        float lodRadius;
        if (lodIndex == MAX_LOD_COUNT - 1)
            lodRadius = updateData_->camera->getFarClip();
        else
            lodRadius =
                *updateData_->maxLODDistance *
                std::pow(
                    (float) (lodIndex + 1) / (float) (MAX_LOD_COUNT - 1),
                    1.0 / *updateData_->lodExponent
                );

        // TODO - this should be square law
        float lodWeight =
            (lodRadius - prevLODRadius) / updateData_->camera->getFarClip();
        lodWeights[lodIndex] = lodWeight;

        maximumPossibleWork += nInstances * nLODMeshlets * lodWeight;
        prevLODRadius = lodRadius;

        averageMeshletCount += lodWeight * nLODMeshlets;
    }
    // ------------------------------
    // spdlog::info(
    //     "Work Difference: {}", maximumPossibleWork - _maximumPossibleWork
    // );
}
```
