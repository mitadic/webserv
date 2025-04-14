import requests

def main():
    response = requests.get(f"http://localhost:8080/cgi-bin/post_test.py")
    content_length = int(response.headers.get("Content-Length", 0))
    print(content_length)
    print(response)
    print(response.text)

if __name__ == "__main__":
    main()