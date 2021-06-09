import subprocess
import os
import random
import time
import pandas
from benchmarks.util.utils import make_dirs


PATH = os.path.dirname(os.path.abspath(__file__))


def calc_thruput(num_transactions, message_size, test_length):
    total_transferred = num_transactions * 2 * message_size
    return total_transferred / test_length / (1024 * 1024)


def gen_bytes(num_bytes):
    choices = [1, 2, 3, 4, 5, 6, 7, 8, 9]
    res = []
    for i in range(num_bytes):
        res.append(random.choice(choices))

    return bytes(res)


def run_single_threaded(host, port, num_bytes):
    path_client = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/echo-benchmark-client'
    path_server = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/echo-benchmark-server'

    server = subprocess.Popen([path_server, port])

    time.sleep(5)

    data = gen_bytes(num_bytes)
    proc = subprocess.Popen([path_client, host, port, str(num_bytes), data], stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate(timeout=35)

    server.terminate()
    print(stderr)

    res = str(stdout)
    res = res.split('\n')
    res = res[0].split('\\n')
    res = res[0].strip("b'")

    return int(res)


def get_numa_domain_network_card(node):
    proc = subprocess.Popen(f'ssh {node} "cat /sys/class/net/ib0/device/numa_node"', shell=True)
    stdout, stderr = proc.communicate()
    if stdout is None:
        return 0

    res = str(stdout)
    res = res.split('\n')
    res = res[0].split('\\n')
    res = res[0].strip("b'")
    print('numa node: ' + res)

    return int(res)


def run_single_threaded_on_das(port, num_bytes, node1, node2):
    path_client = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/echo-benchmark-client'
    path_server = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/echo-benchmark-server'

    ip_num = node1.strip('node')
    ip_num = ip_num.lstrip('0')
    host = f'10.149.0.{ip_num}'
    print(f'running server on {host}:{port}')

    numa_node_server = get_numa_domain_network_card(node1)

    server = subprocess.Popen(f'ssh {node1} "numactl -N {numa_node_server} -m {numa_node_server} {path_server} {port}"',
                              shell=True)

    time.sleep(5)

    numa_node_client = get_numa_domain_network_card(node2)
    data = gen_bytes(num_bytes)
    cmd = f'ssh {node2} "numactl -N {numa_node_client} -m {numa_node_client} {path_client} {host} {port} {num_bytes} {data}" '

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    stdout, stderr = proc.communicate(timeout=35)

    server.terminate()
    print('stderr' + str(stderr))

    res = str(stdout)
    print('stdout:' + res)
    res = res.split('\n')
    res = res[0].split('\\n')
    res = res[0].strip("b'")

    return int(res)


def run_benchmark(on_das=False, node1=None, node2=None):
    make_dirs(PATH)

    message_sizes = [64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384]
    test_length = 30
    records = []

    for size in message_sizes:
        print(f'Running benchmark for message size {size}')
        if on_das:
            print('Running on DAS')
            num_transactions = run_single_threaded_on_das(str(random.randrange(1000, 9999)), size, node1, node2)
        else:
            print('Running Locally')
            num_transactions = run_single_threaded('localhost', '8877', size)

        thruput = calc_thruput(num_transactions, size, test_length)

        print(f'Thruput: {thruput}, transactions: {num_transactions}')

        res = {
            'Message Size': size,
            'Num Transactions': num_transactions,
            'Throughput (MB/s)': thruput,
            'Test Length (s)': test_length
        }
        records.append(res)
    df = pandas.DataFrame.from_records(records)

    df.to_csv(PATH + '/data/results_single_thread.csv')


if __name__ == '__main__':
    run_benchmark()
