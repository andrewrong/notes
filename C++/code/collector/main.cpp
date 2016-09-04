#include <iostream>
#include "src/monitor.h"
#include "src/TimeDb.h"
#include "src/ThreadPool.h"

int main() {
    std::shared_ptr<monitor> m = std::make_shared<monitor>();

    TimeDb timeDb(m.get());
    ThreadPool threadPool(m.get());

    m->status();
}