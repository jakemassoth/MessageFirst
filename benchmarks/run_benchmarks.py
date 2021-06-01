import echo_benchmark.run_benchmark_echo
import netperf_stream_rr.run_benchmark_netperf
import plot_comparison_graphs
import sys


if __name__ == '__main__':
    on_das = (len(sys.argv) == 3)
    if on_das:
        netperf_stream_rr.run_benchmark_netperf.run(on_das=True, node1=sys.argv[1], node2=sys.argv[2])
        echo_benchmark.run_benchmark_echo.run(on_das=True, node1=sys.argv[1], node2=sys.argv[2])
    else:
        netperf_stream_rr.run_benchmark_netperf.run()
        echo_benchmark.run_benchmark_echo.run()
    plot_comparison_graphs.plot_comparison_graphs()
