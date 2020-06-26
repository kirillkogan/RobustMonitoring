cmake_minimum_required(VERSION 2.8)
project(simulator)

set(CMAKE_CXX_STANDARD 11)

add_executable(simulator
		coresim/LDgammaTracker.cpp
	    coresim/GammaTracker.cpp
		coresim/TelemetryTracker.cpp
        coresim/debug.cpp
        coresim/debug.h
        coresim/event.cpp
        coresim/event.h
        coresim/flow.cpp
        coresim/flow.h
        coresim/main.cpp
        coresim/node.cpp
        coresim/node.h
        coresim/packet.cpp
        coresim/packet.h
        coresim/queue.cpp
        coresim/queue.h
        coresim/random_variable.cpp
        coresim/random_variable.h
        coresim/topology.cpp
        coresim/topology.h
		run/experiment.cpp
        run/flow_generator.cpp
        run/flow_generator.h
        run/params.cpp
        run/params.h
        run/stats.cpp
        run/stats.h)
