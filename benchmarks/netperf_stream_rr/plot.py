import pandas
import matplotlib.pyplot as plt
import os
from benchmarks.util.utils import make_dirs
from benchmarks.util.calc_overhead import add_overhead_column

PATH = os.path.dirname(os.path.abspath(__file__))


def plot_overhead(df):
    plt.barh(df.index, 'Overhead', data=df)
    plt.ylabel('Test')
    plt.xlabel('Overhead')
    plt.title('Request-Response Test Overhead')
    plt.tight_layout()
    plt.savefig(PATH + '/data/overhead_vs_test.png')
    plt.close()


def make_test_vs_thruput(df):
    plt.barh('Test', 'tput MBps', data=df)
    plt.ylabel('Test')
    plt.xlabel('Throughput (MB/s) (log scale)')
    plt.title('Throughput in netperf Tests')
    plt.tight_layout()
    plt.xscale('log')
    plt.savefig(PATH + '/plots/message_size_vs_throughput.png')


def make_overhead_graph(df):
    df = df.set_index('Test')
    baseline = df['tput MBps'].loc['TCP_STREAM']
    df = df.drop('TCP_STREAM')
    print(df)
    print(baseline)

    res = add_overhead_column(df, baseline)
    print(res)
    df_stats = res.describe()
    print(df_stats)
    df_stats.to_csv(PATH + '/data/stats.csv')
    plot_overhead(res)


def make_plots():
    make_dirs(PATH)

    df = pandas.read_csv(PATH + '/data/results.csv')
    df = df.rename(columns={'Unnamed: 0': 'Test'})
    make_test_vs_thruput(df)
    make_overhead_graph(df)
    plt.close()


if __name__ == '__main__':
    make_plots()
