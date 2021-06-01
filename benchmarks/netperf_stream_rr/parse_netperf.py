import subprocess
import pandas as pd


def get_numa_domain_network_card(node):
    proc = subprocess.Popen(f'ssh {node} "cat /sys/class/net/ib0/device/numa_node"', shell=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    if stdout is None:
        return 0

    res = str(stdout)
    res = res.split('\n')
    res = res[0].split('\\n')
    res = res[0].strip("b'")
    print('numa node: ' + res)

    return int(res)


def __start_netserver_local():
    server = subprocess.Popen(['netserver'])
    return server


def start_netserver(node=None):
    if node is None:
        return __start_netserver_local()
    else:
        numa = get_numa_domain_network_card(node)
        cmd = f'ssh {node} "numactl -N {numa} -m {numa} netserver"'
        print('running ' + cmd)
        server = subprocess.Popen(cmd, shell=True)
        return server


def netperf_benchmark(test, netperf_global_args, netperf_test_args=None, run_on_das=False, node1=None, node2=None):
    cmd = 'netperf'
    global_args = netperf_global_args.split(' ')
    if run_on_das:
        ip_num = node1.strip('node')
        ip_num = ip_num.lstrip('0')
        host = f'10.149.0.{ip_num}'
        args = [cmd, '-H', host, '-t', test]
    else:
        args = [cmd, '-t', test]

    args.extend(global_args)

    if netperf_test_args:
        args.append('--')
        test_args = netperf_test_args.split(' ')
        args.extend(test_args)

    if run_on_das:
        numa = get_numa_domain_network_card(node2)
        cmd_str = ' '.join(args)
        cmd_run = f'ssh {node2} "numactl -N {numa} -m {numa} {cmd_str}"'
        print('Running ' + cmd_run)
        prun = subprocess.Popen(cmd_run, shell=True, stdout=subprocess.PIPE)
    else:
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
             'tput MBps': line[4],
             'Local CPU Util': line[5],
             'Remote CPU Util': line[6],
             'Local sd us/MB': line[7],
             'Remote sd us/MB': line[8]}]
    return data


def get_data_tcp_rr(line):
    data = [{'Recv Socket Size B': line[0],
             'Send Socket Size B': line[1],
             'Message Size B': line[2],
             'Elapsed time': line[4],
             'tput MBps': line[5],
             'Local CPU Util': line[6],
             'Remote CPU Util': line[7],
             'Local sd us/MB': line[8],
             'Remote sd us/MB': line[9]}]
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
