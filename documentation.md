
# Wednesday, 6 August 2025
Committed everything. In this time I implemented the following:
- *Changed Build Tool*
    - Went from VSCode to CMake, using the Ninja Build tool. Created a CMakeList.txt and linked the
    libraries together.
- Removed intentional memory leaks. In Game::Update(), there is a part of the code responsible for
deleting "killed" dots, but the code only set the pointer of the Dot to a nullptr, and didn't
actually delete it. Later I will remove the deletion entirely.
- Implemented QuadTree. Nothing to say here, this was pretty simple.

My current tasks are:
- [x] Implement a logging system
- [x] Implement a Debug UI system (could be apart of logging too)

Implemented the logging system and for the UI as well, it's quite nice and handy to easily add new
debug info to the screen.

My next task is as follows:
- [x] Analyze the size of each dot, using less memory == able to have more dots
    - Was able to minimize it to 32 bytes, could do more with bit masking

# Sunday, 17 August 2025
Current Stats:
    [LOG] 1% LOW: 26ms
    [LOG] FPS: 33
    [LOG] RenderTime: 5ms
    [LOG] CollisionTime: 14ms
    [LOG] UpdateTime: 0ms
    [LOG] QuadTime: 6ms
    [LOG] DOTS_AMOUNT: 9000


I reimplemented the QuadTree in a new header file, and it is a lot prettier now and cleaner. Problem is, for some reason went
from 70fps to 35fps, basically halving the performance. I need to look into this.

But for now I'm going to focus on some more fun tasks. 

Next on my list is making the data more cache friendly. Cache-friendly layouts provide the most dramatic improvements. 

