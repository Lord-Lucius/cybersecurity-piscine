import re
import os
import sys
import requests
import hashlib
from urllib.parse import urlparse, urljoin
from bs4 import BeautifulSoup


DEBUG_FLAG = True


def handle_options():
    """
    Parse command-line arguments for the spider program.

    Expected usage: ./spider [-rlp] URL

    Options:
        -r        : enable recursive download of images.
        -l [N]    : maximum recursion depth (default 5, requires -r).
        -p [PATH] : directory where images are saved (default ./data/).

    Returns:
        tuple: (recursive_flag, depth_len, save_path, url_to_scrap)

    Raises:
        AssertionError: if arguments are missing, malformed, or invalid
                        (no URL, -l without -r, invalid depth, bad path,
                        unknown option, or URL not starting with http(s)).
    """

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
    """
    Extract all image sources from an HTML document.

    Args:
        html_body (str): raw HTML content of a page.

    Returns:
        set[str]: set of unique 'src' values from all <img> tags
                  (may contain relative URLs).
    """

    soup = BeautifulSoup(html_body, "html.parser")
    images_raw = soup.find_all("img")
    img_set = set()
    img_set.update(x.get("src") for x in images_raw if x.get("src"))
    return img_set


def get_page_links(html_body):
    """
    Extract all hyperlink targets from an HTML document.

    Args:
        html_body (str): raw HTML content of a page.

    Returns:
        set[str]: set of unique 'href' values from all <a> tags
                  (may contain relative URLs).
    """

    soup = BeautifulSoup(html_body, "html.parser")
    links = soup.find_all("a")
    links_set = set()
    links_set.update(x.get("href") for x in links if x.get("href"))
    return links_set


def save_image(images_set, save_path, base_url):
    """
    Download and save images with an authorized extension.

    Relative image URLs are resolved against base_url. Each file is
    named with an 8-char hash prefix (to avoid collisions) followed by
    a truncated original name and its extension. Failed downloads are
    skipped silently.

    Args:
        images_set (set[str]): image URLs (relative or absolute).
        save_path (str): destination directory (created if missing).
        base_url (str): URL of the page the images were found on.

    Returns:
        None
    """

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
                response = requests.get(image, timeout=(5, 5), headers=headers)
                if not response.headers.get("Content-Type", "").startswith("image/"):
                    continue
                img_data = response.content
                final_path = os.path.join(save_path, filename)
                with open(final_path, "wb") as handler:
                    handler.write(img_data)
        except requests.exceptions.RequestException:
            continue


def scrape_page(url, save_path, current_depth, max_depth, visited, base_domain):
    """
    Recursively crawl a page, download its images and follow its links.

    Stops when the maximum depth is exceeded or the URL was already
    visited. Only follows links that stay on the same domain as the
    starting URL. URL fragments (#...) are stripped to avoid revisiting
    the same page.

    Args:
        url (str): URL of the page to scrape.
        save_path (str): destination directory for images.
        current_depth (int): current recursion depth.
        max_depth (int): maximum allowed recursion depth.
        visited (set[str]): URLs already processed (mutated in place).
        base_domain (str): netloc of the starting URL, used to stay
                           within the same domain.

    Returns:
        None
    """

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
    """
    Entry point. Parse options, then start the crawl from the given URL.

    Runs a single-page scrape when -r is absent, or a recursive crawl
    otherwise. Catches and reports argument, connection, recursion and
    keyboard-interrupt errors cleanly.

    Returns:
        None
    """

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
        print(f"requests error: {e}")
    except RecursionError as e:
        print(f"recursion error: {e}")
    except KeyboardInterrupt:
        print("\nInterrupted by user")


if __name__ == "__main__":
    main()
