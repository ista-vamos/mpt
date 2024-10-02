# VAMOS-mpt: Multi-trace prefix transducers

The implementation of the compiler for monitors for hyperproperties
based on multi-trace prefix transducers (MPT).

MPTs are described with a simple format that we show with an example

```haskell
-- file: od.mpt

-- Declare events InputL, OutputL, and Write, all of them
-- with two parameters: `addr` which is 64-bit unsigned integer
-- and `x` which is 32-bit signed integer.
Event InputL, OutputL, Write {
  addr : UInt64,
  x    : Int32
}

-- The MPT called `OD` (the name is not relevant)
mpt OD {
  -- define two input traces, `t1` and `t2` whose type is
  -- "trace of InputL, OutputL, WriteL events"
  in t1 : [InputL, OutputL, Write], t2: [InputL, OutputL, Write];

  -- define a single output trace which is not in fact a trace,
  -- but a single boolean value. So this MPT reads two input traces
  -- and gives a single verdict which is either `true` or `false`.
  out o : Bool;

  -- the initial state of the MPT
  init q0;

  -- Define a self-loop transition over state q0, the transition
  -- matches the prefix expression `_*e1@{InputL + OutputL}` on both
  -- traces and if the matched input or output event is the same on
  -- both traces, the output of this is nothing, so the transducer
  -- continues reading traces.
  q0 -> q0 {
    t1: _*e1@{InputL + OutputL};
    t2: _*e2@{InputL + OutputL};
    cond: t1[e1] == t2[e2];
  }

  -- This transition from state q0 to q1 is triggered if the matched output
  -- events or end of stream events ($) are different and in that case
  -- `false` is put to output and transducer stops.
  q0 -> q1 {
    t1: _*e1@{OutputL + $};
    t2: _*e2@{OutputL + $};
    cond: t1[e1] != t2[e2];
    out: false;
  }

  -- When this transition is taken, the transducer also stops,
  -- but yields the value `true`.
  q0 -> q2 {
    t1: _*e1@{InputL + $};
    t2: _*e2@{InputL + $};
    cond: t1[e1] != t2[e2];
    out: true;
  }
}
```

## Prefix expressions

A prefix expression (PE) is similar to regular expression, but it matches only
a prefix of a string (trace in our case) and the shortest one at that.
PEs are formed with these rules:
 - every event name is a PE (e.g., `InputL` in the example above)
 - if `A` and `B` are PEs, then
   * `{A + B}` are PEs meaning `A` or `B`
   * `{A . B}` are PEs meaning `A` and then `B`
   * `{A . B}` or `{A B}` are PEs meaning `A` and then `B`
   * `{A}` is a PE (explicit parenthesis to alter priorities)
   * `l@{A}` is a PE _labeled_ with the label _l_
 - if `A` is a PE and `B` is a PE matching _exactly one_ event, i.e., `B` is either
   an event name or a disjunction (+) of event names, then `{A*B}` is a PE
   meaning `A` until the first occurrence of `B`. More precisely, `A*B` is defined
   as `B + AB + AAB + AAAB ...` evaluated from left to right.

We use `{` and `}` to group expressions together, to avoid too many `(` and `)`
when we allow matched events to have parameters (future work).
We can avoid `{` and `}` if we keep in mind that the highest priority has `.` then
`*` and then `+`. So we can write the expression `{{{A.A}+B}*C}` as `{A.A + B}*C`.

Labels are nothing more than tags that serve to identify sequences of events matched
by sub-expressions.

### Examples

 - PE `a` is matched only by the event `a`
 - PE `a.b` is matched by a sequence of events `ab`
 - PE `a+b` is matched either by `a` or by `b`
 - PE `{a + b}.a` matches `aa` or `ba`

PEs match the shortest prefix that satisfies them, e.g.,:

 - PE `a` matches `a` from `aaab`
 - PE `aa` matches `aa` from `aaab`
 - PE `ab` does not match `aaab`

#### Bounded iteration