## AoS vs SoA:
Structure of Arrays (SoA) over Array of Structures (AoS) can deliver up to 50x faster execution [NumberAnalytics](https://www.numberanalytics.com/blog/ultimate-guide-to-cache-memory-optimization). Storing Arrays of structures leads to worse performance, because say we want to update the position of every dot on the screen. We initially loop through every Dot, and access its position. The CPU, being smart, will load as many dots into the cache as possible, but in the process also load velocity, radius, and other data. This extra data takes up cache space but isn't even being USED! Not to mention, the dots probably have some small bad alignment, meaning there is data in the cache that is completely empty! Here is a text drawn example.

Assuming 72B of cache memory:
```
[ P1, V1, R1, P2, V2, R2, P3, V3, R3 ]
```
                <=>
```
[ P1, P2, P3, P4, P5, P6, P7, P8, P9 ]
```

As you can see, we can fit 3x more positional data into the cache by packing our data with
SoA.

## Loop Unrolling 
Loop unrolling involves transforming a loop by either reducing or removing iterations. Loop
overhead can be attributed to extra CPU instructions and cache misses. For example,

Example 1, simple for loop on array:
```
for(int i = 0; i < 8; i++){
    array[i] = i * 2;
}
```

The overhead for the above code is as follows:
 - 8 counter increments (i++)
 - 8 conditional checks (i < 8)
 - 8 branch instructions (loop body)
 - **Total: 24 control operations**

Example 2, unrolled for loop on array:
```
for(int i = 0; i < 8; i+=4){
    array[i]    = i * 2;
    array[i + 1] = (i + 1) * 2;
    array[i + 2] = (i + 2) * 2;
    array[i + 3] = (i + 3) * 2;
}
```

The overhead for the above code is as follows:
 - 2 counter increments (i += 4)
 - 2 conditional checks (i < 8)
 - 2 branch instructions (loop body)
 - **Total: 6 control operations**

By unrolling this loop, we reduced the overhead by **75.0%**.

It is also worth mentioning, that loop unrolling can be good for cache hits as well. Consider the
following:
```
for(int i = 0; i < 8; i++){
    array[i] = i * 2;               // Cache miss
}
```

When the cache miss hits in this loop, we have to go refill the cache memory. During this time,
the CPU sits idly waiting for the cache to come back.

```
for(int i = 0; i < 8; i+=4){
    array[i]    = i * 2;            // Cache miss
    array[i + 1] = (i + 1) * 2;     // Cache hit
    array[i + 2] = (i + 2) * 2;     // Cache hit
    array[i + 3] = (i + 3) * 2;     // Cache hit
}
```

Here, we got a cache miss on the 2nd line, so we still have to go retrieve new cache memory. But,
there are still 3 operations where we got a cache hit! So while we retrieve the new cache memory, the
CPU has something to do! CPU's often don't run line by line, and in fact when possible will run code
in any order it wants in order to speed up performance. So these other lines will actually be
calculated WHILE the cache is getting retrieved.

So my next tasks are as follows:
- [ ] Change implementation of the Dots. Instead of having a Dot structure, I will have a Dots class
which will store all the data of the dots in seperate arrays. This will surely improve performance.
- [ ] I will look around for any places to do some loop unrolling!

Update:
 - Implemented the SoA, but performance remains the same:
    [LOG] 1% LOW: 28ms          **(+2ms)**
    [LOG] FPS: 33               **(n/a)**
    [LOG] RenderTime: 5ms       **(n/a)**    
    [LOG] CollisionTime: 14ms   **(n/a)**
    [LOG] UpdateTime: 0ms       **(n/a)**
    [LOG] QuadTime: 7ms         **(+1ms)**
    [LOG] DOTS_AMOUNT: 9000     **(n/a)**
    
    I'm curious as to why that is, maybe I need to go deeper. Currently i'm storing the positions and
    velocities as glm::vec2, but for a pure approach it might be better to split it up.

Update:
I did some profiling with a new SimpleProfiler class i created:
    [SimpleProfiler Report]:
        QuadTree query: 22.09ms avg (2650.23ms total, 120 calls)
        filter dead: 0.43ms avg (51.63ms total, 120 calls)
        alive_indices: 0.00ms avg (0.16ms total, 120 calls)
        total: 22.52ms avg (2702.58ms total, 120 calls)
    [LOG] LOGGING KVP DEBUG VALUES:
    [LOG] 1% LOW: 44ms
    [LOG] FPS: 21
    [LOG] RenderTime: 8ms
    [LOG] CollisionTime: 22ms
    [LOG] UpdateTime: 0ms
    [LOG] QuadTime: 12ms
    [LOG] DOTS_AMOUNT: 9000

It seems like mose of the time is taken up by that nasty QuadTree query, an a negligable amount by filtering out the dead ones first. I'm next going to
profile the QuadTree query.

For the query we have the counted stats:

    QuadTree stats: 
    visits=147596 rejects=102962 checks=9848 efficiency=69.8%

    MAX_DOTS=32 MAX_LEVELS=4

Woah, this is really innefficient. We are checking 147k but only actually finding a range we are looking for 70% of the time. Maybe i should try
another approach with a faster lookup time.

Update:
Implemented a Spatial Grid. I'm very impressed. Something to know about Quad Trees is that they perform very well for dynamic objects, where size,
location and velocity all vary. But for uniform objects, distributed evenly with same size and speed, it only creates an overhead. Clocking the new
implementation gives me this:

        [LOG] LOGGING KVP DEBUG VALUES:
        [LOG] 1% LOW: 13ms          **(-31ms / -70.45%)**
        [LOG] FPS: 70               **(+49fps / +333.33%)**
        [LOG] RenderTime: 8ms       **(n/a)**
        [LOG] CollisionTime: 3ms    **(-19ms / -86.36%)**
        [LOG] UpdateTime: 0ms       **(n/a)**
        [LOG] DOTS_AMOUNT: 9000     **(n/a)**

This is really cool. I'm finally back to where i was before, and along the way I could drop that dang QuadTree.

 - [x] Next up is some clean-up. I want to hammer in this profiler I created, and clean up some of my code. It should be pretty simple to get profiling data, such as timings and counts, avg and
screen debug info.
- [ ] Take a look at some optimizations for the renderer.

# Cool pics
    Accidentally caused this cool effect while implementing SoA. If you look closely, you can see that
    the QuadTreeNodes contain vast amounts of dots, yet along an axis there is an insane
    distribution of QuadTreeNodes. This happened because I incorrectly mistyped a 'y' to a 'x' like so:

```
if (!bounds.contains(dots.positions_x[index], dots.positions_x[index])) {
    return false;
}
```
![Accidental, 2025-08-17](images/2025-08-17-screenshot.png "Accidental")

