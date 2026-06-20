import sys
import os
from datetime import datetime
from PIL import Image, ExifTags, UnidentifiedImageError


def handle_arguments():
    """
    Collect the list of file paths passed as command-line arguments.

    Expected usage: ./scorpion FILE1 [FILE2 ...]

    Returns:
        list[str]: the file paths given on the command line.

    Raises:
        AssertionError: if no file argument is provided.
    """
    if len(sys.argv) < 2:
        raise AssertionError("missing at least one argument (path to file/filename)")
    return sys.argv[1:]


def check_file(path):
    """
    Check that a file exists and has an authorized image extension.

    Authorized extensions: .jpg, .jpeg, .png, .gif, .bmp (case-insensitive).

    Args:
        path (str): path to the file to validate.

    Returns:
        bool: True if the file exists and has a valid extension,
              False otherwise.
    """
    authorized_extension = [".jpg", ".jpeg", ".png", ".gif", ".bmp"]
    if not os.path.exists(path):
        return False
    _, ext = os.path.splitext(os.path.basename(path))
    if ext.lower() not in authorized_extension:
        return False
    return True


def get_basic_metadata(path):
    """
    Retrieve filesystem metadata for a file.

    Uses os.stat to read size and timestamps, then formats the dates
    into a human-readable string.

    Args:
        path (str): path to the file.

    Returns:
        dict: {
            "size": file size in bytes (str),
            "creation_date": formatted change/creation date (str),
            "modification_date": formatted modification date (str),
        }
    """
    stats = os.stat(path)
    return {
        "size": str(stats.st_size) + " bytes",
        "creation_date": datetime.fromtimestamp(stats.st_ctime).strftime("%Y-%m-%d %H:%M:%S"),
        "modification_date": datetime.fromtimestamp(stats.st_mtime).strftime("%Y-%m-%d %H:%M:%S")
    }


def get_exif_data(path):
    """
    Extract EXIF metadata from an image and translate tag codes to names.

    Opens the image with Pillow, reads its EXIF dictionary, and maps the
    numeric tag IDs to readable names using ExifTags.TAGS. Returns an empty
    dict if the file is not a valid image or has no EXIF data.

    Args:
        path (str): path to the image file.

    Returns:
        dict: EXIF tag names mapped to their values (empty if none).
    """
    try:
        img = Image.open(path)
        exif = {ExifTags.TAGS[k]: v for k, v in img.getexif().items() if k in ExifTags.TAGS}
        return exif
    except UnidentifiedImageError as e:
        print(f"UnidentifiedImageError: {e}")
    return {}


def display_metadata(path, basic, exif):
    """
    Display basic and EXIF metadata of a file inside a full box.

    The box width adapts to the longest line. The file path is shown
    centered in the header; basic and EXIF sections follow, each line
    framed with vertical borders. Shows "No EXIF data found" when the
    EXIF dictionary is empty.

    Args:
        path (str): file path (shown in the header).
        basic (dict): system metadata (size, dates).
        exif (dict): EXIF metadata (may be empty).

    Returns:
        None
    """
    header = f"File: {path}"
    lines = ["Basic metadata:"]
    for key, value in basic.items():
        lines.append(f"  {key:<20}: {value}")

    lines.append("")
    lines.append("EXIF metadata:")
    if not exif:
        lines.append("  No EXIF data found")
    else:
        for key, value in exif.items():
            lines.append(f"  {key:<20}: {value}")

    inner = max(len(header), max(len(line) for line in lines))
    border = "=" * (inner + 4)

    def boxed(text, center=False):
        content = text.center(inner) if center else text.ljust(inner)
        return f"| {content} |"

    print()
    print(border)
    print(boxed(header, center=True))
    print(border)
    for line in lines:
        print(boxed(line))
    print(border + "\n")


def main():
    """
    Entry point. Parse arguments, then process each file in turn.

    For every file, validates it, reads basic and EXIF metadata, and
    displays them. Invalid files are skipped. Catches filesystem and
    image-reading errors so the program never crashes on bad input.

    Returns:
        None
    """
    try:
        files = handle_arguments()
        for file in files:
            if not check_file(file):
                continue
            basic_metadata = get_basic_metadata(file)
            img_exif = get_exif_data(file)
            display_metadata(file, basic_metadata, img_exif)
    except (FileNotFoundError, OSError) as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()
