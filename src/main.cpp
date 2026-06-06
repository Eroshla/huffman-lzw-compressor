#include "cli/orchestrator.hpp"

int main(int argc, char* argv[]) {
    Orchestrator orch;
    orch.run(argc, argv);
    return 0;
}
