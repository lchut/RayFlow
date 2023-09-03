#pragma once
#include <tbb/tbb.h>
#include <functional>
#include <RayFlow/Util/vecmath.h>

namespace rayflow {

class Scheduler {
public:
    Scheduler() = default;

    static void Parallel2D(const Point2i& count, std::function<void(Point2i)> task) {
        tbb::task_arena arena;
        arena.execute([&](){tbb::parallel_for(tbb::blocked_range2d<int>(0, count.y, 0, count.x), 
        [=](const tbb::blocked_range2d<int, int>& r) {
        for (int y = r.rows().begin(); y != r.rows().end(); ++y) {
            for (int x = r.cols().begin(); x != r.cols().end(); ++x) {
                task(Point2i(x, y));
            }
        }
    }
    , tbb::auto_partitioner());});
    }

    static Scheduler* GetInstance() {
        return mSchedulerInstance;
    }

    static Scheduler* mSchedulerInstance;
};

}