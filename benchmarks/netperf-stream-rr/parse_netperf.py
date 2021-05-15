import subprocess
import pandas as pd


def netperf_benchmark(host, test, netperf_global_args, netperf_test_args=None):
    cmd = 'netperf'
    global_args = netperf_global_args.split(' ')
    args = [cmd, '-H', host, '-t', test]
    args.extend(global_args)
    if netperf_test_args:
        args.append('--')
        test_args = netperf_test_args.split(' ')
        args.extend(test_args)

    prun = subprocess.Popen(args, stdout=subprocess.PIPE, text=True)
    output = str(prun.communicate())

    output = output.split('\n')
    output = output[0].split('\\n')

    for i in range(1, len(output) - 1):
        print(output[i])

    # weird value gets added sometimes
    output.remove("', None)")

    if test == 'TCP_STREAM':
        res = output[-1]
    elif test == 'TCP_RR':
        res = output[-2]
    else:
        raise ValueError('test must be either TCP_RR or TCP_STREAM')

    return res


def get_data_tcp_stream(line):
    data = [{'Recv Socket Size B': line[0],
             'Send Socket Size B': line[1],
             'Message Size B': line[2],
             'Elapsed time': line[3],
             'tput KBps': line[4],
             'Local CPU Util': line[5],
             'Remote CPU Util': line[6],
             'Local sd us/KB': line[7],
             'Remote sd us/KB': line[8]}]
    return data


def get_data_tcp_rr(line):
    data = [{'Recv Socket Size B': line[0],
             'Send Socket Size B': line[1],
             'Message Size B': line[2],
             'Elapsed time': line[4],
             'tput KBps': line[5],
             'Local CPU Util': line[6],
             'Remote CPU Util': line[7],
             'Local sd us/KB': line[8],
             'Remote sd us/KB': line[9]}]
    return data


def parse_to_df(line, test_name):
    line = line.split()
    print(line)
    if 'TCP_STREAM' in test_name:
        data = get_data_tcp_stream(line)
    elif 'TCP_RR' in test_name:
        data = get_data_tcp_rr(line)
    else:
        raise ValueError('Test must be either TCP_STREAM or TCP_RR')

    df = pd.DataFrame.from_records(data, index=[test_name])

    return df


if __name__ == '__main__':
    global_args = '-l 30 -c -C -f K'
    host = '127.0.0.1'
    message_sizes = [64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384]

    rr_experiements = []
    for size in message_sizes:
        rr_experiements.append(['TCP_RR {}B'.format(size), '-r {},{}'.format(size, size)])

    print(rr_experiements)

    out = netperf_benchmark(host, 'TCP_STREAM', global_args)
    master_df = parse_to_df(out, 'TCP_STREAM')

    for elem in rr_experiements:
        out = netperf_benchmark(host, 'TCP_RR', global_args, elem[1])
        temp_df = parse_to_df(out, elem[0])
        master_df = master_df.append(temp_df, verify_integrity=True)

    print(master_df)

    master_df.to_csv('results.csv')
