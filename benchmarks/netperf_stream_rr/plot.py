import pandas
import matplotlib.pyplot as plt
import os
from . import utils


PATH = os.path.dirname(os.path.abspath(__file__))


def add_overhead_column(df, baseline):
    res = pandas.DataFrame(columns=['Overhead'], index=df.index)
    res['Overhead'] = df['tput MBps'].apply(lambda x: utils.get_overhead(x, baseline))
    return res


def plot_overhead(df):
    plt.barh(df.index, 'Overhead', data=df)
    plt.ylabel('Test')
    plt.xlabel('Overhead')
    plt.tight_layout()
    plt.savefig(PATH + '/plots/overhead_vs_test_netperf.png')
    plt.close()


def make_test_vs_thruput(df):
    plt.barh('Test', 'tput MBps', data=df)
    plt.xlabel('Throughput [MB/s] (log scale)')
    plt.tight_layout()
    plt.xscale('log')
    plt.savefig(PATH + '/plots/message_size_vs_throughput_netperf.png')
    plt.close()


def make_overhead_graph(df):
    df = df.set_index('Test')
    baseline = df['tput MBps'].loc['TCP_STREAM']
    df = df.drop('TCP_STREAM')

    res = add_overhead_column(df, baseline)
    df_stats = res.describe()
    df_stats.to_csv(PATH + '/data/stats.csv')
    print(f'Average overhead introduced by TCP_RR: {res["Overhead"].mean()}')
    plot_overhead(res)


def make_plots():
    utils.make_dirs(PATH)

    df = pandas.read_csv(PATH + '/data/results.csv')
    df = df.rename(columns={'Unnamed: 0': 'Test'})
    make_test_vs_thruput(df)
    make_overhead_graph(df)
    plt.close()


if __name__ == '__main__':
    make_plots()
