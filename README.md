# JACK

The JACK C++ agent framework

## ABOUT

JACK is a framework for building distributed reasoning systems that are based upon the symbolic AI technology of BDI (Beliefs, Desires and Intentions) intelligent software agents. JACK is a set of software libraries and tools written in C++ that help you build teamed intelligent software agent applications. From high-level BDI reasoning plans to ROS 2 tools, and with powerful developer tools, JACK has what you need for exploring the potential of intelligent agents and building commercial and operational AI systems. And it's all open source. JACK builds on the legacy of the original JACK written in Java, now known as “JACK Classic”.

## GETTING STARTED
Looking to get started with JACK?

Our build guide is here. Once you've built and installed JACK, start by learning some
basic concepts and taking a look at our beginner tutorials. Intelligent agents require
a different way of thinking; they are not just another procedural programming
approach, so take your time to understand the concepts before leaping into an
application.

### Build using colcon

``` bash
colcon build --cmake-args -DCMAKE_BUILD_TYPE:STRING=Debug
```

## Running 'Game of life' Example

``` bash
source install/setup.bash
./install/gol/bin/golapp
```

## JOIN THE JACK COMMUNITY

Coming soon...