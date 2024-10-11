# PixTPC_beamtest Raw2ROOT_br
Offline process program to analysis PixelTPC beam test data
Temp branch for Raw data to ROOT

## How to install ?
Make sure you have installed:
+  [ROOT](https://root.cern.ch)
+  [Garfield++](https://garfieldpp.web.cern.ch/garfieldpp), for cepc PixTPC simulation
+  [nlohmann](https://github.com/nlohmann/json), json parser, task config
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
## How to Run?

    ```shell
    cepcPixTPC task.json
    ```
PS: this program is under developing...
