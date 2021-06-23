import matplotlib.pyplot as plt
import pandas as pd
import os
from . import utils


PATH = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def get_overhead_column(message_first_1_thread_df, netperf_df):
    overhead_thruput_df = pd.concat([netperf_df['Netperf Throughput (MB/s)'].rename('netperf'),
                                         message_first_1_thread_df['MessageFirst Throughput (MB/s)'].rename(
                                             'MessageFirst')], axis=1)

    return overhead_thruput_df.apply(lambda row: utils.get_overhead(row['MessageFirst'], row['netperf']), axis=1)


def plot_overhead(df):
    plt.bar('Message Size', 'Overhead (Thruput)', data=df)
    plt.xlabel('Message size (B)')
    plt.ylabel('Overhead')

    plt.savefig(PATH + '/plots/overhead_netperf_messagefirst.png')
    plt.close()


def make_overhead_plot():
    utils.make_dirs(PATH)

    message_first_1_thread_df = pd.read_csv(DATA_PATH + '/echo_benchmark/data/results_single_thread.csv')
    netperf_df = pd.read_csv(DATA_PATH + '/netperf_stream_rr/data/results.csv')

    message_first_1_thread_df = message_first_1_thread_df.rename(
        columns={'Throughput (MB/s)': 'MessageFirst Throughput (MB/s)'})

    netperf_df = utils.fix_csv_parse(netperf_df)

    message_first_1_thread_df['Overhead (Thruput)'] = get_overhead_column(message_first_1_thread_df, netperf_df)

    message_first_1_thread_df['Overhead (Thruput)'].describe().to_csv(PATH + '/data/overhead_thruput_stats.csv')
    print(f'Average overhead introduced by MessageFirst: {message_first_1_thread_df["Overhead (Thruput)"].mean()}')

    plot_overhead(message_first_1_thread_df)




