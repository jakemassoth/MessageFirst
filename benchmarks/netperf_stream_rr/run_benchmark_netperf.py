import os.path
import parse_netperf
import plot
import calc_overhead
import sys

PATH = os.path.dirname(os.path.abspath(__file__))


def run():
    on_das = (len(sys.argv) == 3)
    if on_das:
        node1 = sys.argv[1]
        node2 = sys.argv[2]
    global_args = '-l 30 -c -C -f M'
    message_sizes = [64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384]

    rr_experiements = []
    for size in message_sizes:
        rr_experiements.append(['TCP_RR {}B'.format(size), '-r {},{}'.format(size, size)])

    print(rr_experiements)

    if on_das:
        print('Running on DAS')
        server = parse_netperf.start_netserver(node1)
        out = parse_netperf.netperf_benchmark('TCP_STREAM', global_args, run_on_das=True, node=node2)
        master_df = parse_netperf.parse_to_df(out, 'TCP_STREAM')

        for elem in rr_experiements:
            out = parse_netperf.netperf_benchmark('TCP_RR', global_args, elem[1], run_on_das=True, node=node2)
            temp_df = parse_netperf.parse_to_df(out, elem[0])
            master_df = master_df.append(temp_df, verify_integrity=True)
    else:
        print('Running Locally')
        server = parse_netperf.start_netserver()
        out = parse_netperf.netperf_benchmark('TCP_STREAM', global_args)
        master_df = parse_netperf.parse_to_df(out, 'TCP_STREAM')

        for elem in rr_experiements:
            out = parse_netperf.netperf_benchmark('TCP_RR', global_args, elem[1])
            temp_df = parse_netperf.parse_to_df(out, elem[0])
            master_df = master_df.append(temp_df, verify_integrity=True)

    server.terminate()

    print(master_df)
    master_df.to_csv(PATH + '/data/results.csv')

    plot.run()
    calc_overhead.run()


if __name__ == '__main__':
    run()
