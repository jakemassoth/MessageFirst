import os
import matplotlib.pyplot as plt
import pandas
from . import utils

PATH = os.path.dirname(os.path.abspath(__file__))


def plot_message_size_vs_throughput(df):
    plt.bar('Message Size', 'Throughput (MB/s)', data=df)
    plt.xlabel('Message Size [B]')
    plt.ylabel('Throughput [MB/s] (log scale)')
    plt.yscale('log')
    plt.savefig(PATH + '/plots/message_size_vs_throughput_messagefirst.png')


def plot_message_size_vs_transactions(df):
    plt.bar('Message Size', 'Transactions per second', data=df)
    plt.xlabel('Message Size [B]')
    plt.ylabel('Transactions per Second')
    plt.savefig(PATH + '/plots/message_size_vs_transactions_messagefirst.png')


def make_plots():
    utils.make_dirs(PATH)

    df = pandas.read_csv(PATH + '/data/results_single_thread.csv')
    df['Message Size'] = df['Message Size'].apply(str)
    df['Transactions per second'] = df['Num Transactions'].apply(lambda x: x / 30)
    plot_message_size_vs_transactions(df)
    plt.close()
    plot_message_size_vs_throughput(df)
