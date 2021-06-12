import os
import subprocess
import time
import random
import unittest

PATH = os.path.dirname(os.path.abspath(__file__))


def gen_bytes(num_bytes):
    choices = [1, 2, 3, 4, 5, 6, 7, 8, 9]
    res = []
    for i in range(num_bytes):
        res.append(random.choice(choices))

    return bytes(res)


def generic_test(num_bytes):
    path_client = PATH + '/client-test'
    path_server = PATH + '/server-test'

    server = subprocess.Popen([path_server])

    time.sleep(5)

    data = gen_bytes(num_bytes)
    proc = subprocess.Popen([path_client, 'localhost', '8877', str(num_bytes), data])
    ret = proc.wait(timeout=35)

    server.terminate()
    return ret


class TestMessageFirst(unittest.TestCase):
    def test_1_byte(self):
        ret = generic_test(1)
        self.assertEqual(ret, 0)

    def test_8_byte(self):
        ret = generic_test(8)
        self.assertEqual(ret, 0)

    def test_16_byte(self):
        ret = generic_test(16)
        self.assertEqual(ret, 0)


if __name__ == '__main__':
    unittest.main()
