# UBIQUANT CHALLENGE: MATCH ENGINE
## TODO
- [ ] Use `mmap` to read the file and test the performance between `mmap` and `read` system call.
- [ ] Use multi-thread to accelerate the match engine.
- [ ] Try to design a better network communication framework such as `epoll`.

## Environment Setup
### Step 1: Check for CMake

1. Open a terminal window.
2. Type the following command to check if CMake is installed on your system:

```bash
cmake --version
```

If CMake is installed, you should see the version number in the output. If it's not installed, follow the instructions below to install it.

### For Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y cmake
```

### For macOS:

You can install CMake using [Homebrew](https://brew.sh/), a package manager for macOS. If you don't have Homebrew installed, you can install it by pasting the following command in your terminal:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Once Homebrew is installed, you can install CMake with the following command:

```bash
brew install cmake
```

Now you have CMake installed on your machine and you're ready to move on to the next step of your environment setup.

## Introduction
...