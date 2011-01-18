#include <systopo.h>

using namespace systopo;

int main(void)
{
    System * s = getSystemTopology();
    delete s;

    return 0;
}
