import requests
import os

file_path = './links.txt'
download_folder = './pics'

def download_image(url, folder_path):
    try:
        response = requests.get(url)
        if response.status_code == 200:
            filename = os.path.join(folder_path, url.split('/')[-1])
            with open(filename, 'wb') as file:
                file.write(response.content)
            print(f"Downloaded: {filename}")
        else:
            print(f"Failed to download: {url} - Status code: {response.status_code}")
    except Exception as e:
        print(f"Error downloading {url}: {str(e)}")

os.makedirs(download_folder, exist_ok=True)

with open(file_path, 'r') as file:
    for line in file:
        url = line.strip()
        download_image(url, download_folder)
