# MessageFirst: a reliable user API for message-oriented network workloads
This repository contains code for my thesis project where I designed and implemented MessageFirst, which is a user API
for message-oriented network workloads. It is a blocking API that supports the notion of a "message", a unit of transmission
that is not considered done until a message has been sent and a respective response has been received.
## Downloading, building and running tests
Before running any benchmarks, first build MessageFirst and run the tests to make sure your environment is set up correctly.
### Downloading MessageFirst
Clone the repo as so:

```shell
git clone https://github.com/jakemassoth/MessageFirst.git
cd MessageFirst
```

### Python
We use [Miniconda](https://docs.conda.io/en/latest/miniconda.html) to create isolated python environments with the
necessary packages and versions needed to run the MessageFirst tests and benchmarks.

Download the Miniconda installer for Linux:

```shell
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
chmod +x ./Miniconda3-latest-Linux-x86_64.sh
./Miniconda3-latest-Linux-x86_64.sh
```

The installer will ask where Miniconda should be installed. If you are running the benchmarks locally, feel free to use
the default. **If you are running the benchmarks on the DAS-5, make sure to install Miniconda in `/var/scratch/<USERNAME>/miniconda3`.**

Answer "yes" when the installer asks if it should run `conda init`.

Now that Miniconda is installed, we can create the MessageFirst Python environment.

```shell
source ~/.bashrc
conda create --name MessageFirst --file spec-file.txt
echo "conda activate MessageFirst" >> ~/.bashrc
source ~/.bashrc
rm Miniconda3-latest-Linux-x86_64.sh
```

You should now see `(MessageFirst)` prepended to your terminal prompt.

### Building MessageFirst
Requires CMake 3.15.

```shell
cmake .
make
make test
```

If these all run successfully, you're good to go!

## Running Examples
TODO

## Running Benchmarks
This repository contains two benchmarks:
- `netperf_stream_rr` compares the throughput achieved by netperf's `TCP_RR` test to a baseline `TCP_STREAM` test
- `echo_benchmark` benchmarks the MessageFirst API implementation.

### Installing netperf
First, install [netperf](https://linux.die.net/man/1/netperf) for your distribution. Netperf is a network benchmarking 
tool used in the `netperf_stream_rr` benchmark. On Ubuntu 18.04:

```shell
sudo apt update
sudo apt install netperf
```

Verify that netperf is working with a quick test benchmark:

```shell
netserver
netperf
```

This benchmark should complete successfully after about 10 seconds and display some statistics.

### Running locally
Running benchmarks locally is pretty simple:

```shell
python benchmarks/run_benchmarks.py
```

This will take a few minutes to run. It will generate .csv files containing the data from the benchmarks in the relevant
`data` folders for each benchmark.

To plot graphs:

```shell
python benchmarks/make_plots.py
```

This will generate .png plots for each benchmark in its relevant `plots` folder.

### Running on DAS-5
First, reserve two nodes as so:

```shell
preserve -np 3 -t 900
```

Use `preserve -llist` to list the reservations currently active. Note the node numbers you have been assigned. 

Then run the benchmark as so:

```shell
python benchmarks/run_benchmarks.py node<X> node<Y>
```

Replace <X> and <Y> with the node numbers of your reservation.

After this completes (it should take a few minutes), make the plots:

```shell
python benchmarks/make_plots.py
```

This will generate .png plots for each benchmark in its relevant `plots` folder.