
# Wednesday, 6 August 2025
Comitted everything. In this time I implemented the following:
 - *Changed Build Tool*
    - Went from VSCode to CMake, using the Ninja Build tool. Created a CMakeList.txt and linked the libraries together.
 - Removed intentional memory leaks. In Game::Update(), there is a part of the code responsible for deleting "killed" dots, but the code only set the pointer of the Dot to a nullptr, and didn't actually delete it. Later I will remove the deletion entirely.
 - Implemented QuadTree. Nothing to say here, this was pretty simple.

My current tasks are:
 -[X] Implement a logging system
 -[X] Implement a Debug UI system (could be apart of logging too)

Implemented the logging system and for the UI as well, it's quite nice and handy to easily add new debug info to the screen.

My next task is as follows:
 - [ ] Analyze the size of each dot, using less memory == able to have more dots
