# libemotify-stderr

A simple library that will spice up your **stderr** output.

![DEMO GIF](demo.gif)

**libemotify-stderr** replaces **stderr** file descriptor with a pipe and processes program output before redirecting it back to original **stderr**.
All this to add emojis at the start of error lines.
Or to make them colorful.

## Building
Library is built using **cmake** and **make**.
In repository directory:
```
mkdir build
cd build
cmake ..
make
```

## How to use
Add the built **.so** to your `LD_PRELOAD`:
```
export LD_PRELOAD=/path/to/libemotify-stderr.so
```
or
```
LD_PRELOAD=/path/to/libemotify-stderr.so command
```

## Envirnoment variable settings
* `EMOJIFY_COLORS=1` will enable coloring of the **stderr** output
* `EMOJIFY_REACTS=0` will disable emojis

## Known issues
* Sometimes the emojis will get outputed several times per line 😔
* git uses stderr for output and doesn't like this lib 😠
