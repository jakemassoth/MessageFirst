import pandas
import matplotlib.pyplot as plt
import os

PATH = os.path.dirname(os.path.abspath(__file__))


def make_test_vs_thruput(df):
    plt.barh('Test', 'tput MBps', data=df)
    plt.ylabel('Test')
    plt.xlabel('Throughput (MB/s)')
    plt.title('Throughput in netperf Tests')
    plt.tight_layout()
    plt.xscale('log')
    plt.savefig(PATH + '/data/message_size_vs_throughput.png')


def make_plots(df):
    df = df.rename(columns={'Unnamed: 0': 'Test'})
    make_test_vs_thruput(df)
    plt.close()


def run():
    df = pandas.read_csv(PATH + '/data/results.csv')
    print(df)
    print(df.columns)
    make_plots(df)


if __name__ == '__main__':
    run()
