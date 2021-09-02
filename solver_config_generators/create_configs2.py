import sys
import os.path

configs = {
    # "encoder": ("0 $1 $2", None),
    # "encoder": ("0 $1 $2", [10,60,300]),
    # "encoder": ("0 $1 $2", [0]),
    # "dynamic": ("1 $1 $2", [5, 10, 100]),
    # "dynamic": ("1 $1 $2", [0, 5, 10, 100, 1000, 10000]),
    "incremental": ("2 $1 $2", [5, 10, 100]),
    # "incremental": ("2 $1 $2", [0, 5, 10, 100, 1000, 10000]),
    # "propagator": ("3 $1 $2", None),
    # "pairs": ("4 $1 $2", None),
    # "topdown": ("5 $1 $2", [5, 10, 100, 1000, 10000]),
    # "bottomlayers": ("6 $1 $2", None),
    # "extendedgroups": ("7 $1 $2", None),

}

def create_file(target_dir: str, config_name:str, config: str):
    start_name = "starexec_run_"
    header = """#!/bin/sh    

"""
    file_name = start_name + config_name + ".sh"
    f = open(target_dir + "/" + file_name, "a")
    f.write(header)

    f.write("""if [[ $1 == *.ctt ]]
then

""")
    f.write("./ctt_benchmark " + config + "\n")
    f.write("""
elif [[ $1 == *.opb ]]
then

""")
    f.write("./pseudo_boolean_test " + config + "\n")
    f.write("""
elif [[ $1 == *.wcnf ]]
then

""")
    f.write("./max_sat " + config + "\n")
    f.write("fi\n")

    f.close()


def create_files(target_dir: str):
    if not os.path.isdir(target_dir):
        raise Exception(target_dir + " is not a directory")

    for k, v in configs.items():
        if v[1] is None:
            create_file(target_dir, k, v[0])
        else:
            for d in v[1]:
                delay = str(d)
                create_file(target_dir, k + "_" + delay, v[0] + " " + delay)



def main():
    if len(sys.argv) != 2:
        raise Exception("wrong number of arguments")
    target_dir = sys.argv[1]

    create_files(target_dir)


if __name__ == '__main__':
    main();
