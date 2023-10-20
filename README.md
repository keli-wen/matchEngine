# UBIQUANT CHALLENGE: MATCH ENGINE
## TODO
- [x] Use `mmap` to read the file and test the performance between `mmap` and `read` system call.
- [ ] Use multi-thread to accelerate the match engine.
- [ ] Try to design a better network communication framework such as `epoll`.

## Environment Setup
### Step 0: Clone the Repo
Use the following command to clone the repo (since we contain the submodule, we need to add `--recursive`)
```bash
git clone --recursive https://github.com/keli-wen/matchEngine.git
```

### Step 1: Check for CMake

1. Open a terminal window.
2. Type the following command to check if CMake is installed on your system:

```bash
cmake --version
```

If CMake is installed, you should see the version number in the output. If it's not installed, follow the instructions below to install it.

#### For Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y cmake
```

#### For macOS:

You can install CMake using [Homebrew](https://brew.sh/), a package manager for macOS. If you don't have Homebrew installed, you can install it by pasting the following command in your terminal:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Once Homebrew is installed, you can install CMake with the following command:

```bash
brew install cmake
```

Now you have CMake installed on your machine and you're ready to move on to the next step of your environment setup.

### Step 2: Check for Boost

1. Open a terminal window.
2. To check if Boost is installed on your system, type the following command:

```bash
whereis boost
```

If Boost is installed, you should see the directory paths to the Boost library. If it's not installed, follow the instructions below to install it.

#### For Ubuntu:

1. Update the package list for the latest version of the repository:

```bash
sudo apt-get update
```

2. Install the Boost library:

```bash
sudo apt-get install -y libboost-all-dev
```

#### For macOS:
Once Homebrew is installed, you can install Boost with the following command:

```bash
brew install boost
```

After the installation is complete, verify the installation of Boost by checking its directory paths again:

```bash
whereis boost
```

### Step3: Test the `matchEngine`
Now that you have set up the environment, you can use the following commands to build this project:
```bash
mkdir build
cd build
cmake ..
make test_order_run # To ensure the environment is set up successfully.
```

Once you have built the entire project, you are now ready to run the engine, provided you have the appropriate dataset at hand:

1. **Dataset Preparation**:
   Begin by executing the script below to create the necessary dataset folder structure:
   
   ```bash
   bash scripts/create_datafolder.sh
   ```

   This will result in an example dataset tree structure as follows:
   ```
   .
   ├── input_data
   │   └── 20160202
   │       ├── alpha
   │       ├── order_log
   │       └── prev_trade_info
   ├── output_data
   │   ├── pnl_and_position
   │   └── twap_order
   └── output_standard
       ├── pnl_and_position
       └── twap_order
   ```

2. **Engine Activation**:
   Now, navigate to the `build` directory and run the following command to activate `matchEngine`:
   
   ```bash
   cd build
   make engine_main_run
   ```

3. **Output Verification**:
   Lastly, utilize the `check_twap.sh` and `check_pnl.sh` scripts to verify the correctness of the engine output. Here are example commands and their expected output:
   
   ```shell
   $ bash script/check_twap.sh
   ✅ Files 20160202_3_1 are identical.
   Pass: 1/1

   $ bash script/check_pnl.sh
   ✅ Files 20160202_3_1 are identical.
   Pass: 1/1
   ```

## Introduction
...