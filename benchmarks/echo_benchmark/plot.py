import os
import matplotlib.pyplot as plt
import pandas
from benchmarks.util.utils import make_dirs


PATH = os.path.dirname(os.path.abspath(__file__))


def plot_message_size_vs_throughput(df):
    plt.bar('Message Size', 'Throughput (MB/s)', data=df)
    plt.xlabel('Message Size (B)')
    plt.ylabel('Throughput (MB/s) (log scale)')
    plt.yscale('log')
    plt.title('Message Size vs Throughput')
    plt.savefig(PATH + '/plots/message_size_vs_throughput.png')


def plot_message_size_vs_transactions(df):
    plt.bar('Message Size', 'Transactions per second', data=df)
    plt.xlabel('Message Size (B)')
    plt.ylabel('Transactions per Second')
    plt.title('Message Size vs Transactions per S')
    plt.savefig(PATH + '/plots/message_size_vs_transactions.png')


def make_plots():
    make_dirs(PATH)

    df = pandas.read_csv(PATH + '/data/results_single_thread.csv')
    df['Message Size'] = df['Message Size'].apply(str)
    df['Transactions per second'] = df['Num Transactions'].apply(lambda x: x / 30)
    plot_message_size_vs_transactions(df)
    plt.close()
    plot_message_size_vs_throughput(df)
