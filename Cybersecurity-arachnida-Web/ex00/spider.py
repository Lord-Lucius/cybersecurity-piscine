import re
import os
import sys
import requests
import hashlib
from urllib.parse import urlparse, urljoin
from bs4 import BeautifulSoup


DEBUG_FLAG = True


def handle_options():
    recursive_flag = False
    depth_len = 5
    save_path = "./data/"
    args = sys.argv
    i = 1
    if len(args) < 2:
        raise AssertionError("no URL gave")
    while i < len(args) - 1:
        if args[i] == "-r":
            recursive_flag = True
            i += 1
        elif args[i] == "-l":
            if not recursive_flag:
                raise AssertionError("can't use -l without using -r (recursive)")
            if i + 1 >= len(args) - 1:
                raise AssertionError("no depth given to -l")
            if not args[i + 1].isdigit() or int(args[i + 1]) == 0:
                raise AssertionError(
                    "-l argument should be a positive integer different than 0"
                )
            depth_len = int(args[i + 1])
            i += 2
        elif args[i] == "-p":
            if i + 1 >= len(args) - 1:
                raise AssertionError("no path given to -p")
            if args[i + 1][0] == "-" or re.search("^https?://", args[i + 1]):
                raise AssertionError("-p path doesn't look like a valid path")
            save_path = args[i + 1]
            i += 2
        else:
            raise AssertionError(
                "option not recognized, please enter a valid option (-r -l -p)"
            )
    url_to_scrap = args[-1]
    if re.search("^https?://", url_to_scrap) is None:
        raise AssertionError("URL must start with http:// or https://")
    return recursive_flag, depth_len, save_path, url_to_scrap


def get_page_images(html_body):
    soup = BeautifulSoup(html_body, "html.parser")
    images_raw = soup.find_all("img")
    img_set = set()
    img_set.update(x.get("src") for x in images_raw if x.get("src"))
    return img_set


def get_page_links(html_body):
    soup = BeautifulSoup(html_body, "html.parser")
    links = soup.find_all("a")
    links_set = set()
    links_set.update(x.get("href") for x in links if x.get("href"))
    return links_set


def save_image(images_set, save_path, base_url):
    os.makedirs(save_path, exist_ok=True)
    authorized_extension = ["jpg", "jpeg", "png", "gif", "bmp"]
    headers = {"User-Agent": "Mozilla/5.0"}
    for image in images_set:
        image = urljoin(base_url, image)
        try:
            if urlparse(image).path.split(".")[-1].lower() in authorized_extension:
                name, ext = os.path.splitext(os.path.basename(urlparse(image).path))
                short_hash = hashlib.md5(image.encode()).hexdigest()[:8]
                filename = short_hash + "_" + name[:50] + ext
                img_data = requests.get(image, timeout=(5, 5), headers=headers).content
                final_path = os.path.join(save_path, filename)
                with open(final_path, "wb") as handler:
                    handler.write(img_data)
        except requests.exceptions.RequestException:
            continue


def scrape_page(url, save_path, current_depth, max_depth, visited, base_domain):
    if current_depth > max_depth:
        return
    if url in visited:
        return
    visited.add(url)
    if DEBUG_FLAG:
        print(current_depth, url)
    headers = {"User-Agent": "Mozilla/5.0"}
    try:
        r = requests.get(url, timeout=(5, 5), headers=headers)
    except requests.exceptions.RequestException:
        return
    if r.status_code != 200:
        if DEBUG_FLAG:
            print(f"==== status_code: {r.status_code} ====")
        return
    body = r.text
    save_image(get_page_images(body), save_path, url)
    links = get_page_links(body)
    for link in links:
        absolute = urljoin(url, link).split("#")[0]
        if urlparse(absolute).netloc == base_domain:
            scrape_page(
                absolute, save_path, current_depth + 1, max_depth, visited, base_domain
            )


def main():
    try:
        recursif_mod, depth_len, save_path, url_to_scrap = handle_options()
        base_domain = urlparse(url_to_scrap).netloc
        if not recursif_mod:
            scrape_page(url_to_scrap, save_path, 0, 0, set(), base_domain)
        else:
            scrape_page(url_to_scrap, save_path, 0, depth_len, set(), base_domain)
    except AssertionError as e:
        print(f"AssertionError: {e}")
    except (requests.exceptions.ConnectionError, ConnectionRefusedError) as e:
        print(f"requests error ({e})")
    except RecursionError as e:
        print(f"too much recursion ({e})")
    except KeyboardInterrupt:
        print("\nInterrupted by user")


if __name__ == "__main__":
    main()
