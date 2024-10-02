# RVHyper example

This example shows how to create a monitor that can monitor OD
as specified for the tool RVHyper.


## Generating the monitor

Run from this directory:
```
../../mptc od.mpt inputs.cpp
```

## Compiling and running the monitor

Compiling:

```
cd /tmp/mpt
cmake .
make
```

Running:
```
./monitor 1.tr 2.tr ...
```

where `*.tr` are input files (see below).
Three such example files are attached in this directory.


## Inputs

The input encodes a sequence of values of two n-bit variables `in` and `out` (n
is at most 64). It is a file where each line is `INS;` or `;OUTS` where `INS`
is a comma-separated list of `in_b` where `b` is the number of the bit set to
true and analogously for `OUTS`.

For example `in_0,in_2;` encodes `in=5` (101 in binary).

IMPORTANT: For this simple example, we assume that OUTS are set only in the
last event (line), otherwise it is 0 all the time.

And example input
file:

```
;
;
;
;
in_1,in_2;
;
;
in_1,in_2,in_3;
;
;out_0,out_1,out_3

```

Explore the file `inputs.cpp`

## RVHyper

TBD
