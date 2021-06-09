import echo_benchmark.benchmark_echo
import netperf_stream_rr.benchmark_netperf
import sys


if __name__ == '__main__':
    on_das = (len(sys.argv) == 3)
    if on_das:
        netperf_stream_rr.benchmark_netperf.run_benchmark(on_das=True, node1=sys.argv[1], node2=sys.argv[2])
        echo_benchmark.benchmark_echo.run_benchmark(on_das=True, node1=sys.argv[1], node2=sys.argv[2])
    else:
        netperf_stream_rr.benchmark_netperf.run_benchmark()
        echo_benchmark.benchmark_echo.run_benchmark()
