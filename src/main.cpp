#include <hikvision_api/timer.h>

int main(int argc, const char** argv) {

    Timer timer;

    timer.syncToFirstInterval();
    
    std::cout << "sync done" << std::endl;

    return 0;
}