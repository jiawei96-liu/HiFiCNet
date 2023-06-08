import sys
import os
import subprocess


def execShellCommand(cmd):
    print("\033[32m++ "+cmd+" ====>\033[0m")
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, executable="/bin/bash")
    ret = p.stdout.read().decode('utf-8')
    print(ret)
    return ret


def read_file_as_str(file_path):
    # 判断路径文件存在
    if not os.path.isfile(file_path):
        raise TypeError("file does not exist")

    all_the_text = open(file_path).read()
    # print(all_the_text)
    return all_the_text

