import pandas
import matplotlib.pyplot as plt
import os

PATH = os.path.dirname(os.path.abspath(__file__))


def plot_overhead(df):
    plt.barh(df.index, 'Overhead', data=df)
    plt.ylabel('Test')
    plt.xlabel('Overhead')
    plt.title('Request-Response Test Overhead')
    plt.tight_layout()
    plt.savefig(PATH + '/data/overhead_vs_test.png')
    plt.close()


def get_overhead(experiment, baseline):
    return abs((experiment - baseline) / baseline)


def calc_overhead(df, baseline):
    res = pandas.DataFrame(columns=['Overhead'], index=df.index)
    res['Overhead'] = df['tput MBps'].apply(lambda x: get_overhead(x, baseline))
    return res


def run():
    df = pandas.read_csv(PATH + '/data/results.csv')
    df = df.rename(columns={'Unnamed: 0': 'Test'})
    df = df.set_index('Test')
    baseline = df['tput MBps'].loc['TCP_STREAM']
    df = df.drop('TCP_STREAM')
    print(df)
    print(baseline)

    res = calc_overhead(df, baseline)
    print(res)
    df_stats = res.describe()
    print(df_stats)
    df_stats.to_csv(PATH + '/data/stats.csv')
    plot_overhead(res)


if __name__ == '__main__':
    run()
