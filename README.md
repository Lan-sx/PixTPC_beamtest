# PixTPC_beamtest 
Offline process program to analysis PixelTPC beam test data

---
Expbr


## How to install ?
Make sure you have installed:
+  [ROOT](https://root.cern.ch)
+  [Garfield++](https://garfieldpp.web.cern.ch/garfieldpp), for cepc PixTPC simulation
+  [nlohmann](https://github.com/nlohmann/json), json parser, task config
+  [spdlog](https://github.com/gabime/spdlog), logger
+  [cmake](https://cmake.org)
Then,
    ```shell
    mkdir build
    cd build
    cmake ..
    make -j<N>
    make install
    cd ..
    source ./setup.sh
    ```
if there is only `.so` files in `./install/lib` dir after installation, you can modify `CMakeLists.txt` file (just add a new line, then cancel modification), and rebuild.
The `.rootmap` and `.pcm` files will be installed in `./install/lib` dir.

## How to Run?

    ```
    $ cepcPixTPC [-J task.json] [-L info]
    ```
PS: this program is under developing...