PEs allow bounded iteration with the operator `*`. `a*b` means read `a`s
until you see `b`. The shortest-match semantics and boundedness may be a bit
non-intuitive. 

 - PE `a*b` matches `ab` from `ababab`
 - PE `{a.b}*b` matches `ababb` from `ababb`
 - PE `{a+b}*b` matches `ab` from `ababb`
 - PE `{a+b}*b` does not match `aa`
 - PE `{a.b}*b` does not match `abab` -- recall that `a*b` is defined as `b + abb + ababb + abababb + ...`, matching the shortest one


## Multi-trace PEs

A _Multi-trace prefix expression (MPE)_ is a list of PEs associated to traces with an
additional _condition_:

```
t1: _*e1@{InputL + $};
t2: _*e2@{InputL + $};
cond: t1[e1] != t2[e2];
```

Each of the PEs is matched independently of each other on the traces and so the matches
can have different length. When all PEs in an MPE match, the condition is checked
and if it is satisfied, the transition is taken.

### MPE conditions

At the time being, we use only simple equality conditions in the form `l1 = l2`, `l1 != l2`,
`t1[l1] = t1[l2]`, and `t1[l1] != t2[l2]` where `l1`, `l2` are _labels_ from PEs
and `t1`, `t2` are traces:

 - `l1 = l2` (`l1 != l2`) is true if the positions of events matched by sub-expressions
   labeled with `l1` and `l2` are (not) the same.
 - `t1[l1] = t2[l2]` (`t1[l1] != t2[l2]`) is true if the events on traces `t1` and `t2`
   matched by sub-expressions labeled with `l1` and `l2` are (not) the same.

Note that a label may match multiple positions, e.g., if we evaluate `l@{a}*b` on the trace
`aaab`, then `l` will match all a's, i.e., `l` will be associated with positions 0, 1, and 2.
More precisely, since a sub-expression may match more than a single event, `l` will be
associated with _ranges_ of positions (0, 0), (1, 1), and (2, 2).
In such cases, the positions are concatenated and labels (and events on traces) are compared
accordingly. For example, assume we have these two PEs in an MPE:

```
t1: l1@{a}*b;
t2: l2@{a}*b;
```

when evaluated on traces `t1 = aaabaa` and `t2 = aab`, `aaab` is matched on `t1`, `aab` on `t2`,
and  the string of position ranges `(0, 0)(1, 1)(2, 2)` is associated to `l1`, and `(0, 0)(1, 1)`
to `l2`. These conditions would be evaluated in the following way:

 - `l1 = l2` => `(0, 0)(1, 1)(2, 2) = (0, 0)(1, 1)` => `false`
 - `t1[l1] = t2[l2]` => `t1[(0, 0)].t1[(1, 1)].t1[(2, 2)]] = t2[(0, 0)].t2[(1, 1)]]` =>
   `aaa = aa` => `false`

# Using the compiler

## Setup

First, you need to setup [vamos-common](https://github.com/ista-vamos/vamos-common).
The easiest way to do it is to clone and setup whole [VAMOS](https://github.com/ista-vamos/vamos)
(follow the instructions in VAMOS' README).

To setup the project, run:
```
cmake . -Dvamos-common_DIR=<path-to-vamos-common>/cmake/vamos-common
```
or, alternatively, if you built VAMOS with no changes to the structure of submodules,
you can run:

```
cmake . -Dvamos_DIR=<path-to-vamos-top-dir>
```


## Generating a monitor

The main script is `mptc` (*MPT* *C*ompiler) that takes as input the mpt definition
(file with the extension `.mpt`) and some other options. All options can be shown
using the `--help` argument. The basic usage is:

```shell
$ mptc od.mpt
```

This command will generate a C++ project with CMake configuration in `/tmp/mpt`
(this directory can be changed with the `--out-dir` option).
The C++ project then can be configured and compiled:

```shell
cd /tmp/mpt
cmake .
make
```

If everything goes well (it will probably fail because we haven't specified the
input sources for traces, but let's assume we have), the binary `monitor` is generated
and can be directly executed. This binary reads input traces (see below) and
monitors them with the specified MPT.

## Defining inputs

TBD

