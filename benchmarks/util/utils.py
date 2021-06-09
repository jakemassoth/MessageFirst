import os


def make_dirs(PATH):
    if not os.path.exists(PATH + '/data'):
        os.mkdir(PATH + '/data')
    if not os.path.exists(PATH + '/plots'):
        os.mkdir(PATH + '/plots')