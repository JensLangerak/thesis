import sys
import os.path

configs = {
    "encoder": ("0 $1 $2", None),
    # "encoder": ("0 $1 $2", [10,60,300]),
    # "encoder": ("0 $1 $2", [0]),
    # "dynamic": ("1 $1 $2", [0, 5, 10, 100, 1000, 10000]),
    # "incremental": ("2 $1 $2", [0, 5, 10, 100, 1000, 10000]),
    # "propagator": ("3 $1 $2", None),
    # "pairs": ("4 $1 $2", None),
    # "topdown": ("5 $1 $2", [0, 5, 10, 100, 1000, 10000]),
    # "bottomlayers": ("6 $1 $2", None),

}

def create_file(target_dir: str, exec_file:str, config_name:str, config: str):
    start_name = "starexec_run_"
    header = """#!/bin/sh    

"""
    file_name = start_name + config_name + ".sh"
    f = open(target_dir + "/" + file_name, "a")
    f.write(header)
    f.write(exec_file + " " + config + "\n")
    f.close()


def create_files(target_dir: str, exec_file: str):
    if not os.path.isdir(target_dir):
        raise Exception(target_dir + " is not a directory")

    for k, v in configs.items():
        if v[1] is None:
            create_file(target_dir, exec_file, k, v[0])
        else:
            for d in v[1]:
                delay = str(d)
                create_file(target_dir, exec_file, k + "_" + delay, v[0] + " " + delay)



def main():
    if len(sys.argv) != 3:
        raise Exception("wrong number of arguments")
    target_dir = sys.argv[1]
    exec_file = sys.argv[2]

    create_files(target_dir, exec_file)


if __name__ == '__main__':
    main();
