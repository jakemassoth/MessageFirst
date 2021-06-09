import echo_benchmark.plot
import netperf_stream_rr.plot
from benchmarks.comparison import plot_comparison_graphs

if __name__ == '__main__':
    print('Saving netperf benchmark plots in benchmarks/netperf_stream_rr/plots')
    netperf_stream_rr.plot.make_plots()

    print('Saving MessageFirst plots in benchmarks/echo_benchmark/plots')
    echo_benchmark.plot.make_plots()

    print('Saving comparison plots in benchmarks/comparison/plots')
    plot_comparison_graphs.plot_comparison_graphs()
