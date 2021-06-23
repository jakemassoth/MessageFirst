import os


def fix_csv_parse(netperf_df):
    netperf_df = netperf_df.drop([0]).reset_index()
    netperf_df = netperf_df.drop(['index'], axis=1)
    netperf_df = netperf_df.rename(columns={'Unnamed: 0': 'Test'})
    netperf_df = netperf_df.rename(columns={'tput MBps': 'Netperf Throughput (MB/s)'})
    return netperf_df


def get_overhead(experiment, baseline):
    return abs((experiment - baseline) / baseline)


def make_dirs(PATH):
    if not os.path.exists(PATH + '/data'):
        os.mkdir(PATH + '/data')
    if not os.path.exists(PATH + '/plots'):
        os.mkdir(PATH + '/plots')