import sys
import re
import requests
from bs4 import BeautifulSoup

class CustomsErrors:
    class StatusCodeError(Exception):
        def __init__(self, message):
            super().__init__(message)

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


def get_page_images(text):
    soup = BeautifulSoup(text, "html.parser")
    images_raw = soup.find_all('img')
    img_set = set()
    img_set.update(x.get('src') for x in images_raw if x.get('src'))
    if img_set == {}:
        return set()
    return img_set

def get_page_links(text):
    soup = BeautifulSoup(text, "html.parser")
    links = soup.find_all('a')
    links_set = set()
    links_set.update(x.get('href') for x in links if x.get('href'))
    if links_set == {}:
        return set()
    return links_set

def save_image():
    pass

def scrape_page(url, save_path, current_depth, max_depth, visited):
    r = requests.get(url, timeout=(2, 2))
    if r.status_code != 200:
        return

def main():
    try:
        recursif_mod, depth_len, save_path, url_to_scrap = handle_options()
        print("============================ HEADER =============================")
        print(f"{recursif_mod} :: {depth_len} :: {save_path} :: {url_to_scrap}")
        print("=================================================================\n\n")
        if not recursif_mod:
            scrape_page(url_to_scrap, save_path, 0, 0, set())
        else:
            scrape_page(url_to_scrap, save_path, 0, depth_len, set())
    except AssertionError as e:
        print(f"AssertionError: {e}")
    except requests.exceptions.ConnectionError as e:
        print(f"requests error ({e})")
    except CustomsErrors.StatusCodeError as e:
        print(f"status code error ({e})")

if __name__ == "__main__":
    main()
