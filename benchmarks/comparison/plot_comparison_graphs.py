import pandas
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os
from benchmarks.util.utils import make_dirs

PATH = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def plot_overhead(df):
    plt.bar('Message Size', 'Overhead (Thruput)', data=df)
    plt.xlabel('Message size (B)')
    plt.ylabel('Overhead')
    plt.savefig(PATH + '/plots/overhead_netperf_messagefirst.png')
    plt.close()


def plot_transactions(message_first_1_thread_df, netperf_df):
    fix, ax = plt.subplots()
    index = np.arange(9)
    bar_width = 0.35
    netperf = ax.bar(index, netperf_df['Transactions per second'], bar_width, label='Netperf')

    messagefirst = ax.bar(index + bar_width, message_first_1_thread_df['Transactions per second'], bar_width,
                          label='MessageFirst')

    ax.set_xlabel('Message Size [B]')
    ax.set_ylabel('Transactions per Second')
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(message_first_1_thread_df['Message Size'])
    ax.legend()

    plt.savefig(PATH + '/plots/MessageFirst vs Netperf transactions.png')
    plt.close()


def plot_thruput(message_first_1_thread_df, netperf_df):
    fix, ax = plt.subplots()
    index = np.arange(9)
    bar_width = 0.35
    netperf = ax.bar(index, netperf_df['Netperf Throughput (MB/s)'], bar_width, label='Netperf')

    messagefirst = ax.bar(index + bar_width, message_first_1_thread_df['MessageFirst Throughput (MB/s)'], bar_width,
                          label='MessageFirst')

    ax.set_xlabel('Message Size [B]')
    ax.set_ylabel('Throughput [MB/s] (log scale)')
    ax.set_yscale('log')
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(message_first_1_thread_df['Message Size'])
    ax.legend()

    plt.savefig(PATH + '/plots/MessageFirst vs Netperf.png')
    plt.close()


def add_overhead_column(message_first_1_thread_df, netperf_df):
    overhead_thruput_df = pandas.concat([netperf_df['Netperf Throughput (MB/s)'].rename('netperf'),
                                         message_first_1_thread_df['MessageFirst Throughput (MB/s)'].rename(
                                             'MessageFirst')], axis=1)

    return overhead_thruput_df.apply(calc_overhead, axis=1)



def calc_num_transactions(row):
    test = row['Test']
    test = test.strip('TCP_RR ')
    test = test.strip('B')
    num_bytes = int(test)

    thruput = row['Netperf Throughput (MB/s)'] * (1024 * 1024)

    return thruput / (num_bytes * 2)


def get_overhead(experiment, baseline):
    return abs((experiment - baseline) / baseline)


def calc_overhead(row):
    return get_overhead(row['MessageFirst'], row['netperf'])


def fix_csv_parse(netperf_df):
    netperf_df = netperf_df.drop([0]).reset_index()
    netperf_df = netperf_df.drop(['index'], axis=1)
    netperf_df = netperf_df.rename(columns={'Unnamed: 0': 'Test'})
    netperf_df = netperf_df.rename(columns={'tput MBps': 'Netperf Throughput (MB/s)'})
    return netperf_df


def plot_comparison_graphs():
    make_dirs(PATH)

    message_first_1_thread_df = pd.read_csv(DATA_PATH + '/echo_benchmark/data/results_single_thread.csv')
    netperf_df = pd.read_csv(DATA_PATH + '/netperf_stream_rr/data/results.csv')

    message_first_1_thread_df = message_first_1_thread_df.rename(
        columns={'Throughput (MB/s)': 'MessageFirst Throughput (MB/s)'})

    netperf_df = fix_csv_parse(netperf_df)

    # Netperf comes as thruput, so we need to do some more complicated math for that.
    netperf_df['Transactions per second'] = netperf_df.apply(calc_num_transactions, axis=1)
    # MessageFirst comes as total transactions, so we need to get how many there were in 30 seconds.
    message_first_1_thread_df['Transactions per second'] = message_first_1_thread_df['Num Transactions'].apply(lambda x: x / 30)

    plot_thruput(message_first_1_thread_df, netperf_df)

    message_first_1_thread_df['Overhead (Thruput)'] = add_overhead_column(message_first_1_thread_df, netperf_df)
    message_first_1_thread_df['Overhead (Thruput)'].describe().to_csv(PATH + '/data/overhead_thruput_stats.csv')
    plot_overhead(message_first_1_thread_df)

    print('Average transaction time (netperf) microseconds')
    transactions_per_second_netperf = netperf_df['Transactions per second'].mean()
    print((1 / transactions_per_second_netperf) * (10 ** 6))

    print('Average transaction time (messagefirst) microseconds')
    transactions_per_second_messagefirst = message_first_1_thread_df['Transactions per second'].mean()
    print((1 / transactions_per_second_messagefirst) * (10 ** 6))


if __name__ == '__main__':
    plot_comparison_graphs()
