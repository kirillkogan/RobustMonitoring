# Robust Distributed Monitoring of Traffic Flows
This repository contains the code to reproduce the experimental evaluation in the paper "Robust Distributed Monitoring of Traffic Flows."

Evaluations are based on the YAPS simulator(https://github.com/NetSys/simulator).

## Compile: 

`CXX=g++ cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-std=c++14"` then `make`. 

## Run:

`./simulator 1 conf_file.txt > output.ans`, where `conf_file.txt` is a configuration file 

Simulator will print to file `output.ans` the values of all calculated metrics (see `output_standart.txt` for example). 


## Parameters variation

* `speed` - correspond to beta, in evaluations we varied `speed` from 10 to 20 to construct Fig. 8b and Fig. 8f.
* `queue_size` - buffers size in bytes (the number of packets multiplied by 1500).
* `flowlet_size` - value of FL.

`conf_standart.txt` -- configuration file for the standard experiment.
`output_standart.txt` -- values of all calculated metrics in the standart experiment.
