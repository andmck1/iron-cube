# Project overview
- This is a graphics application built in C++ using Vulkan for Linux.

# Architectural Principles
- This project use a layered architecture to abstract platform, 
renderer, resource management, scene management and app code.
- This project uses a service locator pattern for accessing services
by using interfaces for function signatures.

# Dev Tips
- Always check the file structure in src/ first.
- Always check for available packages in the root CMakeLists.txt
- Always add new files to the relevant CMakeLists.txt file for that module.
- All private member variables have a trailing _, functions come before
properties and public comes before private.
- Always run clang format after making files:
```bash
find src -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i {} +
```

# AI Tips
- Always compact and summarise before running tasks.
- This is a British English project so no Americanisms in the code
(eg. initialize over initialise).

