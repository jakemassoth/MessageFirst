import subprocess
import os
import random
import sys
import time
import pandas
import matplotlib.pyplot as plt


def plot_message_size_vs_throughput(df):
    plt.bar('Message Size', 'Throughput (KB/s)', data=df)
    plt.xlabel('Message Size (B)')
    plt.ylabel('Throughput (KB/s)')
    plt.title('Message Size vs Throughput')
    plt.savefig('message_size_vs_throughput.png')


def plot_message_size_vs_transactions(df):
    plt.bar('Message Size', 'Num Transactions', data=df)
    plt.xlabel('Message Size (B)')
    plt.ylabel('Number of Transactions')
    plt.title('Message Size vs Number of Transactions in 30s')
    plt.savefig('message_size_vs_transactions.png')


def plot_graphs(df):
    df['Message Size'] = df['Message Size'].apply(str)
    plot_message_size_vs_transactions(df)
    plt.close()
    plot_message_size_vs_throughput(df)


def calc_thruput(num_transactions, message_size, test_length):
    total_transferred = num_transactions * 2 * message_size
    return total_transferred / test_length / 1024


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


def run_single_threaded_on_das(port, num_bytes, node1, node2):
    path_client = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/echo-benchmark-client'
    path_server = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/echo-benchmark-server'

    ip_num = node1.strip('node')
    ip_num = ip_num.lstrip('0')
    host = f'10.149.0.{ip_num}'
    print(f'running server on {host}:{port}')

    server = subprocess.Popen(f'ssh {node1} "{path_server} {port}"', shell=True)

    time.sleep(5)

    data = gen_bytes(num_bytes)
    proc = subprocess.Popen(f'ssh {node2} "{path_client} {host} {port} {num_bytes} {data}"', stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, shell=True)
    stdout, stderr = proc.communicate(timeout=35)

    server.terminate()
    print('stderr' + str(stderr))

    res = str(stdout)
    print('stdout:' + res)
    res = res.split('\n')
    res = res[0].split('\\n')
    res = res[0].strip("b'")

    return int(res)


if __name__ == '__main__':
    message_sizes = [64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384]
    test_length = 30
    records = []
    on_das = (len(sys.argv) == 3)

    for size in message_sizes:
        print(f'Running benchmark for message size {size}')
        if on_das:
            print('Running on DAS')
            num_transactions = run_single_threaded_on_das(str(random.randrange(1000, 9999)), size, sys.argv[1], sys.argv[2])
        else:
            print('Running Locally')
            num_transactions = run_single_threaded('localhost', '8877', size)

        thruput = calc_thruput(num_transactions, size, test_length)

        print(f'Thruput: {thruput}, transactions: {num_transactions}')

        res = {
            'Message Size': size,
            'Num Transactions': num_transactions,
            'Throughput (KB/s)': thruput,
            'Test Length (s)': test_length
        }
        records.append(res)
    df = pandas.DataFrame.from_records(records)

    df.to_csv('results_single_thread.csv')

    plot_graphs(df)

    print(df)
