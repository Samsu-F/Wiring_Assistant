# Wiring Assistant
This is a project for the seminar [Selected Fun Problems of the ACM Programming Contest](https://db.cs.uni-tuebingen.de/teaching/ss24/acm-programming-contest-proseminar/) at the University of Tübingen in the summer semester of 2024.

Included are my presentation slides, the seminar paper I wrote, and the implementation of the algorithm I developed.

## The Code
### Installation
A Makefile is included so you only need to run `make` in the `code` directory. The executable will be at `code/build/wiring_assistant`.

### Usage
`wiring_assistant [OPTIONS]`

Options:<br>
  `-h`    (help)  Show help message and exit.<br>
  `-g`    (graph) Print the graph after the reduction step.<br>
  `-p`    (path)  Mark the cheapest path in the printed graph. Implies -g.<br>
  `-t`    (time)  Measure and print the time to run each step.<br>

Input is read from stdin.

For example, when in the `code` directory, run
```shell
build/wiring_assistant -pt <../testdata/example_input.txt
```
to see which path my algorithm takes for the example scenario used in the paper, and how long it takes to solve.
