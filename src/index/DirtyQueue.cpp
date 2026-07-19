#include "DirtyQueue.h"
#include <iostream>

void DirtyQueue::Mark(
    TriangleId triangle)
{

    std::lock_guard<std::mutex> lock(mutex);

    if (triangle >= exists.size())
    {
        exists.resize(
            triangle + 1,
            false);
    }

    if (exists[triangle])
    {
        return;
    }

    exists[triangle] = true;

    queue.push(
        triangle);
}

bool DirtyQueue::Empty() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return queue.empty();
}

TriangleId DirtyQueue::Pop()
{
    std::lock_guard<std::mutex> lock(mutex);

    TriangleId id =
        queue.front();

    queue.pop();

    static int counter = 0;

    ++counter;

    if (counter % 50000 == 0)
    {
        std::cout
            << "DirtyQueue processed: "
            << counter
            << std::endl;
    }

    return id;
}

void DirtyQueue::Complete(
    TriangleId triangle)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (triangle < exists.size())
    {
        exists[triangle] = false;
    }
}


void DirtyQueue::Clear()
{
    std::lock_guard<std::mutex> lock(mutex);

    while (!queue.empty())
    {
        queue.pop();
    }

    exists.clear();
}