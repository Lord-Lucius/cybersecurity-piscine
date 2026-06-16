import sys


def check_filepath():
    pass

def handle_arguments():
    args_list = sys.argv
    if len(args_list) < 2:
        raise AssertionError("missing at least one argument (path to file/filename)")


def main():
    try:
        handle_arguments()
    except AssertionError as e:
        print(f"AssertionError: {e}")


if __name__ == "__main__":
    main()
