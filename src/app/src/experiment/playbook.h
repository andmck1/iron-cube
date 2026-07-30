#include "application/application.h"
#include <string>
#include <vector>

struct ApplicationSettings
{
    std::string shaderPipeline;
};

static std::vector<ApplicationSettings> playbook{
    {.shaderPipeline = "AsteroidInstancedVertex"},
    {.shaderPipeline = "AsteroidInstancedVertexCompute"},
    {.shaderPipeline = "AsteroidMesh"},
    {.shaderPipeline = "AsteroidMeshCompute"},
    {.shaderPipeline = "AsteroidMeshDXExample"},

};

class ExperimentPlaybook
{
    public:
    void init(Application *application);
    void run();

    bool isFinished = false;

    private:
    void runNGFXCommand();
    void next();

    ApplicationSettings *currentApplicationSettings_ = nullptr;
    unsigned int currentPlaybookIndex_ = 0;
    std::string ngfxCommand_ = "ngfx --activity \"GPU Trace Profiler\" "
                               "--exe \"$(pwd)/build/bin/iron-cube\" "
                               "--dir \"$(pwd)\" "
                               "--limit-to-frames 60 "
                               "--architecture \"Ampere GA10x\" "
                               "--metric-set-id 0 "
                               "--hes-enabled 1 "
                               "--per-line-active-threads-per-warp 1 "
                               "--multi-pass-metrics "
                               "--time-every-action "
                               "--auto-export "
                               "--keep-going "
                               "--output-dir ./data/{}";
    std::string outputDirectory_ = "";

    Application *application_ = nullptr;
};
