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
    plt.title('MessageFirst Overhead Relative to Netperf (Transactions)')
    plt.savefig(PATH + '/plots/overhead_netperf_messagefirst.png')
    plt.close()


def calc_num_transactions(row):
    test = row['Test']
    test = test.strip('TCP_RR ')
    test = test.strip('B')
    num_bytes = int(test)

    thruput = row['Netperf Throughput (MB/s)'] * (1024 * 1024)

    return (thruput / (num_bytes * 2)) * 30


def get_overhead(experiment, baseline):
    return abs((experiment - baseline) / baseline)


def calc_overhead(row):
    return get_overhead(row['MessageFirst'], row['netperf'])


def plot_comparison_graphs():
    make_dirs(PATH)

    message_first_1_thread_df = pd.read_csv(DATA_PATH + '/echo_benchmark/data/results_single_thread.csv')
    netperf_df = pd.read_csv(DATA_PATH + '/netperf_stream_rr/data/results.csv')

    message_first_1_thread_df = message_first_1_thread_df.rename(
        columns={'Throughput (MB/s)': 'MessageFirst Throughput (MB/s)'})

    print(message_first_1_thread_df.columns)
    print(message_first_1_thread_df)

    netperf_df = netperf_df.drop([0]).reset_index()
    netperf_df = netperf_df.drop(['index'], axis=1)
    netperf_df = netperf_df.rename(columns={'Unnamed: 0': 'Test'})
    netperf_df = netperf_df.rename(columns={'tput MBps': 'Netperf Throughput (MB/s)'})
    netperf_df['Num Transactions'] = netperf_df.apply(calc_num_transactions, axis=1)

    print(netperf_df.columns)
    print(netperf_df)

    fix, ax = plt.subplots()
    index = np.arange(9)
    bar_width = 0.35
    netperf = ax.bar(index, netperf_df['Netperf Throughput (MB/s)'], bar_width, label='Netperf')

    messagefirst = ax.bar(index + bar_width, message_first_1_thread_df['MessageFirst Throughput (MB/s)'], bar_width,
                          label='MessageFirst')

    ax.set_xlabel('Message Size (B)')
    ax.set_ylabel('Throughput (MB/s) (log scale)')
    ax.set_yscale('log')
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(message_first_1_thread_df['Message Size'])
    ax.legend()

    plt.savefig(PATH + '/plots/MessageFirst vs Netperf.png')
    plt.close()

    fix, ax = plt.subplots()
    netperf = ax.bar(index, message_first_1_thread_df['Num Transactions'], bar_width, label='Netperf')

    messagefirst = ax.bar(index + bar_width, netperf_df['Num Transactions'], bar_width,
                          label='MessageFirst')

    ax.set_xlabel('Message Size (B)')
    ax.set_ylabel('Number of Transactions in 30s')
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(message_first_1_thread_df['Message Size'])
    ax.legend()

    plt.savefig(PATH + '/plots/MessageFirst vs Netperf transactions.png')
    plt.close()

    overhead_transaction_df = pandas.concat([netperf_df['Num Transactions'].rename('netperf'),
                                             message_first_1_thread_df['Num Transactions'].rename('MessageFirst')],
                                            axis=1)

    overhead_thruput_df = pandas.concat([netperf_df['Netperf Throughput (MB/s)'].rename('netperf'),
                                         message_first_1_thread_df['MessageFirst Throughput (MB/s)'].rename(
                                             'MessageFirst')], axis=1)

    message_first_1_thread_df['Overhead (Transactions)'] = overhead_transaction_df.apply(calc_overhead, axis=1)
    message_first_1_thread_df['Overhead (Thruput)'] = overhead_thruput_df.apply(calc_overhead, axis=1)

    message_first_1_thread_df['Overhead (Transactions)'].describe().to_csv(
        PATH + '/data/overhead_transaction_stats.csv')
    message_first_1_thread_df['Overhead (Thruput)'].describe().to_csv(PATH + '/data/overhead_thruput_stats.csv')

    print('Average transaction time (netperf) microseconds')
    transactions_per_second_netperf = netperf_df['Num Transactions'].mean()
    print((1 / transactions_per_second_netperf) * (10 ** 6))

    print('Average transaction time (messagefirst) microseconds')
    transactions_per_second_messagefirst = message_first_1_thread_df['Num Transactions'].mean()
    print((1 / transactions_per_second_messagefirst) * (10 ** 6))

    plot_overhead(message_first_1_thread_df)


if __name__ == '__main__':
    plot_comparison_graphs()
