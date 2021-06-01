import pandas
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os

PATH = os.path.dirname(os.path.abspath(__file__))


def plot_comparison_graphs():
    message_first_1_thread_df = pd.read_csv(PATH + '/echo_benchmark/data/results_single_thread.csv')
    netperf_df = pd.read_csv(PATH + '/netperf_stream_rr/data/results.csv')

    message_first_1_thread_df = message_first_1_thread_df.rename(
        columns={'Throughput (MB/s)': 'MessageFirst Throughput (MB/s)'})

    print(message_first_1_thread_df.columns)
    print(message_first_1_thread_df)

    netperf_df = netperf_df.drop([0]).reset_index()
    netperf_df = netperf_df.drop(['index'], axis=1)
    netperf_df = netperf_df.rename(columns={'Unnamed: 0': 'Test'})
    netperf_df = netperf_df.rename(columns={'tput MBps': 'Netperf Throughput (MB/s)'})

    print(netperf_df.columns)
    print(netperf_df)

    combined_df = pandas.concat([message_first_1_thread_df['Message Size'],
                                 message_first_1_thread_df['MessageFirst Throughput (MB/s)'],
                                 netperf_df['Netperf Throughput (MB/s)']
                                 ], axis=1)

    fix, ax = plt.subplots()
    index = np.arange(9)
    bar_width = 0.35
    netperf = ax.bar(index, combined_df['Netperf Throughput (MB/s)'], bar_width, label='Netperf')

    messagefirst = ax.bar(index+bar_width, combined_df['MessageFirst Throughput (MB/s)'], bar_width,
                          label='MessageFirst')

    ax.set_xlabel('Message Size (B)')
    ax.set_ylabel('Throughput (MB/s)')
    ax.set_yscale('log')
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(combined_df['Message Size'])
    ax.legend()

    plt.savefig(PATH + '/plots/MessageFirst vs Netperf.png')


if __name__ == '__main__':
    plot_comparison_graphs()
