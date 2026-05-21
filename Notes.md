# Useful commands

Format all files
```bash
find src -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i {} +
```

Find and replace across subdirectory
```bash
find src -type f -exec sed -i 's/Initializ/Initialis/g' {} +;
```

Neovim:
- In snacks picker go Tab -> C-a -> C-q to move items to quickfix list
