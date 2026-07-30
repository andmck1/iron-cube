#include "playbook.h"

#include <chrono>
#include <format>

void ExperimentPlaybook::init(Application *application)
{
    application_ = application;
    currentPlaybookIndex_ = 0;
    currentApplicationSettings_ = &playbook[currentPlaybookIndex_];

    std::chrono::time_point now = std::chrono::system_clock::now();

    outputDirectory_ = std::format(
        "{:%Y%m%d_%H%M%S}", std::chrono::floor<std::chrono::seconds>(now)
    );
}

void ExperimentPlaybook::run()
{
    runNGFXCommand();
    next();
}

void ExperimentPlaybook::runNGFXCommand()
{
}

void ExperimentPlaybook::next()
{
    currentPlaybookIndex_++;
    currentApplicationSettings_ = &playbook[currentPlaybookIndex_];
}
