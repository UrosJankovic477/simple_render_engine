#include <sre/event/app_run.h>

static bool app_running;

bool SRE_App_running()
{
    return app_running;
}
void SRE_App_start()
{
    app_running = true;
}

void SRE_App_quit()
{
    app_running = false;
}
