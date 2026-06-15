import sys
import re

def handle_options():
    recursive_flag = False
    depth_len = 5
    save_path = "./data/"
    url_to_scrap = ""
    args = sys.argv
    i = 1
    if len(args) < 2:
        raise AssertionError("no URL gave")
    while i < len(args) - 1:
        if args[i] == "-r":
            recursive_flag = True
            i += 1
            continue
        if args[i] == "-l":
            if not recursive_flag:
                raise AssertionError("can't use -l without using -r (recursive)")
            elif i+1 >= len(args):
                raise AssertionError("no depth given to -l")
            elif not args[i + 1].isdigit() or int(args[i+1]) == 0:
                raise AssertionError("-l argument should be a positive number (interger) and different than 0")
            else:
                depth_len = int(args[i+1])
                i += 2
                continue
        if args[i] == "-p":
            i += 1
            if i >= len(args) or args[i][0] == "-" or re.search("^https?://", args[i]):
                raise AssertionError("-p path doesn't look like a valid path")
            save_path = args[i]
            continue
        else:
            raise AssertionError("option not recognized please enter valid option (-r -l -p)")
        i += 1
    url_to_scrap = args[-1]
    x = re.search("^https?://", url_to_scrap)
    if x is None:
        raise AssertionError("URL must start with http:// or https://")
    return recursive_flag, depth_len, save_path, url_to_scrap


def scrape_page(url, save_path, recursive_flag, depth_len):
    pass


def main():
    try:
        recursif_mod, depth_len, save_path, url_to_scrap = handle_options()
        print(f"{recursif_mod} :: {depth_len} :: {save_path} :: {url_to_scrap}")
        scrape_page(url_to_scrap, save_path, recursif_mod, depth_len)
    except AssertionError as e:
        print(f"AssertionError: {e}")

if __name__ == "__main__":
    main()
